//
// Created by cx on 2021-03-25.
//

#include "widget_manager.hpp"
#include <ranges>
#include <algorithm>

WidgetManager& WidgetManager::get_instance() {
    static WidgetManager wm;
    return wm;
}

gui_id WidgetManager::request_gui_id() {
    auto max = std::ranges::max_element(m_living_widgets, [](auto a, auto b) {
        return a.id < b.id;
    });

    if(max != std::end(m_living_widgets)) {
        return gui_id{++max->id};
    } else {
        return gui_id{1};
    }
}

gui_id WidgetManager::request_gui_id_for_child(Widget* parent) {
    assert(parent);
    auto max_id = 0;
    for(const auto w : m_living_widgets) {
        max_id = std::max(w.id, max_id);
    }
    return {max_id + 1};
}
void WidgetManager::set_screen_size(Vec2i16 size) {
    m_screen_size = size;
}

Vec2i16 WidgetManager::screen_size() const {
    return m_screen_size;
}
