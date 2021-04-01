//
// Created by cx on 2021-03-25.
//
#pragma once
#include "widget.hpp"

class Matrix;
class BufferCursor;
class SimpleFont;

namespace cx::widget {

class TextView;
class EditorWindow;

class StatusBar : public Widget {
public:
    friend class EditorWindow;
    StatusBar(gui_id id, Widget* parent);
    void draw() override;
    ~StatusBar() override;

    static StatusBar* create(gui_id id, Widget* parent, Matrix mvp);
    static Boxed<StatusBar> boxed_create(gui_id id, Widget* parent, Matrix mvp);
    void set_font_renderer(SimpleFont* font);

private:
    Vec3f m_bg_color;
    Vec3f m_font_color;
    Boxed<TextView> m_text_view;
    BufferCursor* m_buf_cursor;
};

}// namespace cx::widget
