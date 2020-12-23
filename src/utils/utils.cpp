//
// Created by 46769 on 2020-12-14.
//

#include "utils.hpp"
#include <fstream>

namespace util {
    void printmsg(const char *msg) {
        fmt::print("{}\n", msg);
    }
}
#ifdef DEBUG

Timer::Timer(const std::string_view &title) : benchmark_title(title) {
    begin = hiResClk::now();
}
Timer::~Timer() {
    end = hiResClk::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>((end - begin)).count();
    fmt::print("{}: {}us\n", benchmark_title, duration);
}
#endif

namespace util::file {
    std::vector<std::string> read_file_to_lines(const fs::path &filePath) {
        if (!fs::exists(filePath)) {
            throw std::runtime_error{fmt::format("Error reading file {} to memory, path doesn't exist", filePath.string())};
        }
        std::vector<std::string> result;
        result.reserve(16);
        std::string place_holder;
        place_holder.reserve(128);
        result.push_back(place_holder);
        std::ifstream file{filePath};
        for (auto i = 0; std::getline(file, result[i++]); result.push_back(place_holder));
#ifdef INSTRUMENTATION
        fmt::print("Read {} lines. Contents:\n", result.size());
        for(const auto& l : result) fmt::print("{}\n", l);
#endif
        return result;
    }
}
