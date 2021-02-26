//
// Created by 46769 on 2020-12-27.
//

#pragma once

#include <ui/render/shader.hpp>
#include <ui/render/vertex_buffer.hpp>
#include <core/math/matrix.hpp>

namespace ui {
    struct View;

    struct ViewCursor {

        /// Called from within View's constructor/factory method
        /// N.B.!! Must take a *fully* and *well formed/created* View object (pointer to)
        /// as it will use this pointer to query the TextData buffer object where it's cursor is located at
        /// within the buffer
        static std::unique_ptr<ViewCursor> create_from(std::unique_ptr<View> &owning_view);
        static std::unique_ptr<ViewCursor> create_from(View* owning_view);

        void update_cursor_data(GLfloat x, GLfloat y);
        void draw();
        void forced_draw();

        void set_line_rect(GLfloat x1, GLfloat x2, GLfloat y1);
        void set_line_rect(GLfloat x1, GLfloat x2, GLfloat y1, int height);

        // void set_projection(glm::mat4 orthoProjection);
        void set_projection(Matrix orthoProjection);
        void setup_dimensions(int Width, int Height);

        int index{0};/// absolute position in text buffer
        Matrix mvp;
        View *view = nullptr;
        Shader *shader = nullptr;
        int views_top_line{};
        int width{};
        int height{};
        int pos_x;
        int pos_y;
        RGBAColor caret_color = {1.0f, 0.0f, 0.2f, .4f};
        std::unique_ptr<CursorVAO> cursor_data;
        std::unique_ptr<CursorVAO> line_shade_data;
    };
}