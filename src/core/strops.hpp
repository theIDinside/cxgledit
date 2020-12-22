//
// Created by cx on 2020-11-27.
//
#pragma once
#include <cstddef>
#include <cstdint>

using u64 = uint64_t;

#ifdef INTRINSICS_ENABLED
#include <immintrin.h>

u64 newline_count_64bit(const char* data, std::size_t len);

#else

u64 naive_newline_count(const char* data, std::size_t len);

#endif
namespace str {
    u64 count_newlines(const char* data, std::size_t length);
}