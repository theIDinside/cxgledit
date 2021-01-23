//
// Created by 46769 on 2021-01-23.
//

#pragma once
#include <variant>

struct CaretStyleBlock {};
struct CaretStyleLine {
    int width;
};
using CaretStyleOption = std::variant<CaretStyleLine, CaretStyleBlock>;

template<typename TResult>
constexpr std::string_view serialize_caret_option(const CaretStyleOption &style, TResult &writeTo) {
    auto pattern_match = [&](auto arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, CaretStyleLine>) {
          writeTo = arg.width;
          return "line";
      } else if constexpr (std::is_same_v<T, CaretStyleBlock>) {
          return "block";
      } else {
          static_assert(always_false_v<T>, "non exhaustive pattern match");
      }
    };
    return std::visit(pattern_match, style);
}