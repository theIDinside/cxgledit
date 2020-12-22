
#pragma once
#include <fmt/core.h>
#include <filesystem>

#ifdef DEBUG
#include <chrono>
#include <memory>
#endif

#ifdef WIN32
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#ifdef INSTRUMENTATION
using namespace std::chrono_literals;
using hiResClk = std::chrono::high_resolution_clock;
using chronotp = std::chrono::high_resolution_clock::time_point;
struct Timer {
    explicit Timer(const std::string_view& timer_title);
    ~Timer();
    std::string_view benchmark_title;
    chronotp begin, end;
};
#endif

#ifdef INSTRUMENTATION
#define MICRO_BENCH(title) Timer timer##__LINE__{title}
#define FN_MICRO_BENCH() MICRO_BENCH(__PRETTY_FUNCTION__)
#else
#define MICRO_BENCH(title)
#define FN_MICRO_BENCH() MICRO_BENCH(__PRETTY_FUNCTION__)
#endif

using uint = unsigned int;
using usize = std::size_t;
using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;


namespace fs = std::filesystem;
namespace util::file {
    std::vector<std::string> read_file_to_lines(const fs::path& filePath);
}

namespace util {
    void printmsg(const char* msg);

    template<typename ...Args>
    void println(const char* fmt_string, Args... args) {
        fmt::print(fmt_string, args...);
        fmt::print("\n");
        fflush(stdout);
    }
}
