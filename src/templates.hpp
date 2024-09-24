#pragma once

#include <filesystem>

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>

namespace templates {

  nlohmann::json unique(inja::Arguments &args);
  nlohmann::json filter(inja::Arguments &args, bool invert = false);
  nlohmann::json trace(inja::Arguments &args);
  nlohmann::json debug(inja::Arguments &args);
  nlohmann::json info(inja::Arguments &args);
  nlohmann::json warn(inja::Arguments &args);
  nlohmann::json error(inja::Arguments &args);
  nlohmann::json critical(inja::Arguments &args);
  nlohmann::json merge(inja::Arguments &args);
  nlohmann::json to_ada(inja::Arguments &args);
  nlohmann::json to_camel(inja::Arguments &args);
  nlohmann::json to_capital(inja::Arguments &args);
  nlohmann::json to_cobol(inja::Arguments &args);
  nlohmann::json to_const(inja::Arguments &args);
  nlohmann::json to_cpp(inja::Arguments &args);
  nlohmann::json to_dot(inja::Arguments &args);
  nlohmann::json to_kebab(inja::Arguments &args);
  nlohmann::json to_lower(inja::Arguments &args);
  nlohmann::json to_lower_first(inja::Arguments &args);
  nlohmann::json to_pascal(inja::Arguments &args);
  nlohmann::json to_path(inja::Arguments &args);
  nlohmann::json to_snake(inja::Arguments &args);
  nlohmann::json to_space(inja::Arguments &args);
  nlohmann::json to_train(inja::Arguments &args);
  nlohmann::json to_upper(inja::Arguments &args);
  nlohmann::json to_upper_first(inja::Arguments &args);
  nlohmann::json replace(inja::Arguments &args);
  nlohmann::json padleft(inja::Arguments &args);
  nlohmann::json padright(inja::Arguments &args);
  nlohmann::json sort_by(inja::Arguments &args);
  nlohmann::json to_hex(inja::Arguments &args);

  inja::Environment engine(inja::Environment &env);
  inja::Environment engine(std::filesystem::path template_dir);
  inja::Environment engine();

} // namespace templates
