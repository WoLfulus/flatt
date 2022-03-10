
#include "utils/io.h"
#include "utils/strings.h"
#include "utils/templates.h"

// #include "reflection/reflection.h"

using namespace std;
using namespace std::filesystem;
using namespace inja;
using namespace nlohmann;

string executable_name(string name)
{
  return name
#ifdef _WIN32
         + ".exe"
#endif
    ;
}

path find_executable(string bin)
{
  auto cwd = io::get_current_executable_directory();
  auto name = executable_name(bin);

  auto current = cwd;
  while (true)
  {
    if (io::exists(current / name))
    {
      return current / name;
    }

    if (io::exists(current / "bin" / name))
    {
      return current / "bin" / name;
    }

    if (current.has_parent_path() && current != current.parent_path())
    {
      current = current.parent_path();
      continue;
    }

    return name;
  }
}

path find_flatc()
{
  return find_executable("flatc");
}

int flatc(const path &working_dir, const vector<string> arguments)
{
  return io::shell(find_flatc().string(), arguments, working_dir);
}

json flac_parse_attributes(const flatbuffers::Vector<flatbuffers::Offset<reflection::KeyValue>> *list)
{
  auto attributes = json({});
  if (list == nullptr)
  {
    return attributes;
  }

  for (int i = 0; i < list->Length(); i++)
  {
    auto entry = list->Get(i);
    attributes[entry->key()->str()] = entry->value()->str();
  }

  return attributes;
}

json flac_parse_documentation(const flatbuffers::Vector<flatbuffers::Offset<flatbuffers::String>> *list)
{
  auto documentation = json::array({});
  if (list == nullptr)
  {
    return documentation;
  }

  for (int i = 0; i < list->Length(); i++)
  {
    documentation.push_back(list->Get(i)->str());
  }

  return documentation;
}

optional<string> flatc_reflection(const path &file)
{
  string location = std::tmpnam(nullptr);
  io::create_directories(location);

  auto bfbs = path(location) / path(file).filename().replace_extension(".bfbs");

  auto arguments = vector<string>{
    "--binary", "--bfbs-gen-embed", "--bfbs-comments", "--bfbs-builtins",
    "-o",       location.c_str(),   "--schema",        file.string(),
  };

  auto status = flatc(io::get_current_executable_directory(), arguments);
  if (status != 0)
  {
    return {};
  }

  auto [exists, buffer] = io::read_file(bfbs);
  if (!exists)
  {
    return {};
  }

  auto &schema = *reflection::GetSchema(buffer.c_str());

  // auto contexts = set<string>({});
  // auto exchanges = map<string, set<string>>({});

  auto objects = schema.objects();

  auto data = json({});
  data["file_ident"] = schema.file_ident()->str();
  data["file_ext"] = schema.file_ext()->str();
  data["tables"] = json::array({});
  data["structs"] = json::array({});
  data["enums"] = json::array({});
  data["services"] = json::array({});
  data["advanced_features"] = json::array({});

  auto features = schema.advanced_features();
  if ((features & reflection::AdvancedFeatures::AdvancedArrayFeatures) != reflection::AdvancedFeatures::NONE)
  {
    data["advanced_features"].push_back("advanced_array_features");
  }
  if ((features & reflection::AdvancedFeatures::AdvancedUnionFeatures) != reflection::AdvancedFeatures::NONE)
  {
    data["advanced_features"].push_back("advanced_union_features");
  }
  if ((features & reflection::AdvancedFeatures::DefaultVectorsAndStrings) != reflection::AdvancedFeatures::NONE)
  {
    data["advanced_features"].push_back("defaualt_vectors_and_strings");
  }
  if ((features & reflection::AdvancedFeatures::OptionalScalars) != reflection::AdvancedFeatures::NONE)
  {
    data["advanced_features"].push_back("optional_scalars");
  }

  for (auto i = 0; i < objects->Length(); i++)
  {
    auto obj = objects->Get(i);
    auto name = obj->name()->str();

    auto type = json({
      { "id", entt::hashed_string::value(name.c_str()) },
      { "name", name.substr(name.find_last_of('.') + 1) },
      { "namespace", name.substr(0, name.find_last_of('.')) },
      { "minalign", obj->minalign() },
      { "attributes", flac_parse_attributes(obj->attributes()) },
      { "documentation", flac_parse_documentation(obj->documentation()) },
      { "fields",
        [&]() {
          auto fields = json::array({});

          auto list = obj->fields();
          if (list == nullptr)
          {
            return fields;
          }

          for (int i = 0; i < list->Length(); i++)
          {
            auto entry = list->Get(i);
            fields.push_back({
              { "id", entry->id() },
              { "name", entry->name()->str() },
              { "type", entry->type()->index() },
              { "offset", entry->offset() },
              { "key", entry->key() },
              { "deprecated", entry->deprecated() },
              { "optional", entry->optional() },
              { "required", entry->required() },
              { "attributes", flac_parse_attributes(entry->attributes()) },
              { "documentation", flac_parse_documentation(entry->documentation()) },
              // TODO: default value
            });
          }

          return fields;
        }() },
    });

    if (obj->is_struct())
    {
      data["structs"].push_back(type);
    }
    else
    {
      data["tables"].push_back(type);
    }
  }

  return data.dump(2);
}

