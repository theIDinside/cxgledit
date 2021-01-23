//
// Created by 46769 on 2021-01-13.
//

#pragma once
#include <cassert>
#include <stdexcept>
#include <glad/glad.h>

template<typename T>
struct Vec2 {
    T x, y;
    T &operator[](int i) noexcept {
        assert(i < 3);
        switch (i) {
            case 0:
                return x;
            case 1:
                return y;
        }
    }

    const T& operator[](int i) const noexcept {
        assert(i < 3);
        switch (i) {
            case 0:
                return x;
            case 1:
                return y;
        }
    }

    friend bool operator==(const Vec2& lhs, const Vec2& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Vec2& v);
};

template<typename T>
struct Vec3 {
    T x, y, z;
    T &operator[](int i) noexcept {
        assert(i < 3);
        switch (i) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
        }
    }

    const T& operator[](int i) const noexcept {
        assert(i < 3);
        switch (i) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
        }
    }

    friend bool operator==(const Vec3& lhs, const Vec3& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Vec3& v);
};

template <typename T>
struct Vec4 {
    T x, y, z, w;

    T &operator[](int i) noexcept {
        assert(i < 4);
        switch (i) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            case 3:
                return w;
        }
    }

    const T& operator[](int i) const noexcept {
        assert(i < 4);
        switch (i) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            case 3:
                return w;
        }
    }

    friend bool operator==(const Vec4& lhs, const Vec4& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Vec4& v);
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