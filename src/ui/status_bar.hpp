//
// Created by 46769 on 2021-01-08.
//

#pragma once
#include <core/core.hpp>
#include <core/vector.hpp>

class TextData;
class BufferCursor;

namespace ui {
class View;

struct StatusBar {
    ~StatusBar();
    Boxed<View> ui_view;
    BufferCursor *buffer_cursor;
    Vec3f bg_color{0.25f, 0.25f, 0.27f};
    Vec3f font_color{0.9f, 0.8f, 0.6f};
    void draw(View *view);
    void set_buffer_cursor(BufferCursor *cursor);
    static StatusBar *create(int width, int height, int x, int y);
    void print_debug_info() const;
};
}