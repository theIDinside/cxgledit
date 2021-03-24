//
// Created by cx on 2021-03-22.
//

#include "widget.hpp"
#include <algorithm>
#include <ranges>

namespace cx::widget {


Widget::Widget(gui_id widget_id, Widget* parent)
    : m_id(widget_id), m_size{}, m_fixed_width(), m_fixed_height(), m_pos{}, p_layout{nullptr},
      p_parent(parent), m_children{}, m_visible{true} {

}
Widget::~Widget() { delete p_layout; }

void Widget::set_layout(cx::widget::Layout* layout) { p_layout = layout; }

const Vec2i16& Widget::get_relative_position() const { return m_pos; }

std::optional<gui_id> Widget::find_widget(Vec2i16 pos) {
    if (m_children.empty()) return {};
    if (is_within_bounds(bounding_rect(), pos)) {
        auto it = std::ranges::find_if(m_children,
                                       [pos](auto child) { return is_within_bounds(child->bounding_rect(), pos); });
        if (it != std::end(m_children)) return (*it)->m_id;
        else
            return {};
    } else {
        return {};
    }
}

BoundingRect Widget::bounding_rect() const {
    return BoundingRect{.top_left = m_pos,
                        .bottom_right = {
                                .x = static_cast<i16>(m_pos.x + m_size.x),
                                .y = static_cast<i16>(m_pos.y + m_size.y),
                        }};
}

void Widget::add_child(Widget* widget) {
    widget->set_parent(this);
    m_children.push_back(widget);
}
std::vector<Widget*> Widget::children() const { return m_children; }

void Widget::set_height(int height) { m_size.y = height; }
i16 Widget::height() const { return m_size.y; }
void Widget::set_width(i16 width) { m_size.x = width; }
i16 Widget::width() const { return m_size.x; }
void Widget::set_size(Vec2i16 size) {
    m_size = size;
}
Vec2i16 Widget::size() const {
    return Vec2i16 {
        .x = m_fixed_width.value_or(m_size.x),
        .y = m_fixed_height.value_or(m_size.y)
    };
}

Vec2i16 Widget::calculate_absolute_position() const {
    if (p_parent) p_parent->calculate_absolute_position() + m_pos;
    else
        return m_pos;
}
Layout* Widget::get_layout() const { return p_layout; }
Layout* Widget::get_layout() { return p_layout; }
void Widget::set_parent(Widget* parentWidget) { p_parent = parentWidget; }
Widget* Widget::parent() { return p_parent; }
Widget* Widget::parent() const { return p_parent; }

bool Widget::is_fixed_size() const { return (m_fixed_height) || (m_fixed_width); }
void Widget::show() { m_visible = true; }
void Widget::hide() { m_visible = false; }
bool Widget::is_visible() const { return m_visible; }

void Widget::layout_widget() {
    if(p_layout) {
        p_layout->arrange_widget(this);
    } else {
        for(auto c : children()) {
            c->layout_widget();
        }
    }
}


void Widget::resize(std::optional<i16> maybe_width, std::optional<i16> maybe_height) {
    m_fixed_width = maybe_width;
    m_fixed_height = maybe_height;
    if(p_parent != nullptr) {
        p_parent->layout_widget();
    } else {
        for(auto w : children()) w->layout_widget();
    }
}
std::optional<i16> Widget::fixed_width() const { return m_fixed_width; }
std::optional<i16> Widget::fixed_height() const { return m_fixed_height; }

void Widget::set_anchor(Vec2i16 pos) {
    m_pos = pos;
}

constexpr bool is_within_bounds(BoundingRect bounds, Vec2i16 pos) {
    return (bounds.top_left.x <= pos.x && bounds.bottom_right.x >= pos.x) &&
           (bounds.top_left.y >= pos.y && bounds.bottom_right.y <= pos.y);
}

}// namespace cx::widget
