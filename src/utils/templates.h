#pragma once

namespace templates {

  using namespace std;
  using namespace inja;

  json unique(Arguments &args) {
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

  json filter(Arguments &args, bool invert = false) {
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

  json trace(Arguments &args) {
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

  json debug(Arguments &args) {
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

  json info(Arguments &args) {
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

  json warn(Arguments &args) {
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

  json error(Arguments &args) {
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

  json critical(Arguments &args) {
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

  json merge(Arguments &args) {
    auto m = json::array({});
    for (auto i = 0; i < args.size(); i++) {
      auto arg = *args[i];
      if (arg.is_array()) {
        for (auto item : arg) {
          m.push_back(item);
        }
      }
    }
    return m;
  }

  json ada_case(Arguments &args) {
    return str::ada_case(*args[0]);
  }

  json camel_case(Arguments &args) {
    return str::camel_case(*args[0]);
  }

  json capital_case(Arguments &args) {
    return str::capital_case(*args[0]);
  }

  json cobol_case(Arguments &args) {
    return str::cobol_case(*args[0]);
  }

  json const_case(Arguments &args) {
    return str::const_case(*args[0]);
  }

  json cpp_case(Arguments &args) {
    return str::cpp_case(*args[0]);
  }

  json dot_case(Arguments &args) {
    return str::dot_case(*args[0]);
  }

  json kebab_case(Arguments &args) {
    return str::kebab_case(*args[0]);
  }

  json lower_case(Arguments &args) {
    return str::lower_case(*args[0]);
  }

  json pascal_case(Arguments &args) {
    return str::pascal_case(*args[0]);
  }

  json path_case(Arguments &args) {
    return str::path_case(*args[0]);
  }

  json snake_case(Arguments &args) {
    return str::snake_case(*args[0]);
  }

  json space_case(Arguments &args) {
    return str::space_case(*args[0]);
  }

  json train_case(Arguments &args) {
    return str::train_case(*args[0]);
  }

  json upper_case(Arguments &args) {
    return str::upper_case(*args[0]);
  }

  json replace(Arguments &args) {
    return str::replace_all(*args[0], *args[1], *args[2]);
  }

  json padleft(Arguments &args) {
    return str::padleft(*args[0], *args[1], *args[2]);
  }

  json padright(Arguments &args) {
    return str::padright(*args[0], *args[1], *args[2]);
  }

  json sort_by(Arguments &args) {
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

  json hex(Arguments &args) {
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

  Environment engine(Environment &env) {
    env.set_trim_blocks(true);
    env.set_lstrip_blocks(true);

    // strings
    env.add_callback("padright", 3, padright);
    env.add_callback("padleft", 3, padleft);
    env.add_callback("ada_case", 1, ada_case);
    env.add_callback("camel_case", 1, camel_case);
    env.add_callback("capital_case", 1, capital_case);
    env.add_callback("cobol_case", 1, cobol_case);
    env.add_callback("const_case", 1, const_case);
    env.add_callback("cpp_case", 1, cpp_case);
    env.add_callback("dot_case", 1, dot_case);
    env.add_callback("kebab_case", 1, kebab_case);
    env.add_callback("lower_case", 1, lower_case);
    env.add_callback("pascal_case", 1, pascal_case);
    env.add_callback("path_case", 1, path_case);
    env.add_callback("snake_case", 1, snake_case);
    env.add_callback("space_case", 1, space_case);
    env.add_callback("train_case", 1, train_case);
    env.add_callback("upper_case", 1, upper_case);
    env.add_callback("replace", 3, replace);
    env.add_callback("hex", 1, hex);

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

  Environment engine(std::filesystem::path template_dir) {
    auto env = Environment{ template_dir.string() };
    return engine(env);
  }

  Environment engine() {
    auto env = Environment();
    return engine(env);
  }

} // namespace templates
