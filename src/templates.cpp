#include <iomanip>

#include "./strings.hpp"
#include "./templates.hpp"

using namespace std;
using namespace nlohmann;
using namespace inja;

json templates::unique(Arguments &args) {
  auto result = json::array({});
  auto prop = *args[0];
  auto values = *args[1];

  auto parts = str::split(prop, '.');

  map<json, json> uniques;

  for (auto value : values) {
    auto found = true;
    auto current = value;

    for (auto part : parts) {
      auto attr = current.find(part);
      if (attr == current.end()) {
        found = false;
        break;
      }
      current = *attr;
    }

    if (found) {
      if (uniques.find(current) == uniques.end()) {
        uniques[current] = value;
      }
    }
  }

  for (auto [_, entry] : uniques) {
    result.push_back(entry);
  }
  return result;
}

json templates::filter(Arguments &args, bool invert) {
  auto result = json::array({});
  if (args.size() == 0) {
    return result;
  } else if (args.size() == 1) {
    auto values = *args[0];
    if (!values.is_array()) {
      return result;
    }
    return values;
  } else if (args.size() == 2) {
    auto values = *args[0];
    if (!values.is_array()) {
      return result;
    }

    auto prop = *args[1];
    auto parts = str::split(prop, '.');
    for (auto value : values) {
      auto matches = true;
      auto current = value;

      for (auto part : parts) {
        auto attr = current.find(part);
        if (attr == current.end()) {
          matches = false;
          break;
        }
        current = *attr;
      }

      if ((matches && !invert) || (!matches && invert)) {
        result.push_back(value);
      }
    }
  } else if (args.size() == 3) {
    auto entries = *args[0];
    auto prop = *args[1];
    auto value = *args[2];

    if (!entries.is_array()) {
      return result;
    }

    auto parts = str::split(prop, '.');
    for (auto entry : entries) {
      auto matches = true;
      auto current = entry;

      for (auto part : parts) {
        auto attr = current.find(part);
        if (attr == current.end()) {
          matches = false;
          break;
        }

        current = *attr;
      }

      if (current.is_array()) {
        matches = false;
        for (auto entry : current) {
          if (entry == value) {
            matches = true;
            break;
          }
        }
      } else {
        matches = matches && (current == value);
      }

      if ((matches && !invert) || (!matches && invert)) {
        result.push_back(entry);
      }
    }
  }

  return result;
}

json templates::trace(Arguments &args) {
  inja::Environment env;
  inja::Template tpl = env.parse("{{ data }}");

  for (auto i = 0; i < args.size(); i++) {
    auto arg = *args[i];

    json data = json::object();
    data["data"] = arg;

    spdlog::trace("{}: {}", i, env.render(tpl, data).c_str());
  }
  return json(nullptr);
}

json templates::debug(Arguments &args) {
  inja::Environment env;
  inja::Template tpl = env.parse("{{ data }}");

  for (auto i = 0; i < args.size(); i++) {
    auto arg = *args[i];

    json data = json::object();
    data["data"] = arg;

    spdlog::debug("{}: {}", i, env.render(tpl, data).c_str());
  }
  return json(nullptr);
}

json templates::info(Arguments &args) {
  inja::Environment env;
  inja::Template tpl = env.parse("{{ data }}");

  for (auto i = 0; i < args.size(); i++) {
    auto arg = *args[i];

    json data = json::object();
    data["data"] = arg;

    spdlog::info("{}: {}", i, env.render(tpl, data).c_str());
  }
  return json(nullptr);
}

json templates::warn(Arguments &args) {
  inja::Environment env;
  inja::Template tpl = env.parse("{{ data }}");

  for (auto i = 0; i < args.size(); i++) {
    auto arg = *args[i];

    json data = json::object();
    data["data"] = arg;

    spdlog::warn("{}: {}", i, env.render(tpl, data).c_str());
  }
  return json(nullptr);
}

json templates::error(Arguments &args) {
  inja::Environment env;
  inja::Template tpl = env.parse("{{ data }}");

  for (auto i = 0; i < args.size(); i++) {
    auto arg = *args[i];

    json data = json::object();
    data["data"] = arg;

    spdlog::error("{}: {}", i, env.render(tpl, data).c_str());
  }
  return json(nullptr);
}

json templates::critical(Arguments &args) {
  inja::Environment env;
  inja::Template tpl = env.parse("{{ data }}");

  for (auto i = 0; i < args.size(); i++) {
    auto arg = *args[i];

    json data = json::object();
    data["data"] = arg;

    spdlog::critical("{}: {}", i, env.render(tpl, data).c_str());
  }
  return json(nullptr);
}

json templates::merge(Arguments &args) {
  auto m = json::array({});
  for (auto i = 0; i < args.size(); i++) {
    auto arg = *args[i];
    if (arg.is_array()) {
      for (auto item : arg) {
        m.push_back(item);
      }
    } else {
      m.push_back(arg);
    }
  }
  return m;
}

