//
// Created by 46769 on 2021-01-13.
//

#include "vec_3.hpp"
#include <sstream>
#include <array>
#include <charconv>

bool operator==(const Vec3<float> &lhs, const Vec3<float> &rhs) {
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
}

std::ostream &operator<<(std::ostream &os, const Vec3<float> v) {
    std::array<char, 15> x;
    std::array<char, 15> y;
    std::array<char, 15> z;

    os << "\"";

    if(auto[ptr, ec] = std::to_chars(x.data(), x.data() + x.size(), v.x); ec == std::errc()) {
        os << std::string_view{x.data(), ptr} << " ";
    }
    if(auto[ptr, ec] = std::to_chars(y.data(), y.data() + y.size(), v.y); ec == std::errc()) {
        os << std::string_view{y.data(), ptr} << " ";
    }

    if(auto[ptr, ec] = std::to_chars(z.data(), z.data() + z.size(), v.z); ec == std::errc()) {
        os << std::string_view{z.data(), ptr} << "\"";
    }
    return os;
}
