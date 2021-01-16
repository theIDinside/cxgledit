//
// Created by 46769 on 2020-12-27.
//

#pragma once

#include <glm/glm.hpp>
#include <ui/render/vertex_buffer.hpp>
#include <ui/render/shader.hpp>

namespace ui {
    class View;

    struct ViewCursor {

        /// Called from within View's constructor/factory method
        /// N.B.!! Must take a *fully* and *well formed/created* View object (pointer to)
        /// as it will use this pointer to query the TextData buffer object where it's cursor is located at
        /// within the buffer
        static std::unique_ptr<ViewCursor> create_from(std::unique_ptr<View> &owning_view);
        static std::unique_ptr<ViewCursor> create_from(View* owning_view);

        void update_cursor_data(GLfloat x, GLfloat y);
        void draw(bool isActive = false);
        void forced_draw();

        void set_line_rect(GLfloat x1, GLfloat x2, GLfloat y1);
        void set_line_rect(GLfloat x1, GLfloat x2, GLfloat y1, int height);

        void set_projection(glm::mat4 orthoProjection);
        void setup_dimensions(int Width, int Height);

        int index{0};/// absolute position in text buffer
        glm::mat4 projection;
        View *view = nullptr;
        Shader *shader = nullptr;
        int line{};
        int width{};
        int height{};
        int pos_x;
        int pos_y;
        std::unique_ptr<CursorVAO> cursor_data;
        std::unique_ptr<CursorVAO> line_shade_data;
    };
}