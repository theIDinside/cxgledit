//
// Created by cx on 2021-03-25.
//

#pragma once
#include <variant>

class Shader;
class Matrix;
class SimpleFont;

struct TextRenderable {
    Shader* shader;
    Matrix* matrix;
    SimpleFont* font;
};

struct DefaultRenderable {
    Shader* shader;
    Matrix* matrix;
};

struct RenderableObjectConfig {
    std::variant<TextRenderable, DefaultRenderable> param;
};