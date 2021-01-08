//
// Created by 46769 on 2020-12-14.
//

#include "utils.hpp"
#include <fstream>

namespace util {
    void printmsg(const char *msg) { fmt::print("{}\n", msg); }
}// namespace util
#ifdef DEBUG

Timer::Timer(const std::string_view &title) : benchmark_title(title) { begin = hiResClk::now(); }
Timer::~Timer() {
    end = hiResClk::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>((end - begin)).count();
    fmt::print("{}: {}us\n", benchmark_title, duration);
}
#endif

namespace util::file {
    std::vector<std::string> read_file_to_lines(const fs::path &filePath) {
        if (not fs::exists(filePath)) {
            throw std::runtime_error{
                    fmt::format("Error reading file {} to memory, path doesn't exist", filePath.string())};
        }
        std::vector<std::string> result;
        result.reserve(16);
        std::string place_holder;
        place_holder.reserve(128);
        result.push_back(place_holder);
        std::ifstream file{filePath};
        for (auto i = 0; std::getline(file, result[i++]); result.push_back(place_holder))
            ;
#ifdef INSTRUMENTATION
        fmt::print("Read {} lines. Contents:\n", result.size());
        for (const auto &l : result) fmt::print("{}\n", l);
#endif
        return result;
    }
}// namespace util::file

#ifdef WIN32
#include <windows.h>

auto file_size(const char *file_path) -> std::optional<int> {
    if (fs::exists(file_path)) {
        WIN32_FILE_ATTRIBUTE_DATA fInfo;
        GetFileAttributesEx(file_path, GetFileExInfoStandard, (void *) &fInfo);
        return (fInfo.nFileSizeHigh << 16) | (fInfo.nFileSizeLow);
    }
    return {};
}
#endif

#ifdef LINUX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

    auto file_size(const char* file) -> std::optional<int> {
            struct stat info;
            if(stat(file, &info) == -1) return {};
            else {
                return info.st_size;
            }
    }
#endif
