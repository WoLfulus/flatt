#pragma once

#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

namespace str {
bool ends_with(const std::string value, const std::string ending)
{
  if (ending.length() > value.length())
    return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

bool starts_with(const std::string value, const std::string start)
{
  if (start.length() > value.length())
    return false;
  return std::equal(start.begin(), start.end(), value.begin());
}

string replace_all(string data, string find, string replacement)
{
  auto dot = std::regex("(.*)?" + find + "(.*)?");

  auto current = string(data);
  while (true)
  {
    auto replaced = regex_replace(current, dot, "$1" + replacement + "$2");
    if (replaced != current)
    {
      current = replaced;
      continue;
    }
    break;
  }

  return current;
}

vector<string> split(string value, char delimiter = ' ')
{
  auto result = vector<string>();
  istringstream stream(value);
  for (string token; getline(stream, token, delimiter);)
  {
    result.push_back(std::move(token));
  }
  return result;
}

vector<string> split(string value, vector<char> delimiter = { ' ' })
{
  auto result = vector<string>();
  auto current = ""s;
  for (size_t i = 0; i < value.size(); i++)
  {
    auto split = false;
    auto character = value[i];
    for (auto test : delimiter)
    {
      if (character == test)
      {
        result.push_back(current);
        current = "";
        split = true;
        break;
      }
    }
    if (!split)
    {
      current += character;
    }
  }
  if (current != "")
  {
    result.push_back(current);
  }
  return result;
}

string padleft(string value, int length, string pad = " ")
{
  for (auto i = 0; i < value.length() - length; i++)
  {
    value = pad + value;
  }
  return value;
}

string padright(string value, int length, string pad = " ")
{
  for (auto i = 0; i < value.length() - length; i++)
  {
    value = value + pad;
  }
  return value;
}

vector<string> split(string value, string delimiter, int limit = 0)
{
  size_t last = 0;
  size_t next = 0;
  auto result = vector<string>{};

  while ((next = value.find(delimiter, last)) != string::npos)
  {
    result.push_back(value.substr(last, next - last));
    last = next + delimiter.length();
    auto count = result.size() + (value.length() > 0 ? 1 : 0);
    if (limit > 0 && count >= limit)
    {
      break;
    }
  }

  result.push_back(value.substr(last));
  return result;
}

vector<string> explode(string value)
{
  auto words = split(value, vector<char>{ ' ', '-', '_', '|', '/', '\\', '.' });
  auto parts = vector<string>{};

  for (auto word : words)
  {
    auto upper = false;
    auto prev_upper = false;
    auto next_upper = false;
    auto first = true;
    auto last = false;
    auto current = ""s;

    for (size_t i = 0; i < word.size(); i++)
    {
      auto character = word[i];
      prev_upper = upper;
      upper = isupper(character);
      next_upper = i + 1 < word.size() ? isupper(word[i + 1]) : false;
      last = i == word.size() - 1;
      if (first)
      {
        first = false;
        current += tolower(character);
        continue;
      }
      if (!upper)
      {
        current += character;
        continue;
      }
      if ((!prev_upper && !first) || (!next_upper && !last))
      {
        parts.push_back(current);
        current = "";
      }
      current += tolower(character);
    }

    if (current != "")
    {
      parts.push_back(current);
    }
  }

  return parts;
}

string join(const vector<string> parts, const string &delim)
{
  ostringstream joined;

  auto b = begin(parts), e = end(parts);
  if (b != e)
  {
    copy(b, prev(e), ostream_iterator<string>(joined, delim.c_str()));
    b = prev(e);
  }

  if (b != e)
  {
    joined << *b;
  }

  return joined.str();
}

string lower_case(const string &value)
{
  auto data = string(value);
  std::transform(data.begin(), data.end(), data.begin(), [](auto c) { return tolower(c); });
  return data;
}

string upper_case(const string &value)
{
  auto data = string(value);
  std::transform(data.begin(), data.end(), data.begin(), [](auto c) { return toupper(c); });
  return data;
}

string ucfirst(const string &value)
{
  auto data = string(value);
  if (data.size() >= 1)
  {
    return (char)toupper(data[0]) + data.substr(1);
  }
  return data;
}

string lcfirst(const string &value)
{
  auto data = string(value);
  if (data.size() >= 1)
  {
    return (char)tolower(data[0]) + data.substr(1);
  }
  return data;
}

string snake_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return lower_case(word); });
  return join(parts, "_");
}

string kebab_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return lower_case(word); });
  return join(parts, "-");
}

string pascal_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return ucfirst(lower_case(word)); });
  return join(parts, "");
}

string camel_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return ucfirst(lower_case(word)); });
  return lcfirst(join(parts, ""));
}

string const_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return upper_case(word); });
  return join(parts, "_");
}

string train_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return ucfirst(lower_case(word)); });
  return join(parts, "-");
}

string ada_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return ucfirst(lower_case(word)); });
  return join(parts, "_");
}

string cobol_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return upper_case(word); });
  return join(parts, "-");
}

string dot_case(const string &value)
{
  return join(explode(value), ".");
}

string path_case(const string &value)
{
  return join(explode(value), "/");
}

string space_case(const string &value)
{
  return join(explode(value), " ");
}

string capital_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return ucfirst(lower_case(word)); });
  return join(parts, " ");
}

string cpp_case(const string &value)
{
  auto parts = explode(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) { return lower_case(word); });
  return join(parts, "::");
}

void trim_left(string &s)
{
  s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) { return !isspace(ch); }));
}

void trim_right(string &s)
{
  s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !isspace(ch); }).base(), s.end());
}

void trim(string &s)
{
  trim_left(s);
  trim_right(s);
}

string trim_left_copy(string s)
{
  trim_left(s);
  return s;
}

string trim_right_copy(string s)
{
  trim_right(s);
  return s;
}

string trim_copy(string s)
{
  trim(s);
  return s;
}

template <typename T>
string to_hex(T v, T size = sizeof(T) * 2, string fill = "0", string prefix = "0x")
{
  stringstream stream;
  stream << setfill(fill.length() > 0 ? fill.at(0) : '0') << setw(size) << hex << v;
  return prefix + stream.str();
}

vector<string> parse_tags(string value, char delimiter = ',')
{
  auto parts = split(value, ',');
  transform(parts.begin(), parts.end(), parts.begin(), trim_copy);
  return parts;
}
} // namespace str
