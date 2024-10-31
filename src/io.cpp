#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
  #define NOMINMAX
#endif

#ifdef _WIN32
  #include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <cstdlib>
#include <cstdio>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <numeric>
#include <filesystem>

#include <fmt/ranges.h>

#include <spdlog/spdlog.h>
#include <boost/process.hpp>

#include "./io.hpp"
#include "./hash.hpp"
#include "./strings.hpp"

using namespace std;
using namespace std::filesystem;

static std::string executable_path = "";

path io::get_file_directory(string p) {
  return path(p.c_str()).parent_path();
}

std::filesystem::path io::set_current_executable(const std::string& path) {
  executable_path = path;
  return executable_path;
}

path io::get_current_executable() {
#ifdef _WIN32
  char buffer[4098];
  memset(buffer, 0, sizeof(buffer));
  if (executable_path.empty()) {
    GetModuleFileNameA(nullptr, buffer, sizeof(buffer));
    executable_path = std::string(buffer);
  }
#endif
  return filesystem::path(std::string(executable_path));
}

path io::get_current_executable_directory() {
  return get_file_directory(get_current_executable().string());
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

io::shell_output io::shell(string command, vector<string> args, path p) {
  namespace bp = boost::process;

#ifdef WIN32
  if (!str::ends_with(str::to_lower(command), ".exe")) {
    command += ".exe";
  }
#endif

  if (filesystem::absolute(command) != command) {
    auto local = get_current_executable_directory() / command;
    if (std::filesystem::exists(local)) {
      command = local.string();
    } else {
      command = bp::search_path(command).string();
    }
  }

  if (p.empty()) {
    p = current_path();
  }

  int code = 0;
  bp::ipstream pstdout;
  bp::ipstream pstderr;

  spdlog::trace("executing: {} {}", command, fmt::join(args, " "));
  spdlog::trace("  > cwd: {}", p.string());

  code = bp::system(command, bp::args = args, bp::std_out > pstdout, bp::std_err > pstderr, bp::start_dir = p.string());

  std::ostringstream sstdout, sstderr;

  pstdout >> sstdout.rdbuf();
  pstderr >> sstderr.rdbuf();

  spdlog::trace("  > exit: {}", code);

  return { code, sstdout.str(), sstderr.str() };
}
