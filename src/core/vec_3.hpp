//
// Created by 46769 on 2021-01-13.
//

#pragma once
template <typename T>
struct Vec3 {
    T x, y, z;
};

using Vec3i = Vec3<int>;
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;