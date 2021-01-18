//
// Created by 46769 on 2021-01-13.
//

#include "vector.hpp"
#include <array>
#include <charconv>
#include <sstream>

bool operator==(const Vec2<float> &lhs, const Vec2<float> &rhs) {
    return (lhs.x == rhs.x) && (lhs.y == rhs.y);
}

std::ostream &operator<<(std::ostream &os, const Vec2<float>& v) {
    std::array<char, 15> buf;
    os << "\"";

    if(auto[ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v.x); ec == std::errc()) {
        os << std::string_view{buf.data(), ptr} << " ";
    }
    std::memset(buf.data(), 0, buf.size());
    if(auto[ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v.y); ec == std::errc()) {
        os << std::string_view{buf.data(), ptr} << "\"";
    }
    return os;
}


bool operator==(const Vec3<float> &lhs, const Vec3<float> &rhs) {
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
}

std::ostream &operator<<(std::ostream &os, const Vec3<float>& v) {
    std::array<char, 15> buf;
    os << "\"";

    if(auto[ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v.x); ec == std::errc()) {
        os << std::string_view{buf.data(), ptr} << " ";
    }
    std::memset(buf.data(), 0, buf.size());
    if(auto[ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v.y); ec == std::errc()) {
        os << std::string_view{buf.data(), ptr} << " ";
    }
    std::memset(buf.data(), 0, buf.size());
    if(auto[ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v.z); ec == std::errc()) {
        os << std::string_view{buf.data(), ptr} << "\"";
    }
    return os;
}

bool operator==(const Vec4<float> &lhs, const Vec4<float> &rhs) {
    return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
}

std::ostream &operator<<(std::ostream &os, const Vec4<float> &v) {
    std::array<char, 15> buf;
    os << "\"";

    if(auto[ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v.x); ec == std::errc()) {
        os << std::string_view{buf.data(), ptr} << " ";
    }
    std::memset(buf.data(), 0, buf.size());
    if(auto[ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v.y); ec == std::errc()) {
        os << std::string_view{buf.data(), ptr} << " ";
    }
    std::memset(buf.data(), 0, buf.size());
    if(auto[ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v.z); ec == std::errc()) {
        os << std::string_view{buf.data(), ptr} << " ";
    }
    std::memset(buf.data(), 0, buf.size());
    if(auto[ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), v.w); ec == std::errc()) {
        os << std::string_view{buf.data(), ptr} << "\"";
    }

    return os;
}