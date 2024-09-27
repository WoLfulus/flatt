#pragma once

#include <string>
#include <vector>
#include <iomanip>

namespace str {

  std::string skip(std::string str, int length);

  std::string take(std::string str, int length);

  std::string first(std::string str, int length);
  std::string last(std::string str, int length);

  bool ends_with(const std::string str, const std::string subject);
  bool starts_with(const std::string value, const std::string subject);

  std::string replace_all(std::string data, std::string find, std::string replacement);

  std::vector<std::string> split(std::string value, char delimiter = ' ');
  std::vector<std::string> split(std::string value, std::vector<char> delimiter = { ' ' });

  std::string padleft(std::string value, int length, std::string pad = " ");
  std::string padright(std::string value, int length, std::string pad = " ");

  std::vector<std::string> split(std::string value, std::string delimiter, int limit = 0);
  std::vector<std::string> tokenize(std::string value);

  std::string join(const std::vector<std::string> parts, const std::string &delim);

  std::string to_lower(const std::string &value);
  std::string to_lower_first(const std::string &value);
  std::string to_upper(const std::string &value);
  std::string to_upper_first(const std::string &value);
  std::string to_snake(const std::string &value);
  std::string to_kebab(const std::string &value);
  std::string to_pascal(const std::string &value);
  std::string to_camel(const std::string &value);
  std::string to_const(const std::string &value);
  std::string to_train(const std::string &value);
  std::string to_ada(const std::string &value);
  std::string to_cobol(const std::string &value);
  std::string to_dot(const std::string &value);
  std::string to_path(const std::string &value);
  std::string to_space(const std::string &value);
  std::string to_capital(const std::string &value);
  std::string to_cpp(const std::string &value);

  void trim_left(std::string &s);
  void trim_left(std::string &s, std::string chars);
  void trim_right(std::string &s);
  void trim_right(std::string &s, std::string chars);
  void trim(std::string &s);
  void trim(std::string &s, std::string &chars);

  std::string trim_left_copy(std::string s);
  std::string trim_left_copy(std::string s, std::string chars);
  std::string trim_right_copy(std::string s);
  std::string trim_right_copy(std::string s, std::string chars);
  std::string trim_copy(std::string s);
  std::string trim_copy(std::string s, std::string chars);

  template <typename T>
  std::string to_hex(T v, T size = sizeof(T) * 2, std::string fill = "0", std::string prefix = "0x") {
    std::stringstream stream;
    stream << std::setfill(fill.length() > 0 ? fill.at(0) : '0') << std::setw(size) << std::hex << v;
    return prefix + stream.str();
  }

  std::vector<std::string> parse_tags(std::string value, char delimiter = ',');

} // namespace str
