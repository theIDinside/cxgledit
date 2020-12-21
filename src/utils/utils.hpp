//
// Created by 46769 on 2020-12-20.
//

#pragma once
#include <fmt/core.h>


#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION __FUNCSIG__
#endif

#ifdef WIN32
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#define PANIC(...)                                                                                                          \
    fmt::print("panicked in {}\n\tMessage: ", __PRETTY_FUNCTION__);                                                         \
    util::panic(__VA_ARGS__)

namespace util {

/// panics with an output to stdout, and calls std::abort() to terminate
    template<typename... Args>
    void panic(const char *message, Args... args) {
        fmt::print(message, args...);
        fmt::print("\nExiting.");
        std::abort();
    }
}