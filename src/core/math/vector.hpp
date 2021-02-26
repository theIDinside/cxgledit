//
// Created by 46769 on 2021-01-13.
//

#pragma once
#include <cassert>
#include <core/core.hpp>
#include <glad/glad.h>
#include <stdexcept>

using usize = std::size_t;

constexpr bool vec_index_assertion(const char *fn_name, const char *file, int line_number, int index, int vec_size) {
    if (not(index < vec_size)) {
        util::println("Assertion failed in {} ({}:{}): Type: 'Index assertion': {} not < {}", fn_name, file,
                      line_number, index, vec_size);
        std::abort();
    }
    return true;
}

#define VEC_INDEX_ASSERTION(param, vec_size)                                                                           \
    if constexpr (DEBUG_IS_ON) {                                                                                       \
        if (not(static_cast<int>((param)) < static_cast<int>((vec_size)))) {                                           \
            vec_index_assertion(__FUNCSIG__, __FILE__, __LINE__, param, vec_size);                                     \
        }                                                                                                              \
    }

template<typename T>
struct Vec2 {
    T x, y;
    constexpr T &operator[](usize i) noexcept {
        VEC_INDEX_ASSERTION(static_cast<int>(i), 2);
        auto first = reinterpret_cast<T *>(this);
        return *(first + i);
    }

    constexpr const T &operator[](usize i) const noexcept {
        VEC_INDEX_ASSERTION(static_cast<int>(i), 2);
        auto first = reinterpret_cast<const T *>(this);
        return *(first + i);
    }

    friend bool operator==(const Vec2 &lhs, const Vec2 &rhs);
    friend std::ostream &operator<<(std::ostream &os, const Vec2 &v);
};

template<typename T>
struct Vec3 {
    T x, y, z;
    constexpr T &operator[](usize i) noexcept {
        VEC_INDEX_ASSERTION(static_cast<int>(i), 3);
        auto first = reinterpret_cast<T *>(this);
        return *(first + i);
    }

    constexpr const T &operator[](usize i) const noexcept {
        VEC_INDEX_ASSERTION(static_cast<int>(i), 3);
        auto first = reinterpret_cast<const T *>(this);
        return *(first + i);
    }

    friend bool operator==(const Vec3 &lhs, const Vec3 &rhs);
    friend std::ostream &operator<<(std::ostream &os, const Vec3 &v);
    constexpr auto capacity() const { return 3; }
};

template<typename T>
struct Vec4 {
    T x, y, z, w;

    constexpr T &operator[](usize i) noexcept {
        VEC_INDEX_ASSERTION(static_cast<int>(i), 4);
        auto first = reinterpret_cast<T *>(this);
        return *(first + i);
    }

    constexpr const T &operator[](usize i) const noexcept {
        VEC_INDEX_ASSERTION(static_cast<int>(i), 4);
        auto first = reinterpret_cast<const T *>(this);
        return *(first + i);
    }

    friend bool operator==(const Vec4 &lhs, const Vec4 &rhs);
    friend std::ostream &operator<<(std::ostream &os, const Vec4 &v);
    constexpr auto capacity() const { return 4; }
};

using Vec2i = Vec2<int>;
using Vec2f = Vec2<GLfloat>;
using Vec2d = Vec2<double>;

using Vec3i = Vec3<int>;
using Vec3f = Vec3<GLfloat>;
using Vec3d = Vec3<double>;

using Vec4i = Vec4<int>;
using Vec4f = Vec4<GLfloat>;
using Vec4d = Vec4<double>;

using RGBColor = Vec3f;
using RGBAColor = Vec4f;