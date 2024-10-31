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
#include "modules.hpp"
#include "json.hpp"

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
  auto res = io::shell(find_flatc().string(), arguments, working_dir);
  spdlog::trace("flatc({})\n----------\n{}\n----------\n{}\n----------\n", res.code, res.out, res.err);
  return res.code;
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

auto flatc_reflection(const path &file) {
  string location = std::tmpnam(nullptr);
  filesystem::create_directories(location);

  auto bfbs = path(location) / path(file).filename().replace_extension(".bfbs");

  auto arguments = vector<string>{
    "--binary",
    "--bfbs-gen-embed",
    "--bfbs-comments",
    "--bfbs-builtins",
    "-o",
    location.c_str(),
    "--schema",
    file.string(),
  };

  auto status = flatc(io::get_current_executable_directory(), arguments);
  if (status != 0) {
    return json(nullptr);
  }

  auto [exists, buffer] = io::read_file(bfbs);
  if (!exists) {
    return json(nullptr);
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

  return data;
}

auto on_script_error(lua_State *, sol::protected_function_result pfr) {
  sol::error err = pfr;
  spdlog::error("script error:\n{}\n\n", err.what());
  return pfr;
}

int run_project(const path &entrypoint, const vector<string> &arguments) {
  auto project_found = false;
  auto project_file = path(entrypoint);
  auto project_files = vector<path>{};

  if (filesystem::is_directory(project_file)) {
    project_files.push_back(project_file / "flatt.lua");
    project_files.push_back(project_file / "flatt.tl");
    project_files.push_back(project_file / "init.lua");
    project_files.push_back(project_file / "init.tl");
    project_files.push_back(project_file / "main.lua");
    project_files.push_back(project_file / "main.tl");
  } else {
    project_files.push_back(path(project_file.string() + ".lua"));
    project_files.push_back(path(project_file.string() + ".tl"));
    project_files.push_back(project_file);
  }

  for (auto file : project_files) {
    if (filesystem::exists(file)) {
      project_found = true;
      project_file = file;
      break;
    }
  }

  if (!filesystem::exists(project_file) || !project_found) {
    spdlog::error("Unable to find project file, looked into:");
    for (auto file : project_files) {
      spdlog::error("  - {}", file.string());
    }
    return 1;
  }

  project_file = filesystem::absolute(project_file);

  auto project_dir = project_file.parent_path();

  if (project_file.extension() == ".lua" || project_file.extension() == ".tl") {
    project_file = project_file.replace_extension("");
  }

  project_file = "./" + filesystem::relative(project_file, project_dir).string();

#ifdef _WIN32
  SetCurrentDirectory(project_dir.string().c_str());
#else
  chdir(project_dir.string().c_str());
#endif

  sol::state lua;
  lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::coroutine, sol::lib::string, sol::lib::os,
    sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::bit32, sol::lib::io, sol::lib::ffi, sol::lib::jit,
    sol::lib::utf8);

  // variables

  lua["flatt_executable"] = io::get_current_executable().string();
  lua["flatt_directory"] = io::get_current_executable_directory().string();
  lua["flatt_args"] = arguments;
  lua["flatt_argv"] = arguments;
  lua["flatt_argc"] = arguments.size();

  lua["flatt_project_directory"] = project_dir.string();
  lua["flatt_project_root"] = project_dir.string();
  lua["flatt_project_file"] = project_file.string();

  // logs

  lua["flatt_logger_set_level"] = [&](const std::string &value) {
    spdlog::set_level(spdlog::level::from_str(value));
  };

  lua["flatt_logger_get_level"] = [&]() {
    return spdlog::level::to_string_view(spdlog::get_level());
  };

  lua["flatt_logger_trace"] = [&](const std::string &msg) {
    spdlog::trace(msg);
  };

  lua["flatt_logger_debug"] = [&](const std::string &msg) {
    spdlog::debug(msg);
  };

  lua["flatt_logger_info"] = [&](const std::string &msg) {
    spdlog::info(msg);
  };

  lua["flatt_logger_warn"] = [&](const std::string &msg) {
    spdlog::warn(msg);
  };

  lua["flatt_logger_error"] = [&](const std::string &msg) {
    spdlog::error(msg);
  };

  lua["flatt_logger_critical"] = [&](const std::string &msg) {
    spdlog::critical(msg);
  };

  lua["flatt_logger_fatal"] = [&](const std::string &msg) {
    spdlog::critical(msg);
  };

  // files

  lua["flatt_fs_exists"] = [&](const string &file) {
    return filesystem::exists(file);
  };

  lua["flatt_fs_is_dir"] = [&](const string &value) {
    return (filesystem::exists(value) && filesystem::is_directory(value));
  };

  lua["flatt_fs_is_file"] = [&](const string &value) {
    return (filesystem::exists(value) && !filesystem::is_directory(value));
  };

  lua["flatt_fs_read_file"] = [&](const string &file) {
    auto [success, content] = io::read_file(file);
    return content;
  };

  lua["flatt_fs_remove_file"] = [&](const string &file) {
    if (lua["flatt_fs_is_file"](file)) {
      return filesystem::remove(file);
    } else {
      return false;
    }
  };

  lua["flatt_fs_write_file"] = [&](const string &file, const string &content) {
    return io::write_file(file, content);
  };

  lua["flatt_fs_write_json"] = [&](const string &file, const sol::object &content) {
    return io::write_file(file, json(content).dump(2));
  };

  lua["flatt_fs_hash_dir"] = [&](const std::string &path) {
    return io::hash_dir(path);
  };

  lua["flatt_fs_hash_file"] = [&](const string &file) {
    return io::hash_file(file);
  };

  lua["flatt_fs_hash"] = [&](const string &file) {
    if (filesystem::is_directory(file)) {
      return io::hash_dir(file);
    } else {
      return io::hash_file(file);
    }
  };

  lua["flatt_fs_list_files"] = [&](const std::string &path) {
    auto files = io::list_files(path);
    auto paths = vector<string>{};
    for (auto file : files) {
      paths.push_back(file.string());
    }
    return sol::as_table(paths);
  };

  lua["flatt_fs_list_dirs"] = [&](const std::string &path) {
    auto values = io::list_dirs(path);
    auto paths = vector<string>{};
    for (auto value : values) {
      paths.push_back(value.string());
    }
    return sol::as_table(paths);
  };

  lua["flatt_fs_list"] = [&](const std::string &path) {
    auto paths = vector<string>{};

    auto values = io::list_dirs(path);
    for (auto value : values) {
      paths.push_back(value.string());
    }

    values = io::list_files(path);
    for (auto value : values) {
      paths.push_back(value.string());
    }
    return sol::as_table(paths);
  };

  // string

  lua["flatt_str_pad_left"] = [&](const std::string &value, const int length, const std::optional<std::string> &pad) {
    auto padv = pad.has_value() ? pad.value() : " ";
    return str::padleft(value, length, padv);
  };

  lua["flatt_str_pad_right"] = [&](const std::string &value, const int length, const std::optional<std::string> &pad) {
    auto padv = pad.has_value() ? pad.value() : " ";
    return str::padright(value, length, padv);
  };

  lua["flatt_str_tokenize"] = [&](const std::string &value) {
    return sol::as_table(str::tokenize(value));
  };

  auto flatt_str_split = [&](const std::string &value, const std::string &delimiter) {
    return sol::as_table(str::split(value, delimiter));
  };

  auto flatt_str_split_limit = [&](const std::string &value, const std::string &delimiter, int limit) {
    return sol::as_table(str::split(value, delimiter, limit));
  };

  lua["flatt_str_split"] = sol::overload(flatt_str_split, flatt_str_split_limit);

  lua["flatt_str_ends_with"] = [&](const std::string &value, const std::string &match) {
    return str::ends_with(value, match);
  };

  lua["flatt_str_starts_with"] = [&](const std::string &value, const std::string &match) {
    return str::starts_with(value, match);
  };

  lua["flatt_str_trim"] = [&](const std::string &value) {
    return str::trim_copy(value);
  };

  lua["flatt_str_trim_left"] = [&](const std::string &value) {
    return str::trim_left_copy(value);
  };

  lua["flatt_str_trim_right"] = [&](const std::string &value) {
    return str::trim_right_copy(value);
  };

  lua["flatt_str_join"] = [&](const sol::as_table_t<vector<string>> &parts, const string &delim = ",") {
    return str::join(parts.value(), delim);
  };

  lua["flatt_str_lower"] = [&](const string &value) {
    return str::to_lower(value);
  };

  lua["flatt_str_upper"] = [&](const string &value) {
    return str::to_upper(value);
  };

  lua["flatt_str_upper_first"] = [&](const string &value) {
    return str::to_upper_first(value);
  };

  lua["flatt_str_lower_first"] = [&](const string &value) {
    return str::to_lower_first(value);
  };

  lua["flatt_str_snake"] = [&](const string &value) {
    return str::to_snake(value);
  };

  lua["flatt_str_kebab"] = [&](const string &value) {
    return str::to_kebab(value);
  };

  lua["flatt_str_pascal"] = [&](const string &value) {
    return str::to_pascal(value);
  };

  lua["flatt_str_camel"] = [&](const string &value) {
    return str::to_camel(value);
  };

  lua["flatt_str_const"] = [&](const string &value) {
    return str::to_const(value);
  };

  lua["flatt_str_train"] = [&](const string &value) {
    return str::to_train(value);
  };

  lua["flatt_str_ada"] = [&](const string &value) {
    return str::to_ada(value);
  };

  lua["flatt_str_cobol"] = [&](const string &value) {
    return str::to_cobol(value);
  };

  lua["flatt_str_dot"] = [&](const string &value) {
    return str::to_dot(value);
  };

  lua["flatt_str_path"] = [&](const string &value) {
    return str::to_path(value);
  };

  lua["flatt_str_space"] = [&](const string &value) {
    return str::to_space(value);
  };

  lua["flatt_str_capital"] = [&](const string &value) {
    return str::to_capital(value);
  };

  lua["flatt_str_cpp"] = [&](const string &value) {
    return str::to_cpp(value);
  };

  // templates

  lua["flatt_template_render"] = [&](const std::string &file, const std::string &data) {
    auto engine = templates::engine();
    return engine.render(file, json::parse(data));
  };

  // functions

  lua["flatt_exec"] = [&](const string &command, const sol::as_table_t<vector<string>> &arguments,
                        const std::optional<string> &path) {
    auto res = io::shell(command, arguments.value(), path.has_value() ? path.value() : "");
    auto table = lua.create_table();
    table["code"] = res.code;
    table["success"] = res.code == 0;
    table["output"] = res.out;
    table["error"] = res.err;
    return table;
  };

  lua["flatbuffers_compile"] = [&](const sol::as_table_t<vector<string>> &arguments) {
    return flatc(project_dir, arguments.value());
  };

  lua["flatbuffers_reflect"] = [&](const string &schema) -> auto {
    return to_sol(lua, flatc_reflection(filesystem::absolute(schema)));
  };

  register_embedded_modules(lua);

  lua["__main__"] = project_file.string();

  // TODO: move all this shit to modules folder

  auto result = lua.safe_script(R"(
    if package.path ~= "" then
      package.path = package.path .. ";"
    end
    package.path = package.path .. ";" .. flatt_project_directory .. "/?.lua;"
  )",
    on_script_error);
  if (!result.valid()) {
    return -1;
  }

  result = lua.safe_script(R"(
    do
      local loader = require("luarocks.loader")

      local __cfg = require("luarocks.core.cfg")
      __cfg.init({
        project_dir = flatt_project_root
      })

      local __fs = require("luarocks.fs")
      __fs.init()

      local lr_path = require("luarocks.path")
      local lr_util = require("luarocks.util")

      lr_path.use_tree(flatt_project_root.."/lua_modules")
      lr_path.add_to_package_paths(flatt_project_root.."/lua_modules")

      local cmd = require("luarocks.cmd")
    end

    function luarocks_supressed(callback, ...)

      local fs = require("flatt.fs")

      local _stderr = io.stderr
      local _stdout = io.stdout

      local result = callback(...)

      io.stdout = _stdout
      io.stderr = _stderr

      return result
    end

    function luarocks_command(name, args)
      args = args or {}
      return luarocks_supressed(function()
        local cmd = require("luarocks.cmd")
        return flatt_exec("luarocks", { name, table.unpack(args) })
      end)
    end

    function luarocks_command_inprocess(name, args)
      args = args or {}
      return luarocks_supressed(function()
        local cmd = require("luarocks.cmd")
        return cmd.run_command("description", { [name] = "luarocks.cmd."..name }, "luarocks.cmd.external", name, table.unpack(args))
      end)
    end

    local __luarocks__ = false

    _G["luarocks_require"] = function()
      error("luarocks not enabled. call luarocks_enable() first")
    end

    function luarocks_enable()
      local fs = require("flatt.fs")
      local log = require("flatt.logger")
      local strings = require("flatt.strings")
      local locks = require("luarocks.deplocks")
      local persist = require("luarocks.persist")

      log.info("Enabling luarocks ...")

      if not flatt_exec("luarocks", { "init", "--no-wrapper-scripts", "--no-gitignore", "--lua-versions=5.4", "--project-tree="..flatt_project_root.."/lua_modules" }) then
        error("Failed to initialize luarocks")
      end

      luarocks_supressed(function()
        -- luarocks_command_inprocess("path", {})
      end)

      local ok = false
      local rocklist = {
        dependencies = {}
      }
      if fs.is_file("luarocks.lock") then
        ok = locks.load("flatt_project", flatt_project_directory)
        if ok then
          for k, v in locks.each("dependencies") or pairs({}) do
            rocklist.dependencies[k] = v
          end
        end
      else
        ok = locks.init("flatt_project", flatt_project_directory)
      end

      if not rocklist then
        rocklist = {
          dependencies = {}
        }
      else
        if not rocklist.dependencies then
          rocklist.dependencies = {}
        end
      end

      _G["luarocks_require"] = function (name, modname)
        local fs = require("flatt.fs")
        local project = require("flatt.project")

        if not name then
          error("luarocks_require() requires a module name")
        end

        log.trace("Trying to require(\""..name.."\") ")
        local __package__, __module__ = pcall(require, name)
        if __package__ then
          return __module__
        end
        log.trace("  ...failed")

        if not modname then
          modname = name
          local modname_end = string.find(name, "\\.")
          if modname_end then
            modname = name:sub(1, modname_end - 1)
          end

          if name ~= modname then
            log.trace("Trying to require(\""..modname.."\") ")
            local __package__, __module__ = pcall(require, modname)
            if __package__ then
              return __module__
            end
            log.trace("  ...failed")
          end
        end

        log.trace("Trying to install "..modname)

        function luarocks_module_version(name)
          local ret = luarocks_command("show", { modname, "--mversion", "--project-tree="..flatt_project_root.."/lua_modules" })
          if ret.success then
            return strings.trim(ret.out)
          end
          return nil
        end

        local depname = modname
        local depver = luarocks_module_version(depname)

        if depver == nil then
          local ret = luarocks_command("install", { modname, "--pin", "--project-tree="..flatt_project_root.."/lua_modules" })
          if not ret.success then
            log.error("Failed to install "..modname.." (exit = "..ret.code..") ")
            return nil
          else
            depver = luarocks_module_version(depname)
            log.info("Installed "..depname.." (version: "..depver..") ")
            rocklist["dependencies"][depname] = depver
            persist.save_as_module("luarocks.lock", rocklist)
          end
        end

        return require(modname)
      end
    end

    )",
    on_script_error);
  if (!result.valid()) {
    return -1;
  }

  result = lua.safe_script(R"(
    require("tl").loader()
    return require(__main__)
  )",
    on_script_error);
  if (!result.valid()) {
    return -1;
  }

  if (result.get_type() == sol::type::number) {
    return result.get<int>();
  }

  return 0;
}

int main(int argc, const char *argv[]) {

  io::set_current_executable(argv[0]);

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
    arguments.push_back(".");
  }

  auto file = arguments[1];
  arguments.erase(arguments.begin(), arguments.begin() + 2);

  return run_project(file, arguments);
}
