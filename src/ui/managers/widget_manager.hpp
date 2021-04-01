//
// Created by cx on 2021-03-25.
//

#pragma once
#include <ui/widgets/widget.hpp>

using cx::widget::gui_id;
using cx::widget::Widget;

class WidgetManager {
public:
    WidgetManager(const WidgetManager&) = delete;
    static WidgetManager& get_instance();
    gui_id request_gui_id();
    gui_id request_gui_id_for_child(Widget* parent);
    void set_screen_size(Vec2i16 size);
    [[nodiscard]] Vec2i16 screen_size() const;
private:
    WidgetManager() = default;
    std::vector<gui_id> m_living_widgets;
    Vec2i16 m_screen_size;
};