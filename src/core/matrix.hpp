//
// Created by 46769 on 2021-01-18.
//

#pragma once
#include <glm/glm.hpp>

struct Matrix {

};

glm::mat4 orthographic_projection(int width, int height, int view_scrolled);
glm::mat4 scrolled_projection(int width, int height, int scroll);