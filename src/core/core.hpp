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


template<typename T> concept FloatingPoint = requires(T t) {
    std::is_floating_point_v<T>;
};


template <FloatingPoint FP>
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

template <RangeType Range>
inline bool is_within(int value, Range range) {
    return (value >= range.begin) && (value <= range.end);
}

template <RangeType Range>
inline bool is_within(Range inner, Range outer) {
    return (inner.begin >= outer.begin) && (inner.end <= outer.end);

}

inline bool is_within(int value, int range_begin, int range_end) {
    return (value >= range_begin) && (value <= range_end);
}

template <typename View>
inline bool is_within(int cursor_line, View* view) {
    return (cursor_line >= view->cursor->views_top_line) && (cursor_line <= (view->cursor->views_top_line + view->lines_displayable));
}