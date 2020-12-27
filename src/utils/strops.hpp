//
// Created by cx on 2020-11-19.
//
#pragma once
#include <cstdint>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

struct CountResult {
    size_t found_at_idx;
};

/**
 * Counts elements with value ch in container str. Returns an optional containing a vector of CountResult, which is
 * a thin wrapper around integer values holding the index where the element was found.
 * @tparam ContainerType
 * @tparam ElemType
 * @param str
 * @param ch
 * @return
 */
template<typename ContainerType, typename ElemType>
std::optional<std::vector<CountResult>> count_elements(const ContainerType &str, ElemType ch) {
    static_assert(std::is_same_v<typename ContainerType::value_type, ElemType>,
                  "Container must contain elements of type ElemType");
    std::vector<CountResult> res;
    auto index = 0;
    for (const auto &c : str) {
        if (c == ch) { res.push_back(CountResult{static_cast<size_t>(index)}); }
        index++;
    }
    if (!res.empty()) return res;
    else
        return {};
}

/*
 * std::optional<int> a = 10;
 * std::optional<int> b = {};
 * or(a, [](auto e) {
 *
 * })
 *
 */

template<typename T, typename MapTo, typename Fn>
constexpr auto map_or(std::optional<T> opt, MapTo or_value, Fn fn) -> MapTo {
    if (opt) {
        return fn(*opt);
    } else {
        return or_value;
    }
}