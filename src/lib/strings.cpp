#include <regex>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <filesystem>

#include <entt/core/hashed_string.hpp>

#include "./strings.hpp"

using namespace str;
using namespace std;

string str::skip(string str, int length) {
  if (length < 0) {
    return str;
  }
  if (str.length() <= length) {
    return "";
  }
  return str.substr(str.length() - length);
}

string str::take(string str, int length) {
  if (length < 0) {
    return "";
  }
  if (str.length() <= length) {
    return str;
  }
  return str.substr(0, length);
}

string str::first(string str, int length) {
  return take(str, length);
}

string str::last(string str, int length) {
  return skip(str, str.length() - length);
}

bool str::ends_with(const string str, const string subject) {
  if (subject.length() > str.length())
    return false;
  return equal(subject.rbegin(), subject.rend(), str.rbegin());
}

bool str::starts_with(const string value, const string subject) {
  if (subject.length() > value.length())
    return false;
  return equal(subject.begin(), subject.end(), value.begin());
}

string str::replace_all(string data, string find, string replacement) {
  auto dot = regex("(.*)?" + find + "(.*)?");

  auto current = string(data);
  while (true) {
    auto replaced = regex_replace(current, dot, "$1" + replacement + "$2");
    if (replaced != current) {
      current = replaced;
      continue;
    }
    break;
  }

  return current;
}

vector<string> str::split(string value, char delimiter) {
  auto result = vector<string>();
  stringstream stream(value);
  for (string token; std::getline(stream, token, delimiter);) {
    result.push_back(move(token));
  }
  return result;
}

vector<string> str::split(string value, vector<char> delimiter) {
  auto result = vector<string>();
  auto current = ""s;
  for (size_t i = 0; i < value.size(); i++) {
    auto split = false;
    auto character = value[i];
    for (auto test : delimiter) {
      if (character == test) {
        result.push_back(current);
        current = "";
        split = true;
        break;
      }
    }
    if (!split) {
      current += character;
    }
  }
  if (current != "") {
    result.push_back(current);
  }
  return result;
}

string str::padleft(string value, int length, string pad) {
  for (auto i = 0; i < value.length() - length; i++) {
    value = pad + value;
  }
  return value;
}

string str::padright(string value, int length, string pad) {
  for (auto i = 0; i < value.length() - length; i++) {
    value = value + pad;
  }
  return value;
}

vector<string> str::split(string value, string delimiter, int limit) {
  size_t last = 0;
  size_t next = 0;
  auto result = vector<string>{};

  while ((next = value.find(delimiter, last)) != string::npos) {
    result.push_back(value.substr(last, next - last));
    last = next + delimiter.length();
    auto count = result.size() + (value.length() > 0 ? 1 : 0);
    if (limit > 0 && count >= limit) {
      break;
    }
  }

  result.push_back(value.substr(last));
  return result;
}

vector<string> str::tokenize(string value) {
  auto words = split(value, vector<char>{ ' ', '-', '_', '|', '/', '\\', '.' });
  auto parts = vector<string>{};

  for (auto word : words) {
    auto upper = false;
    auto prev_upper = false;
    auto next_upper = false;
    auto first = true;
    auto last = false;
    auto current = ""s;

    for (size_t i = 0; i < word.size(); i++) {
      auto character = word[i];
      prev_upper = upper;
      upper = isupper(character);
      next_upper = i + 1 < word.size() ? isupper(word[i + 1]) : false;
      last = i == word.size() - 1;
      if (first) {
        first = false;
        current += tolower(character);
        continue;
      }
      if (!upper) {
        current += character;
        continue;
      }
      if ((!prev_upper && !first) || (!next_upper && !last)) {
        parts.push_back(current);
        current = "";
      }
      current += tolower(character);
    }

    if (current != "") {
      parts.push_back(current);
    }
  }

  return parts;
}

