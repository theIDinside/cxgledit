//
// Created by 46769 on 2021-01-18.
//

#include "matrix.hpp"

Matrix my_screen_projection_2D(int width, int height, int scrolled) {
    Matrix m;
    Vec4f a{2/(float)width, 0, 0, 0};
    Vec4f b{0, 2/(float(height - scrolled)), 0, 0};
    Vec4f c{0, 0, -1, 0};
    Vec4f d{-1, - (float(height + scrolled) / float(height - scrolled)), 0, 1};
    m[0] = a;
    m[1] = b;
    m[2] = c;
    m[3] = d;
    return m;
}