json templates::to_ada(Arguments &args) {
  return str::to_ada(*args[0]);
}

json templates::to_camel(Arguments &args) {
  return str::to_camel(*args[0]);
}

json templates::to_capital(Arguments &args) {
  return str::to_capital(*args[0]);
}

json templates::to_cobol(Arguments &args) {
  return str::to_cobol(*args[0]);
}

json templates::to_const(Arguments &args) {
  return str::to_const(*args[0]);
}

json templates::to_cpp(Arguments &args) {
  return str::to_cpp(*args[0]);
}

json templates::to_dot(Arguments &args) {
  return str::to_dot(*args[0]);
}

json templates::to_kebab(Arguments &args) {
  return str::to_kebab(*args[0]);
}

json templates::to_lower(Arguments &args) {
  return str::to_lower(*args[0]);
}

json templates::to_lower_first(Arguments &args) {
  return str::to_lower_first(*args[0]);
}

json templates::to_pascal(Arguments &args) {
  return str::to_pascal(*args[0]);
}

json templates::to_path(Arguments &args) {
  return str::to_path(*args[0]);
}

json templates::to_snake(Arguments &args) {
  return str::to_snake(*args[0]);
}

json templates::to_space(Arguments &args) {
  return str::to_space(*args[0]);
}

json templates::to_train(Arguments &args) {
  return str::to_train(*args[0]);
}

json templates::to_upper(Arguments &args) {
  return str::to_upper(*args[0]);
}

json templates::to_upper_first(Arguments &args) {
  return str::to_upper_first(*args[0]);
}

json templates::replace(Arguments &args) {
  return str::replace_all(*args[0], *args[1], *args[2]);
}

json templates::padleft(Arguments &args) {
  return str::padleft(*args[0], *args[1], *args[2]);
}

json templates::padright(Arguments &args) {
  return str::padright(*args[0], *args[1], *args[2]);
}

json templates::sort_by(Arguments &args) {
  auto list = *args[1];
  auto prop = "/" + str::replace_all(*args[0], "\\.", "/");
  auto sorted = json::array();

  auto prop_ptr = json::json_pointer(prop);

  using iterator = decltype(*list.begin());

  const auto sorter = [&prop_ptr](const iterator a, const iterator b) {
    std::string val1 = a[prop_ptr];
    std::string val2 = b[prop_ptr];
    return val1.compare(val2) < 0;
  };

  std::sort(list.begin(), list.end(), sorter);

  return list;
}

json templates::to_hex(Arguments &args) {
  auto num = *args[0];
  if (num.is_number_float()) {
    return str::to_hex<float>(num);
  } else if (num.is_number_unsigned()) {
    return str::to_hex<unsigned int>(num);
  } else if (num.is_number_integer() || num.is_number()) {
    return str::to_hex<int>(num);
  }
  return num;
}

Environment templates::engine(Environment &env) {
  // env.set_trim_blocks(true);
  // env.set_lstrip_blocks(true);

  // strings
  env.add_callback("padright", 3, padright);
  env.add_callback("padleft", 3, padleft);

  env.add_callback("lower", 1, to_lower);
  env.add_callback("upper", 1, to_upper);
  env.add_callback("upper_first", 1, to_upper_first);
  env.add_callback("lower_first", 1, to_lower_first);
  env.add_callback("snake", 1, to_snake);
  env.add_callback("kebab", 1, to_kebab);
  env.add_callback("pascal", 1, to_pascal);
  env.add_callback("camel", 1, to_camel);
  env.add_callback("const", 1, to_const);
  env.add_callback("train", 1, to_train);
  env.add_callback("cobol", 1, to_cobol);
  env.add_callback("dot", 1, to_dot);
  env.add_callback("path", 1, to_path);
  env.add_callback("space", 1, to_space);
  env.add_callback("capital", 1, to_capital);
  env.add_callback("cpp", 1, to_cpp);
  env.add_callback("hex", 1, to_hex);

  env.add_callback("replace", 3, replace);

  // log
  env.add_callback("trace", trace);
  env.add_callback("debug", debug);
  env.add_callback("info", info);
  env.add_callback("warn", warn);
  env.add_callback("error", error);
  env.add_callback("critical", critical);

  // arrays
  env.add_callback("unique", 2, unique);
  env.add_callback("merge", merge);
  env.add_callback("filter", [](Arguments &args) {
    return filter(args, false);
  });
  env.add_callback("ifilter", [](Arguments &args) {
    return filter(args, true);
  });
  env.add_callback("sort_by", 2, sort_by);

  return env;
}

Environment templates::engine(std::filesystem::path template_dir) {
  auto env = Environment{ template_dir.string() };
  return engine(env);
}

Environment templates::engine() {
  auto env = Environment();
  return engine(env);
}
