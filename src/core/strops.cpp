//
// Created by cx on 2020-11-27.
//

#include "strops.hpp"
#include "core.hpp"

#ifdef INTRINSICS_ENABLED
#include <immintrin.h>
/*
 * does broad word scanning of data, it scans 8 bytes in "parallell", checking for new lines
 * this does _not_ have to be done this complicated and manually. The compiler actually does this for us
 * as soon as we set optimization level to above "none". However, with SIMD & AVX2, we can instead of 8 bytes,
 * scan 32 bytes, which should increase the speed a bit further. But holy shit that's a lot of unnecessary work for
 * what is relatively small amount of textual data. Scanning a file with 5000 lines, takes in the order of ~40-50 ns with
 * the optimization set to O3 and doing the naive for-loop
*/
optimization turned on u64 newline_count_64bit(const char *data, std::size_t len) {
    constexpr auto WORD_SIZE = 8;// 8 bytes = 64 bit
    constexpr auto PEXT_FACTOR = 0x0101010101010101L;
    auto pop_count = 0;
    auto total_iterations = len / WORD_SIZE;
    auto mask = "\n\n\n\n\n\n\n\n";
    auto wm = *(u64 *) mask;

    for (auto i = 0; i < total_iterations; i++) {
        auto index = i * 8;
        auto w0 = ~(*(u64 *) (data + index) ^ wm);
        auto w1 = (w0 >> 4) & w0;
        auto w2 = (w1 >> 2) & w1;
        auto w3 = (w2 >> 1) & w2;

        auto bits = _pext_u64(w3, PEXT_FACTOR);

        // below translates to (on my system) to pop_count = _mm_popcnt_u64(bits);
        pop_count += __builtin_popcount(bits);
    }
    for (auto i = total_iterations * WORD_SIZE; i < len; ++i) {
        if (data[i] == '\n') pop_count++;
    }
    return pop_count;
}

#else
u64 naive_newline_count(const char *data, std::size_t len) {
    auto res = 0;
    for (auto i = 0; i < len; i++)
        if (data[i] == '\n') res++;
    return res;
}
#endif
namespace str {

    u64 count_newlines(const char *data, std::size_t length) {
#ifdef INTRINSICS_ENABLED
        return newline_count_64bit(data, length);
#else
        return naive_newline_count(data, length);
#endif
    }
}// namespace str

using Result = std::vector<std::string_view>;
std::vector<std::string_view> util::str::vec_split_string(const std::string &str, const char delimiter) {
    std::string_view v{str};
    std::vector<std::string_view> res;
    while(!v.empty()) {
        auto pos = v.find(delimiter);
        res.push_back(v.substr(0, pos));
        v.remove_prefix(pos);
    }

    return res;
}

std::list<std::string_view> util::str::list_split_string(const std::string &str, const char delimiter) {
    std::string_view v{str};
    std::list<std::string_view> res;
    while(!v.empty()) {
        auto pos = v.find(delimiter);
        if(pos == std::string_view::npos) {
            res.push_back(v);
            v.remove_prefix(v.size());
        } else {
            res.push_back(v.substr(0, pos));
            v.remove_prefix(pos+1);
        }
    }
    return res;
}
