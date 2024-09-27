#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <regex>
#include <string>
#include <vector>
#include <optional>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <flatbuffers/reflection_generated.h>
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/util.h>
#include <flatbuffers/minireflect.h>

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <sol/sol.hpp>

#include <entt/core/hashed_string.hpp>

#include "io.hpp"
#include "strings.hpp"
#include "templates.hpp"
#include "hash.hpp"

using namespace std;
using namespace std::filesystem;
using namespace inja;
using namespace nlohmann;

string executable_name(string name) {
  return name
#ifdef _WIN32
         + ".exe"
#endif
    ;
}

path find_executable(string bin) {
  auto cwd = io::get_current_executable_directory();
  auto name = executable_name(bin);

  auto current = cwd;
  while (true) {
    if (filesystem::exists(current / name)) {
      return current / name;
    }

    if (filesystem::exists(current / "bin" / name)) {
      return current / "bin" / name;
    }

    if (current.has_parent_path() && current != current.parent_path()) {
      current = current.parent_path();
      continue;
    }

    return name;
  }
}

path find_flatc() {
  return find_executable("flatc");
}

int flatc(const path &working_dir, const vector<string> arguments) {
  return io::shell(find_flatc().string(), arguments, working_dir);
}

json flac_parse_attributes(const flatbuffers::Vector<flatbuffers::Offset<reflection::KeyValue>> *list) {
  auto attributes = json::object({});
  if (list == nullptr) {
    return attributes;
  }

  for (int i = 0; i < list->size(); i++) {
    auto entry = list->Get(i);
    attributes[entry->key()->str()] = entry->value()->str();
  }

  return attributes;
}

json flac_parse_documentation(const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *list) {
  auto documentation = json::array({});
  if (list == nullptr) {
    return documentation;
  }

  for (int i = 0; i < list->size(); i++) {
    documentation.push_back(list->Get(i)->str());
  }

  return documentation;
}

json flac_parse_documentation_text(const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *list) {
  string doc = "";
  if (list == nullptr) {
    return doc;
  }

  for (int i = 0; i < list->size(); i++) {
    doc += list->Get(i)->str() + "\n";
  }

  return doc.substr(0, doc.size() - 1);
}

