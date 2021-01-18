//
// Created by 46769 on 2021-01-18.
//

#include "matrix.hpp"

glm::mat4 orthographic_projection(int width, int height, int scrolled) {
    glm::mat4 mvp;
    glm::vec4 a{2/(float)width, 0, 0, 0};
    glm::vec4 b{0, 2/(float)height, 0, 0};
    glm::vec4 c{0, 0, -1, 0};
    glm::vec4 d{-1, -(height + scrolled) / (height - scrolled), 0, 1};
    mvp[0] = a;
    mvp[1] = b;
    mvp[2] = c;
    mvp[3] = d;
    return mvp;
}

glm::mat4 scrolled_projection(int width, int height, int scroll) {
    return orthographic_projection(width, height, scroll);
}
