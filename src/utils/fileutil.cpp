#include "fileutil.hpp"
#include <filesystem>
#include <fmt/core.h>

namespace fs = std::filesystem;

std::optional<std::string> get_path(const char *path) {
    if (fs::exists(path)) {
        fmt::print("found path/file: {}\n", path);
        return path;
    } else {
        return {};
    }
}