optional<string> flatc_reflection(const path &file) {
  string location = std::tmpnam(nullptr);
  filesystem::create_directories(location);

  auto bfbs = path(location) / path(file).filename().replace_extension(".bfbs");

  auto arguments = vector<string>{
    "--binary", "--bfbs-gen-embed", "--bfbs-comments", "--bfbs-builtins",
    "-o",       location.c_str(),   "--schema",        file.string(),
  };

  auto status = flatc(io::get_current_executable_directory(), arguments);
  if (status != 0) {
    return {};
  }

  auto [exists, buffer] = io::read_file(bfbs);
  if (!exists) {
    return {};
  }

  auto schema_ptr = reflection::GetSchema(buffer.c_str());
  auto &schema = *schema_ptr;

  auto objects = schema.objects();
  auto enums = schema.enums();
  auto services = schema.services();
  auto files = schema.fbs_files();

  auto file_ident = schema.file_ident() == nullptr ? "" : schema.file_ident()->str();
  auto file_ext = schema.file_ext() == nullptr ? "" : schema.file_ext()->str();

  auto data = json({});
  data["file_ident"] = json::string_t(file_ident);
  data["file_ext"] = json::string_t(file_ext);
  data["tables"] = json::array({});
  data["structs"] = json::array({});
  data["enums"] = json::array({});
  data["services"] = json::array({});
  data["files"] = json::array({});
  data["advanced_features"] = json({});

  // Types

  data["advanced_features"]["advanced_array_features"] =
    (schema.advanced_features() & reflection::AdvancedFeatures::AdvancedArrayFeatures) != 0;
  data["advanced_features"]["advanced_union_features"] =
    (schema.advanced_features() & reflection::AdvancedFeatures::AdvancedUnionFeatures) != 0;
  data["advanced_features"]["optional_scalars"] =
    (schema.advanced_features() & reflection::AdvancedFeatures::OptionalScalars) != 0;
  data["advanced_features"]["defaualt_vectors_and_strings"] =
    (schema.advanced_features() & reflection::AdvancedFeatures::DefaultVectorsAndStrings) != 0;

  auto type_name = [&](const reflection::BaseType type) {
    // If this errors, it means reflection data changed.
    static_assert(reflection::BaseType::MaxBaseType == 19);

    switch (type) {
    case reflection::BaseType::None:
      return "none"s;
    case reflection::BaseType::UType:
      return "utype"s;
    case reflection::BaseType::Bool:
      return "bool"s;
    case reflection::BaseType::Byte:
      return "byte"s;
    case reflection::BaseType::UByte:
      return "ubyte"s;
    case reflection::BaseType::Short:
      return "short"s;
    case reflection::BaseType::UShort:
      return "ushort"s;
    case reflection::BaseType::Int:
      return "int"s;
    case reflection::BaseType::UInt:
      return "uint"s;
    case reflection::BaseType::Long:
      return "long"s;
    case reflection::BaseType::ULong:
      return "ulong"s;
    case reflection::BaseType::Float:
      return "float"s;
    case reflection::BaseType::Double:
      return "double"s;
    case reflection::BaseType::String:
      return "string"s;
    case reflection::BaseType::Vector:
      return "vector"s;
    case reflection::BaseType::Obj:
      return "obj"s;
    case reflection::BaseType::Union:
      return "union"s;
    case reflection::BaseType::Array:
      return "array"s;
    case reflection::BaseType::Vector64:
      return "vector64"s;
    default:
      spdlog::error("Unknown type: {}", static_cast<int>(type));
      return "unknown"s;
    }
  };

  auto type_info = [&](const reflection::Type *type) {
    auto name = type_name(type->base_type());
    auto data = json({
      { "id", json::number_integer_t(entt::hashed_string::value(name.c_str())) },
      { "index", type->index() },
      { "name", name },
      { "size", type->base_size() },
      { "length", type->fixed_length() },
    });
    data["element_type"] = type_name(type->element());
    if (name == "array" || name == "vector" || name == "vector64") {
      data["element_size"] = type->element_size();
    } else {
      data["element_size"] = nullptr;
    }
    return data;
  };

  // Objects

  for (auto i = 0; i < objects->size(); i++) {
    auto obj = objects->Get(i);
    auto name = obj->name()->str();

    auto type = json({
      { "id", json::number_integer_t(entt::hashed_string::value(name.c_str())) },
      { "name", name.substr(name.find_last_of('.') + 1) },
      { "namespace", name.substr(0, name.find_last_of('.')) },
      { "attributes", flac_parse_attributes(obj->attributes()) },
      { "documentation", json({
                           { "text", flac_parse_documentation_text(obj->documentation()) },
                           { "lines", flac_parse_documentation(obj->documentation()) },
                         }) },
      { "minalign", json::number_integer_t(obj->minalign()) },
      { "declaration_file", obj->declaration_file()->str() },
      { "fields",
        [&]() {
          auto fields = json::array({});

          auto list = obj->fields();
          if (list == nullptr) {
            return fields;
          }

          for (int i = 0; i < list->size(); i++) {
            auto entry = list->Get(i);
            fields.push_back({
              { "id", json::number_integer_t(entry->id()) },
              { "name", json::string_t(entry->name()->str()) },
              { "type", type_info(entry->type()) },
              { "attributes", flac_parse_attributes(entry->attributes()) },
              { "documentation", json({
                                   { "text", flac_parse_documentation_text(entry->documentation()) },
                                   { "lines", flac_parse_documentation(entry->documentation()) },
                                 }) },
              { "offset", json::number_integer_t(entry->offset()) },
              { "padding", json::number_integer_t(entry->padding()) },

              { "key", json::boolean_t(entry->key()) },
              { "deprecated", json::boolean_t(entry->deprecated()) },
              { "optional", json::boolean_t(entry->optional()) },
              { "required", json::boolean_t(entry->required()) },
              { "offset64", json::boolean_t(entry->offset64()) },

              { "default_integer", json::number_integer_t(entry->default_integer()) },
              { "default_float", json::number_integer_t(entry->default_real()) },
            });
          }

          return fields;
        }() },
    });

    if (obj->is_struct()) {
      type["bytesize"] = json::number_integer_t(obj->bytesize());
      data["structs"].push_back(type);
    } else {
      data["tables"].push_back(type);
    }
  }

  // Enums

  for (auto i = 0; i < enums->size(); i++) {
    auto en = enums->Get(i);
    auto name = en->name()->str();

    auto count = 0;

    bool has_values = false;
    int64_t min_value = 0, max_value = 0;

    auto values = en->values();
    if (values != nullptr) {
      for (int i = 0; i < values->size(); i++) {
        auto entry = values->Get(i);
        auto value = entry->value();
        if (!has_values) {
          has_values = true;
          min_value = max_value = value;
        } else {
          min_value = value < min_value ? value : min_value;
          max_value = value > max_value ? value : max_value;
        }
        count += 1;
      }
    }

    auto type = json({
      { "id", json::number_integer_t(entt::hashed_string::value(name.c_str())) },
      { "name", name.substr(name.find_last_of('.') + 1) },
      { "namespace", name.substr(0, name.find_last_of('.')) },
      { "type", type_info(en->underlying_type()) },
      { "attributes", flac_parse_attributes(en->attributes()) },
      { "documentation", json({
                           { "text", flac_parse_documentation_text(en->documentation()) },
                           { "lines", flac_parse_documentation(en->documentation()) },
                         }) },
      { "min", json(nullptr) },
      { "max", json(nullptr) },
      { "range", json(nullptr) },
      { "count", json(0) },
      { "is_union", en->is_union() },
      { "declaration_file", en->declaration_file()->str() },
      { "values",
        [&]() {
          auto values = json::array({});
          auto list = en->values();
          if (list == nullptr) {
            return values;
          }

          for (int i = 0; i < list->size(); i++) {
            auto entry = list->Get(i);
            auto value = entry->value();
            values.push_back({
              { "name", entry->name()->str() },
              { "value", json::number_integer_t(value) },
              { "attributes", flac_parse_attributes(entry->attributes()) },
              { "documentation", json({
                                   { "text", flac_parse_documentation_text(entry->documentation()) },
                                   { "lines", flac_parse_documentation(entry->documentation()) },
                                 }) },
            });
          }

          return values;
        }() },
    });

    if (has_values) {
      type["min"] = json::number_integer_t(min_value);
      type["max"] = json::number_integer_t(max_value);
      type["range"] = json::number_integer_t(max_value - min_value);
    }

    type["count"] = json::number_integer_t(count);

    data["enums"].push_back(type);
  }

  // Files
  for (auto i = 0; i < files->size(); i++) {
    auto f = files->Get(i);
    if (f->filename() == nullptr) {
      continue;
    }

    auto file = json({
      { "path", json::string_t(f->filename()->str()) },
      { "includes",
        [&]() {
          auto includes = json::array({});

          auto list = f->included_filenames();
          if (list == nullptr) {
            return includes;
          }

          for (int i = 0; i < list->size(); i++) {
            auto entry = list->Get(i);
            includes.push_back(json::string_t(entry->str()));
          }

          return includes;
        }() },

    });

    data["files"].push_back(file);
  }

  return data.dump(2);
}