auto on_script_error(lua_State *, sol::protected_function_result pfr)
{
  sol::error err = pfr;
  spdlog::error("script error: {}", err.what());
  return pfr;
}

int run_project(const path &entrypoint, const vector<string> &arguments)
{
  if (!io::exists(entrypoint))
  {
    spdlog::error("Unable to find project file: {}", entrypoint.string());
    return 1;
  }

  auto project_dir = io::absolute(entrypoint).parent_path();

  sol::state lua;
  lua.open_libraries(
    sol::lib::base, sol::lib::package, sol::lib::coroutine, sol::lib::string, sol::lib::os, sol::lib::math,
    sol::lib::table, sol::lib::debug, sol::lib::bit32, sol::lib::io, sol::lib::ffi, sol::lib::jit, sol::lib::utf8);

  // flatt

  lua["flatt"] = lua.create_table();

  // variables

  lua["flatt"]["vars"] = lua.create_table();
  lua["flatt"]["vars"]["flatt_dir"] = io::get_current_executable_directory().string();
  lua["flatt"]["vars"]["project_dir"] = project_dir.string();
  lua["flatt"]["vars"]["argv"] = arguments;

  // logs

  lua["flatt"]["log"] = lua.create_table();
  lua["flatt"]["log"]["set_level"] = [&](const std::string &value) {
    spdlog::set_level(spdlog::level::from_str(value));
  };
  lua["flatt"]["log"]["get_level"] = [&]() { return spdlog::level::to_string_view(spdlog::get_level()); };
  lua["flatt"]["log"]["trace"] = [](const std::string &msg) { spdlog::trace(msg); };
  lua["flatt"]["log"]["debug"] = [](const std::string &msg) { spdlog::debug(msg); };
  lua["flatt"]["log"]["info"] = [](const std::string &msg) { spdlog::info(msg); };
  lua["flatt"]["log"]["warn"] = [](const std::string &msg) { spdlog::warn(msg); };
  lua["flatt"]["log"]["error"] = [](const std::string &msg) { spdlog::error(msg); };
  lua["flatt"]["log"]["critical"] = [](const std::string &msg) { spdlog::critical(msg); };

  // files

  lua["flatt"]["file"] = lua.create_table();
  lua["flatt"]["file"]["exists"] = [](const string &file) { return io::exists(file); };
  lua["flatt"]["file"]["read"] = [](const string &file) {
    auto [success, content] = io::read_file(file);
    return content;
  };
  lua["flatt"]["file"]["write"] = [](const string &file, const string &content) {
    return io::write_file(file, content);
  };
  lua["flatt"]["file"]["hash"] = [](const string &file) { return io::hash_file(file); };

  // dir

  lua["flatt"]["dir"] = lua.create_table();
  lua["flatt"]["dir"]["hash"] = [](const std::string &path) { return io::hash_dir(path); };
  lua["flatt"]["dir"]["list_files"] = [](const std::string &path) {
    auto files = io::list_files(path);
    auto paths = vector<string>{};
    for (auto file : files)
    {
      paths.push_back(file.string());
    }
    return sol::as_table(paths);
  };

  lua["flatt"]["dir"]["list_dirs"] = [](const std::string &path) {
    auto files = io::list_dirs(path);
    auto paths = vector<string>{};
    for (auto file : files)
    {
      paths.push_back(file.string());
    }
    return sol::as_table(paths);
  };

  // string

  lua["flatt"]["string"] = lua.create_table();
  lua["flatt"]["string"]["pad_left"] = sol::overload(
    [](const std::string &value, const int length) { return str::padleft(value, length, " "); },
    [](const std::string &value, const int length, const std::string &pad) {
      return str::padleft(value, length, pad);
    });
  lua["flatt"]["string"]["pad_right"] = sol::overload(
    [](const std::string &value, const int length) { return str::padleft(value, length, " "); },
    [](const std::string &value, const int length, const std::string &pad) {
      return str::padright(value, length, pad);
    });
  lua["flatt"]["string"]["ends_with"] = [](const std::string &value, const std::string &match) {
    return str::ends_with(value, match);
  };
  lua["flatt"]["string"]["starts_with"] = [](const std::string &value, const std::string &match) {
    return str::starts_with(value, match);
  };
  lua["flatt"]["string"]["split"] = sol::overload(
    [&](const std::string &value, const std::string &delimiter) { return sol::as_table(str::split(value, delimiter)); },
    [&](const std::string &value, const std::string &delimiter, int limit) {
      return sol::as_table(str::split(value, delimiter, limit));
    });
  lua["flatt"]["string"]["trim"] = [](const std::string &value) { return str::trim_copy(value); };
  lua["flatt"]["string"]["trim_left"] = [](const std::string &value) { return str::trim_left_copy(value); };
  lua["flatt"]["string"]["trim_right"] = [](const std::string &value) { return str::trim_right_copy(value); };
  lua["flatt"]["string"]["explode"] = [&](const std::string &value) { return sol::as_table(str::explode(value)); };
  lua["flatt"]["string"]["join"] = [](const sol::as_table_t<vector<string>> &parts, const string &delim = ",") {
    return str::join(parts.value(), delim);
  };
  lua["flatt"]["string"]["lower_case"] = [](const string &value) { return str::lower_case(value); };
  lua["flatt"]["string"]["upper_case"] = [](const string &value) { return str::upper_case(value); };
  lua["flatt"]["string"]["ucfirst"] = [](const string &value) { return str::ucfirst(value); };
  lua["flatt"]["string"]["lcfirst"] = [](const string &value) { return str::lcfirst(value); };
  lua["flatt"]["string"]["snake_case"] = [](const string &value) { return str::snake_case(value); };
  lua["flatt"]["string"]["kebab_case"] = [](const string &value) { return str::kebab_case(value); };
  lua["flatt"]["string"]["pascal_case"] = [](const string &value) { return str::pascal_case(value); };
  lua["flatt"]["string"]["camel_case"] = [](const string &value) { return str::camel_case(value); };
  lua["flatt"]["string"]["const_case"] = [](const string &value) { return str::const_case(value); };
  lua["flatt"]["string"]["train_case"] = [](const string &value) { return str::train_case(value); };
  lua["flatt"]["string"]["ada_case"] = [](const string &value) { return str::ada_case(value); };
  lua["flatt"]["string"]["cobol_case"] = [](const string &value) { return str::cobol_case(value); };
  lua["flatt"]["string"]["dot_case"] = [](const string &value) { return str::dot_case(value); };
  lua["flatt"]["string"]["path_case"] = [](const string &value) { return str::path_case(value); };
  lua["flatt"]["string"]["space_case"] = [](const string &value) { return str::space_case(value); };
  lua["flatt"]["string"]["capital_case"] = [](const string &value) { return str::capital_case(value); };
  lua["flatt"]["string"]["cpp_case"] = [](const string &value) { return str::cpp_case(value); };

  // templates

  lua["flatt"]["template"] = lua.create_table();
  lua["flatt"]["template"]["render_string"] = [](const string &source, const string &data) {
    auto engine = templates::engine();
    return engine.render(source, json::parse(data));
  };

  // functions

  lua["flatt"]["flatc"] = [&](const sol::as_table_t<vector<string>> &arguments) {
    return flatc(project_dir, arguments.value());
  };
  lua["flatt"]["shell"] =
    [](const string &command, const sol::as_table_t<vector<string>> &arguments, const string &path = "") {
      return io::shell(command, arguments.value(), path);
    };
  lua["flatt"]["reflect"] = [&](const string &schema) -> auto
  {
    return flatc_reflection(io::absolute(schema));
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
      package.path = package.path .. flatt.vars.project_dir .. "/?.lua"

      -- more
      -- ...
    )",
    on_script_error);

  auto result = lua.safe_script_file(entrypoint.string(), on_script_error);
  if (!result.valid())
  {
    return -1;
  }

  if (result.get_type() == sol::type::number)
  {
    return result.get<int>();
  }

  return 0;
}

int main(int argc, const char *argv[])
{
  auto console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto logger = std::make_shared<spdlog::logger>("console", console);
  logger->set_level(spdlog::level::trace);

  spdlog::set_default_logger(logger);
  spdlog::set_pattern("%^%v%$");
  spdlog::info("");
  spdlog::info(".-..  .-..-..-. \n"
               "|- |  |-| |  |  \n"
               "'  `-'` ' '  '  \n");
  spdlog::set_pattern(" %^%v%$");
#ifdef _DEBUG
  spdlog::set_level(spdlog::level::trace);
#else
  spdlog::set_level(spdlog::level::info);
#endif

  std::vector<std::string> arguments;
  std::copy(argv, argv + argc, std::back_inserter(arguments));

  if (arguments.size() < 2)
  {
    spdlog::error("Usage: {} \"path/to/project.lua\" [args]", arguments[0]);
    return 1;
  }

  arguments.erase(arguments.begin(), arguments.begin() + 2);
  return run_project(argv[1], arguments);
}
