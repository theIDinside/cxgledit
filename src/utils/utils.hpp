
#pragma once
#include <filesystem>
#include <fmt/core.h>
#include <optional>
#include <variant>

#include <bindingslib/keybindings.hpp>


#ifdef DEBUG
#include <array>
#include <chrono>
#include <map>
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
    explicit Timer(const std::string_view &timer_title);
    ~Timer();

    void stop();
    void reset();

    std::string_view benchmark_title;
    chronotp begin, end;
};
#endif

template <typename T, typename U>
constexpr auto to_t(T t) -> U {
    return t;
}

template <typename T, typename ...Ts>
constexpr auto to_option_vec(T t, Ts... ss) -> std::optional<std::vector<T>> {
    if constexpr (sizeof...(ss) == 0) {
        return std::vector<T> {t};
    } else if constexpr (std::is_same_v<T, Ts...>) {
        return std::vector<T> {t, { ss... }};
    } else {
        return std::vector<std::variant<T, Ts...>> {t, { ss... }};
    }
}

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B) A##B

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define PPCAT(A, B) PPCAT_NX(A, B)

#ifdef INSTRUMENTATION
#define MICRO_BENCH(title)                                                                                             \
    Timer timer##__LINE__ { title }
#define SECTION_MICRO_BENCH(title, section)                                                                            \
    Timer timer##__LINE__ { ##title##section }
#define FN_MICRO_BENCH() MICRO_BENCH(__PRETTY_FUNCTION__)
#define PUSH_FN_SUBSECTION(sectionKey)                                                                                 \
    std::string n = __PRETTY_FUNCTION__;                                                                               \
    n.append(sectionKey);                                                                                              \
    MICRO_BENCH(n)

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
    std::vector<std::string> read_file_to_lines(const fs::path &filePath);
}

/// This is not really a "safe" delete in that sense, but for purposes in this project it is.
/// Why? Because, we actually use pointers in the "owning / non-owning" sense that the C++ standard
/// talks about, thus, if it's a non-owning pointer, deleting it must set it to nullptr, as we have requested the memory to die
/// but we also don't want to keep pointing to that address. So when our CommandInterpreter for instance wants to delete
/// the "current command" it is owning & pointing to, we want to be able to check in successive calls after that if ptr == nullptr

namespace util {

    auto to_string(CXMode mode) -> const char*;

    void printmsg(const char *msg);

    template<typename... Args>
    void println(const char *fmt_string, Args... args) {
        fmt::print(fmt_string, args...);
        fmt::print("\n");
        fflush(stdout);
    }
}// namespace util

template<typename T>
void safe_delete(T *&t) {
    if (t) { delete t; }
    t = nullptr;
}


#ifdef WIN32
auto file_size(const char* file_path) -> std::optional<int>;
#endif
