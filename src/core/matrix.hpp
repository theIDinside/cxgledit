//
// Created by 46769 on 2021-01-18.
//

#pragma once
#include "vector.hpp"

struct Matrix {
    Vec4f data[4];

    Vec4f& operator[](std::size_t index) {
        assert(index < 4);
        return data[index];
    }

    const Vec4f& operator[](std::size_t index) const {
        assert(index < 4);
        return data[index];
    }
};

Matrix my_screen_projection_2D(int width, int height, int view_scrolled);