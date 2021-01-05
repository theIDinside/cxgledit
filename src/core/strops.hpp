//
// Created by cx on 2020-11-27.
//
#pragma once
#include <cstddef>
#include <cstdint>
#include <list>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

using u64 = uint64_t;

#ifdef INTRINSICS_ENABLED
#include <immintrin.h>

u64 newline_count_64bit(const char *data, std::size_t len);

#else

std::vector<int> make_lines_indices(const char *data, std::size_t len);

#endif
namespace str {
    std::vector<int> count_newlines(const char *data, std::size_t length);
}

namespace util::str {
    std::vector<std::string_view> vec_split_string(const std::string &str, char delimiter = ' ');
    std::list<std::string_view> list_split_string(const std::string &str, char delimiter = ' ');
}// namespace util::str