string str::join(const vector<string> parts, const string &delim) {
  ostringstream joined;

  auto b = begin(parts), e = end(parts);
  if (b != e) {
    copy(b, prev(e), ostream_iterator<string>(joined, delim.c_str()));
    b = prev(e);
  }

  if (b != e) {
    joined << *b;
  }

  return joined.str();
}

string str::to_lower(const string &value) {
  auto data = string(value);
  transform(data.begin(), data.end(), data.begin(), [](auto c) {
    return tolower(c);
  });
  return data;
}

string str::to_upper(const string &value) {
  auto data = string(value);
  transform(data.begin(), data.end(), data.begin(), [](auto c) {
    return toupper(c);
  });
  return data;
}

string str::to_upper_first(const string &value) {
  auto data = string(value);
  if (data.size() >= 1) {
    return (char)toupper(data[0]) + data.substr(1);
  }
  return data;
}

string str::to_lower_first(const string &value) {
  auto data = string(value);
  if (data.size() >= 1) {
    return (char)tolower(data[0]) + data.substr(1);
  }
  return data;
}

string str::to_snake(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return to_lower(word);
  });
  return join(parts, "_");
}

string str::to_kebab(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return to_lower(word);
  });
  return join(parts, "-");
}

string str::to_pascal(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return to_upper_first(to_lower(word));
  });
  return join(parts, "");
}

string str::to_camel(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return to_upper_first(to_lower(word));
  });
  return to_lower_first(str::join(parts, ""));
}

string str::to_const(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return str::to_upper(word);
  });
  return str::join(parts, "_");
}

string str::to_train(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return to_upper_first(to_lower(word));
  });
  return join(parts, "-");
}

string str::to_ada(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return to_upper_first(to_lower(word));
  });
  return join(parts, "_");
}

string str::to_cobol(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return to_upper(word);
  });
  return join(parts, "-");
}

string str::to_dot(const string &value) {
  return join(tokenize(value), ".");
}

string str::to_path(const string &value) {
  return join(tokenize(value), "/");
}

string str::to_space(const string &value) {
  return join(tokenize(value), " ");
}

string str::to_capital(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return to_upper_first(to_lower(word));
  });
  return join(parts, " ");
}

string str::to_cpp(const string &value) {
  auto parts = tokenize(value);
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return to_lower(word);
  });
  return join(parts, "::");
}

void str::trim_left(string &s) {
  s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !isspace(ch);
          }));
}

void str::trim_left(string &s, string chars) {
  s.erase(s.begin(), find_if(s.begin(), s.end(), [&](unsigned char ch) {
            return chars.find(ch) != string::npos;
          }));
}

void str::trim_right(string &s) {
  s.erase(
    find_if(
      s.rbegin(), s.rend(),
      [](unsigned char ch) {
        return !isspace(ch);
      })
      .base(),
    s.end());
}

void str::trim_right(string &s, string chars) {
  s.erase(
    find_if(
      s.rbegin(), s.rend(),
      [&](unsigned char ch) {
        return chars.find(ch) != string::npos;
      })
      .base(),
    s.end());
}

void str::trim(string &s) {
  trim_left(s);
  trim_right(s);
}

void str::trim(string &s, string &chars) {
  trim_left(s, chars);
  trim_right(s, chars);
}

string str::trim_left_copy(string s) {
  trim_left(s);
  return s;
}

string str::trim_left_copy(string s, string chars) {
  trim_left(s, chars);
  return s;
}

string str::trim_right_copy(string s) {
  trim_right(s);
  return s;
}

string str::trim_right_copy(string s, string chars) {
  trim_right(s, chars);
  return s;
}

string str::trim_copy(string s) {
  trim(s);
  return s;
}

string str::trim_copy(string s, string chars) {
  trim(s, chars);
  return s;
}

vector<string> str::parse_tags(string value, char delimiter) {
  auto parts = split(value, ',');
  transform(parts.begin(), parts.end(), parts.begin(), [](auto &word) {
    return trim_copy(word);
  });
  return parts;
}
