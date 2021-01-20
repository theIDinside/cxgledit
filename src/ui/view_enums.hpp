//
// Created by 46769 on 2021-01-20.
//

#pragma once
#include <GLFW/glfw3.h>

namespace ui {
enum class ViewType { Text, Command, Modal };

enum class Scroll {
    Up = GLFW_KEY_UP,
    Down = GLFW_KEY_DOWN,
};
}// namespace ui