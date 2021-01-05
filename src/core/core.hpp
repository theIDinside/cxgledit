//
// Created by 46769 on 2020-12-22.
//

#pragma once

#include <fmt/core.h>
#include <utils/utils.hpp>

constexpr auto SWEDISH_LAST_ALPHA_CHAR_UNICODE = 0x00f6u;

template<typename... Args>
void panic(const char *message, Args... args) {
    fmt::print(message, args...);
    fmt::print("\nExiting.\n");
    fflush(stdout);
    std::abort();
}

#define PANIC(...)                                                                                                     \
    fmt::print("panicked in {}\n\tMessage: ", __PRETTY_FUNCTION__);                                                    \
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