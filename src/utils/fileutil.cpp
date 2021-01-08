#include "fileutil.hpp"
#include "utils.hpp"
#include <fmt/core.h>
#include <fstream>


std::optional<std::string> get_path(const char *path) {
    if (fs::exists(path)) {
        fmt::print("found path/file: {}\n", path);
        return path;
    } else {
        return {};
    }
}

std::optional<int> sv_write_file(fs::path file_path, std::string_view write_data) {
    try {
        std::ofstream outf{file_path};
        outf << write_data;
        outf.close();
    } catch(...) {
        util::println("Failed to write file");
    }
    return file_size(file_path.string().c_str());
}
