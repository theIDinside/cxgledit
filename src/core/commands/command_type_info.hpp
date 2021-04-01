//
// Created by cx on 2021-03-25.
//

#pragma once
#include <GLFW/glfw3.h>
#include <optional>
#include <string>
#include <string_view>


enum class Commands {
    OpenFile,
    WriteFile,
    WriteAllFiles,
    GotoLine,
    GotoBookmark,
    UserCommand,
    Search,
    Fail,
    GotoHeader,
    ReloadConfiguration,
    GotoSource
};
enum class Cycle : int {
    Forward = GLFW_KEY_DOWN,        // down
    Backward = GLFW_KEY_UP,       // up
};