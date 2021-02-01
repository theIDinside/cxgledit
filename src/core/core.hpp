//
// Created by 46769 on 2020-12-22.
//

#pragma once

#include <fmt/core.h>
#include <fmt/format.h>
#include <utils/utils.hpp>

constexpr auto SWEDISH_LAST_ALPHA_CHAR_UNICODE = 0x00f6u;

#define AS(value, type) static_cast<type>(value)
#define int_ceil(value) static_cast<int>(std::ceil(value))

#define LET(name) const auto name

#ifdef DEBUG
constexpr auto DEBUG_IS_ON = true;
#else
constexpr auto DEBUG_IS_ON = false;
#endif

/// Assertion that checks if index is not outside of containers size, or that it's 0 when container is empty
#define INDEX_ASSERTION(index, container)                                                                              \
    if constexpr (DEBUG_IS_ON) {                                                                                       \
        asserts::index_assertion(__FUNCSIG__, __FILE__, __LINE__, index, container);                                   \
    }

namespace asserts {
template<typename Container>
concept ContainerAssertable = requires(Container c) {
    c.empty();
    c.size();
};

template<ContainerAssertable C>
constexpr bool index_assertion(const char *fn_name, const char *file, int line_number, int index, C c) {
    if (not(index < AS(c.size(), int) || (index == 0 && c.empty()))) {
        util::println("Assertion failed ({}:{}): Type: 'Index assertion': {} not < {}", index, c.size());
        std::abort();
    }
    return true;
}
}// namespace asserts

template<typename T>
concept FloatingPoint = requires(T t) {
    std::is_floating_point_v<T>;
};

template<FloatingPoint FP>
constexpr int as_int(FP value) {
    return AS(std::round(value), int);
}

/// Boxed = owned & RAII'd
template<typename T>
using Boxed = std::unique_ptr<T>;

template<typename... Args>
void panic(const char *message, Args... args) {
    fmt::print(message, args...);
    fmt::print("\nExiting.\n");
    fflush(stdout);
    std::abort();
}

#define PANIC(...)                                                                                                     \
    fmt::print("panicked on line {}\n\t  {}\n\tMessage: ", __LINE__, __PRETTY_FUNCTION__);                             \
    panic(__VA_ARGS__)

/// Use to implement comparisons between objects. Implements only based on a single value in each object
#define SPACE_SHIP(Type, discriminate_on)                                                                              \
    friend bool operator==(const Type &lhs, const Type &rhs) { return lhs.discriminate_on == rhs.discriminate_on; }    \
    friend bool operator!=(const Type &lhs, const Type &rhs) { return !(lhs == rhs); }                                 \
    friend bool operator<(const Type &lhs, const Type &rhs) { return lhs.discriminate_on < rhs.discriminate_on; }      \
    friend bool operator>(const Type &lhs, const Type &rhs) { return lhs.discriminate_on > rhs.discriminate_on; }

template<typename Container>
concept RevIterable = requires(Container a) {
    a.begin();
    a.end();
    a.rbegin();
    a.rend();
    a.size();
};

/**
 * Helper function to rotate container.
 * @param container - Container to be rotated in place, must adhere to constraint RevIterable
 * @param steps - The amount of steps to rotate. If steps is negative, container will be rotated (in place) left
 * if steps is positive, it will rotate in place right.
 */
template<RevIterable C>
void rotate_container(C &container, int steps) {
    if (auto s = std::abs(steps); s > container.size()) {
        PANIC("Container C size < steps: {} < {}", container.size(), s);
    }

    if (steps < 0) {
        auto _steps = std::abs(steps);
        std::rotate(container.begin(), container.begin() + _steps, container.end());
    } else {
        auto _steps = std::abs(steps);
        std::rotate(container.rbegin(), container.rbegin() + _steps, container.rend());
    }
}

template<typename Range>
concept RangeType = requires(Range r) {
    r.begin;
    r.end;
};

template<RangeType Range>
inline bool is_within(int value, Range range) {
    return (value >= range.begin) && (value <= range.end);
}

template<RangeType Range>
inline bool is_within(Range inner, Range outer) {
    return (inner.begin >= outer.begin) && (inner.end <= outer.end);
}

inline bool is_within(int value, int range_begin, int range_end) {
    return (value >= range_begin) && (value <= range_end);
}

template<typename View>
inline bool is_within(int cursor_line, View *view) {
    return (cursor_line >= view->cursor->views_top_line) &&
           (cursor_line <= (view->cursor->views_top_line + view->lines_displayable));
}

template<class>
inline constexpr bool always_false_v = false;
