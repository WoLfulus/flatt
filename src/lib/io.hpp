#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace io {

  std::filesystem::path get_file_directory(std::string p);
  std::filesystem::path get_current_executable_directory();

  std::pair<bool, std::string> read_file(std::filesystem::path p);
  bool write_file(std::filesystem::path p, std::string data);

  std::optional<std::string> hash_file(std::filesystem::path p);
  std::optional<std::string> hash_dir(std::filesystem::path p);

  std::vector<std::filesystem::path> list_dirs(const std::filesystem::path& dir);
  std::vector<std::filesystem::path> list_files(const std::filesystem::path& dir);

  int shell(
    std::string command, std::vector<std::string> args, std::filesystem::path p = "",
    std::filesystem::path output = "");

  std::string shell_output(std::string command, std::vector<std::string> args, std::filesystem::path p = "");

} // namespace io
