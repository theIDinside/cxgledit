#pragma once

#include <optional>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;


std::optional<std::string> get_path(const char *path);
/// Returns: bytes written
[[maybe_unused]] std::optional<int> sv_write_file(fs::path file_path, std::string_view view);