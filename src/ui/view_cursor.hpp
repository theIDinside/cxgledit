//
// Created by 46769 on 2020-12-27.
//

#pragma once

#include <glm/glm.hpp>
#include <ui/render/vertex_buffer.hpp>
#include <ui/render/shader.hpp>

class View;

struct ViewCursor {
    int line;
    int col;
    int row;
    int index; /// absolute position in text buffer

    int width, height;

    std::unique_ptr<CursorVAO> gpu_data;
    glm::mat4 projection;
    View* view;
    Shader *shader = nullptr;
    /// Called from within View's constructor/factory method
    /// N.B.!! Must take a *fully* and *well formed/created* View object (pointer to)
    /// as it will use this pointer to query the TextData buffer object where it's cursor is located at
    /// within the buffer
    static std::unique_ptr<ViewCursor> create_from(std::unique_ptr<View>& owning_view);


    void setup_dimensions(int width, int height);
    void update_cursor_data(GLfloat x, GLfloat y);
    void draw();
    void forced_draw();
    void set_projection(glm::mat4 orthoProjection);
};