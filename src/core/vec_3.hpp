//
// Created by 46769 on 2021-01-13.
//

#pragma once
#include <cassert>
#include <stdexcept>

template<typename T>
struct Vec3 {
    T x, y, z;

    T &operator[](int i) {
        assert(i < 3);
        switch (i) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            default: {
                throw std::runtime_error{"Out of bounds access of Vec3"};
            }
        }
    }
    friend bool operator==(const Vec3& lhs, const Vec3& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Vec3 v);
};


using Vec3i = Vec3<int>;
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;