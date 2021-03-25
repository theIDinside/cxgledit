//
// Created by cx on 2021-03-25.
//

#pragma once
// We don't need to include this.. It's included via status_bar and text_view
// #include "widget.hpp"

#include "status_bar.hpp"
#include "text_view.hpp"

#include <memory>

namespace cx::widget {

class EditorWindow : public Widget {
public:
    EditorWindow(int buffer_id, gui_id id, Widget* parent);
    ~EditorWindow() override;
    void draw() override;
private:
    int m_buffer_id;
    Boxed<cx::widget::StatusBar> m_status_bar;
    Boxed<cx::widget::TextView> m_text_view;
};
}