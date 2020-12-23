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