auto on_script_error(lua_State *, sol::protected_function_result pfr) {
  sol::error err = pfr;
  spdlog::error("script error: {}", err.what());
  return pfr;
}

int run_project(const path &entrypoint, const vector<string> &arguments) {
  auto project_file = path(entrypoint);

  if (filesystem::is_directory(project_file)) {
    project_file /= "flatt.lua";
  }

  if (!filesystem::exists(project_file)) {
    spdlog::error("Unable to find project file: {}", project_file.string());
    return 1;
  }

  project_file = filesystem::absolute(project_file);
  auto project_dir = project_file.parent_path();

#ifdef _WIN32
  SetCurrentDirectory(project_dir.string().c_str());
#else
  chdir(project_dir.string().c_str());
#endif


  sol::state lua;
  lua.open_libraries(
    sol::lib::base, sol::lib::package, sol::lib::coroutine, sol::lib::string, sol::lib::os, sol::lib::math,
    sol::lib::table, sol::lib::debug, sol::lib::bit32, sol::lib::io, sol::lib::ffi, sol::lib::jit, sol::lib::utf8);


  // variables

  lua["flatt"] = lua.create_table();
  lua["flatt"]["executable_dir"] = io::get_current_executable_directory().string();
  lua["flatt"]["project_dir"] = project_dir.string();
  lua["flatt"]["argv"] = arguments;

  // logs

  lua["log"] = lua.create_table();
  lua["log"]["set_level"] = [&](const std::string &value) {
    spdlog::set_level(spdlog::level::from_str(value));
  };
  lua["log"]["get_level"] = [&]() {
    return spdlog::level::to_string_view(spdlog::get_level());
  };
  lua["log"]["trace"] = [](const std::string &msg) {
    spdlog::trace(msg);
  };
  lua["log"]["debug"] = [](const std::string &msg) {
    spdlog::debug(msg);
  };
  lua["log"]["info"] = [](const std::string &msg) {
    spdlog::info(msg);
  };
  lua["log"]["warn"] = [](const std::string &msg) {
    spdlog::warn(msg);
  };
  lua["log"]["error"] = [](const std::string &msg) {
    spdlog::error(msg);
  };
  lua["log"]["critical"] = [](const std::string &msg) {
    spdlog::critical(msg);
  };

  // files

  lua["file"] = lua.create_table();
  lua["file"]["exists"] = [](const string &file) {
    return filesystem::exists(file);
  };
  lua["file"]["read"] = [](const string &file) {
    auto [success, content] = io::read_file(file);
    return content;
  };
  lua["file"]["write"] = [](const string &file, const string &content) {
    return io::write_file(file, content);
  };
  lua["file"]["hash"] = [](const string &file) {
    return io::hash_file(file);
  };

  // dir

  lua["dir"] = lua.create_table();
  lua["dir"]["hash"] = [](const std::string &path) {
    return io::hash_dir(path);
  };
  lua["dir"]["list_files"] = [](const std::string &path) {
    auto files = io::list_files(path);
    auto paths = vector<string>{};
    for (auto file : files) {
      paths.push_back(file.string());
    }
    return sol::as_table(paths);
  };

  lua["dir"]["list_dirs"] = [](const std::string &path) {
    auto files = io::list_dirs(path);
    auto paths = vector<string>{};
    for (auto file : files) {
      paths.push_back(file.string());
    }
    return sol::as_table(paths);
  };

  // string

  // lua["string"] = lua.create_table();
  lua["string"]["pad_left"] = sol::overload(
    [](const std::string &value, const int length) {
      return str::padleft(value, length, " ");
    },
    [](const std::string &value, const int length, const std::string &pad) {
      return str::padleft(value, length, pad);
    });
  lua["string"]["pad_right"] = sol::overload(
    [](const std::string &value, const int length) {
      return str::padleft(value, length, " ");
    },
    [](const std::string &value, const int length, const std::string &pad) {
      return str::padright(value, length, pad);
    });
  lua["string"]["tokenize"] = [&](const std::string &value) {
    return sol::as_table(str::tokenize(value));
  };
  lua["string"]["split"] = sol::overload(
    [&](const std::string &value, const std::string &delimiter) {
      return sol::as_table(str::split(value, delimiter));
    },
    [&](const std::string &value, const std::string &delimiter, int limit) {
      return sol::as_table(str::split(value, delimiter, limit));
    });
  lua["string"]["ends_with"] = [](const std::string &value, const std::string &match) {
    return str::ends_with(value, match);
  };
  lua["string"]["starts_with"] = [](const std::string &value, const std::string &match) {
    return str::starts_with(value, match);
  };
  lua["string"]["trim"] = [](const std::string &value) {
    return str::trim_copy(value);
  };
  lua["string"]["trim_left"] = [](const std::string &value) {
    return str::trim_left_copy(value);
  };
  lua["string"]["trim_right"] = [](const std::string &value) {
    return str::trim_right_copy(value);
  };
  lua["string"]["join"] = [](const sol::as_table_t<vector<string>> &parts, const string &delim = ",") {
    return str::join(parts.value(), delim);
  };
  lua["string"]["to_lower"] = [](const string &value) {
    return str::to_lower(value);
  };
  lua["string"]["to_upper"] = [](const string &value) {
    return str::to_upper(value);
  };
  lua["string"]["to_upper_first"] = [](const string &value) {
    return str::to_upper_first(value);
  };
  lua["string"]["to_lower_first"] = [](const string &value) {
    return str::to_lower_first(value);
  };
  lua["string"]["to_snake"] = [](const string &value) {
    return str::to_snake(value);
  };
  lua["string"]["to_kebab"] = [](const string &value) {
    return str::to_kebab(value);
  };
  lua["string"]["to_pascal"] = [](const string &value) {
    return str::to_pascal(value);
  };
  lua["string"]["to_camel"] = [](const string &value) {
    return str::to_camel(value);
  };
  lua["string"]["to_const"] = [](const string &value) {
    return str::to_const(value);
  };
  lua["string"]["to_train"] = [](const string &value) {
    return str::to_train(value);
  };
  lua["string"]["to_ada"] = [](const string &value) {
    return str::to_ada(value);
  };
  lua["string"]["to_cobol"] = [](const string &value) {
    return str::to_cobol(value);
  };
  lua["string"]["to_dot"] = [](const string &value) {
    return str::to_dot(value);
  };
  lua["string"]["to_path"] = [](const string &value) {
    return str::to_path(value);
  };
  lua["string"]["to_space"] = [](const string &value) {
    return str::to_space(value);
  };
  lua["string"]["to_capital"] = [](const string &value) {
    return str::to_capital(value);
  };
  lua["string"]["to_cpp"] = [](const string &value) {
    return str::to_cpp(value);
  };

  // templates

  lua["template"] = lua.create_table();
  lua["template"]["render_string"] = [](const string &source, const string &data) {
    auto engine = templates::engine();
    return engine.render(source, json::parse(data));
  };

  // functions

  lua["exec"] = [](const string &command, const sol::as_table_t<vector<string>> &arguments, const string &path = "") {
    return io::shell(command, arguments.value(), path);
  };

  lua["fb"] = lua.create_table();
  lua["fb"]["compile"] = [&](const sol::as_table_t<vector<string>> &arguments) {
    return flatc(project_dir, arguments.value());
  };
  lua["fb"]["reflect"] = [&](const string &schema) -> auto {
    return flatc_reflection(filesystem::absolute(schema));
  };

  lua.safe_script(
    R"(
      --[[
        flatt "standard" library
      ]]

      -- add project directory to package dir
      if package.path ~= "" then
        package.path = package.path .. ";"
      end
      package.path = package.path .. flatt.project_dir .. "/?.lua"

      -- more
      -- ...
    )",
    on_script_error);

  auto result = lua.safe_script_file(project_file.string(), on_script_error);
  if (!result.valid()) {
    return -1;
  }

  if (result.get_type() == sol::type::number) {
    return result.get<int>();
  }

  return 0;
}

int main(int argc, const char *argv[]) {
  auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("console", console);
  logger->set_level(spdlog::level::trace);

  spdlog::set_default_logger(logger);
  spdlog::set_pattern("%^%v%$");
  spdlog::info("");
  spdlog::info(R"(
   __ _       _   _     _
  / _| | __ _| |_| |_  | |_   _  __ _
 | |_| |/ _` | __| __| | | | | |/ _` |
 |  _| | (_| | |_| |_ _| | |_| | (_| |
 |_| |_|\__,_|\__|\__(_)_|\__,_|\__,_|

)");

  spdlog::set_pattern(" %^%v%$");
#ifdef _DEBUG
  spdlog::set_level(spdlog::level::trace);
#else
  spdlog::set_level(spdlog::level::info);
#endif

  std::vector<std::string> arguments;
  std::copy(argv, argv + argc, std::back_inserter(arguments));

  if (arguments.size() < 2) {
    arguments.push_back("./flatt.lua");
  }

  auto file = arguments[1];
  arguments.erase(arguments.begin(), arguments.begin() + 2);

  return run_project(file, arguments);
}
