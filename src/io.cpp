#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
  #define NOMINMAX
#endif

#ifdef _WIN32
  #include <windows.h>
#endif

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <numeric>
#include <filesystem>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "./io.hpp"
#include "./hash.hpp"
#include "./strings.hpp"

using namespace std;
using namespace std::filesystem;

path io::get_file_directory(string p) {
  return path(p.c_str()).parent_path();
}

path io::get_current_executable_directory() {
  char buffer[2048];
#ifdef _WIN32
  GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
#endif

  return get_file_directory(buffer);
}

pair<bool, string> io::read_file(path p) {
  if (!exists(p)) {
    return make_pair(false, string(""));
  }

  ifstream ifs(p.string().c_str(), ios::binary);
  stringstream dss;
  dss << ifs.rdbuf();
  return make_pair(true, dss.str());
}

bool io::write_file(path p, string data) {
  if (!exists(p)) {
    try {
      create_directories(p.parent_path());
    } catch (...) {
      // pass
    }
  }

  ofstream ofs(p.string().c_str(), ios::binary);
  ofs.write(data.c_str(), data.size());
  ofs.close();

  return true;
}

optional<string> io::hash_file(path p) {
  auto [exists, data] = read_file(p);
  return exists ? hashes::sha1(data) : optional<string>{};
}

optional<string> io::hash_dir(path p) {
  auto hashes = vector<string>{};

  if (!filesystem::exists(p)) {
    return {};
  }

  auto it = recursive_directory_iterator(p);
  for (auto dir : it) {
    if (dir.is_regular_file()) {
      auto hash = hash_file(dir);
      if (hash.has_value()) {
        hashes.push_back(hash.value());
      }
    }
  }

  sort(hashes.begin(), hashes.end());

  auto str = string("");
  for (auto hash : hashes) {
    str += hash;
  }

  return hashes::sha1(str);
}

vector<path> io::list_dirs(const path& dir) {
  if (!filesystem::exists(dir) || !filesystem::is_directory(dir)) {
    return {};
  }

  auto root = absolute(dir);
  auto result = vector<path>{};
  for (auto entry : recursive_directory_iterator(root)) {
    if (!entry.is_directory()) {
      continue;
    }
    result.push_back(filesystem::relative(entry, root));
  }

  return result;
}

vector<path> io::list_files(const path& dir) {
  if (!filesystem::exists(dir) || !filesystem::is_directory(dir)) {
    return {};
  }

  auto root = absolute(dir);
  auto result = vector<path>{};
  for (auto entry : recursive_directory_iterator(root)) {
    if (!entry.is_regular_file()) {
      continue;
    }
    result.push_back(filesystem::relative(entry, root));
  }

  return result;
}

int io::shell(string command, vector<string> args, path p, path output) {
  if (p.empty()) {
    p = get_file_directory(command);
  }

  auto finalCommand = "\"" + command + "\" ";
  if (!p.empty()) {
    finalCommand = "cd \"" + p.string() + "\" && " + finalCommand;
  }

  auto finalArgs = accumulate(next(args.begin()), args.end(), args[0], [](auto a, auto b) {
    if (b.find(' ') != string::npos || b == "") {
      b = "\"" + b + "\"";
    }
    return a + " " + b;
  });

  auto final = finalCommand + finalArgs;
  spdlog::trace("");
  spdlog::trace(" shell: {}", final);
  spdlog::trace("");

  if (!output.empty()) {
    return system((final + " > \"" + output.string() + "\"").c_str());
  }

  return system(final.c_str());
}

string io::shell_output(string command, vector<string> args, path p) {
  std::string file = std::tmpnam(nullptr);
  io::shell(command, args, p, file);
  auto [_, output] = io::read_file(file);
  return output;
}
