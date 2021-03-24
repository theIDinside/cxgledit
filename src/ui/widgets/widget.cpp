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
    if (parent != nullptr) { parent->add_child(this); }
}
Widget::~Widget() { delete p_layout; }

void Widget::set_layout(cx::widget::Layout* layout) { p_layout = layout; }

const Vec2i16& Widget::get_relative_position() const { return m_pos; }

std::optional<gui_id> Widget::find_widget(Vec2i16 pos) {
    const auto abs = calculate_absolute_position();
    util::println("Searching in relative position @ ({}, {}). Widget absolute anchor position ({}, {}) - i.e we are looking at absolute pixel position ({}, {})", pos.x, pos.y, abs.x, abs.y, (pos + abs).x, (pos + abs).y);
    if (m_children.empty()) return {};
    if (is_within_bounds(bounding_rect(), pos + m_pos)) {
        util::println("Pos ({}, {}) is within this widget: {}.", pos.x, pos.y, m_id.id);
        auto it = std::ranges::find_if(m_children,
                                       [pos](auto child) { return is_within_bounds(child->bounding_rect(), pos); });
        if (it != std::end(m_children)) {

            auto rel_pos = pos - (*it)->m_pos;
            util::println("Pos ({}, {}) in widget: {}, also is in widget {}, at it's relative position ({}, {})", pos.x, pos.y, m_id.id, (*it)->m_id.id, rel_pos.x, rel_pos.y);

            auto t = abs + pos;
            util::println("Searching for children at relative position ({}, {}) in widget {} - Abs position: {}, {}", rel_pos.x, rel_pos.y, (*it)->m_id.id, t.x, t.y);
            if(auto r = (*it)->find_widget(rel_pos); r) return r;
            else return (*it)->m_id;
        }
        else
            return {};
    } else {
        return {};
    }
}

BoundingRect Widget::bounding_rect() const { return BoundingRect{.top_left = m_pos, .bottom_right = m_pos + m_size}; }

void Widget::add_child(Widget* widget) {
    widget->set_parent(this);
    m_children.push_back(widget);
}
std::vector<Widget*> Widget::children() const { return m_children; }

void Widget::set_height(int height) { m_size.y = height; }
i16 Widget::height() const { return m_size.y; }
void Widget::set_width(i16 width) { m_size.x = width; }
i16 Widget::width() const { return m_size.x; }
void Widget::set_size(Vec2i16 size) { m_size = size; }
Vec2i16 Widget::size() const {
    return Vec2i16{.x = m_fixed_width.value_or(m_size.x), .y = m_fixed_height.value_or(m_size.y)};
}

Vec2i16 Widget::calculate_absolute_position() const {
    if (p_parent != nullptr) {
        return p_parent->calculate_absolute_position() + m_pos;
    } else
        return m_pos;
}
Layout* Widget::get_layout() const { return p_layout; }
void Widget::set_parent(Widget* parentWidget) { p_parent = parentWidget; }
Widget* Widget::parent() const { return p_parent; }

bool Widget::is_fixed_size() const { return (m_fixed_height) || (m_fixed_width); }
void Widget::show() { m_visible = true; }
void Widget::hide() { m_visible = false; }
bool Widget::is_visible() const { return m_visible; }

void Widget::layout_widget() {
    if(!m_children.empty()) {
        if (p_layout) {
            p_layout->arrange_widget(this);
            for(auto c : children()) {
                c->layout_widget();
            }
        } else {
            for (auto c : children()) { c->layout_widget(); }
        }
    }
}

void Widget::resize(std::optional<i16> maybe_width, std::optional<i16> maybe_height) {
    m_fixed_width = maybe_width;
    m_fixed_height = maybe_height;
    if (p_parent != nullptr) {
        p_parent->layout_widget();
    } else {
        if (p_layout) {
            p_layout->arrange_widget(this);
        } else {
            for (auto w : children()) w->layout_widget();
        }
    }
}
std::optional<i16> Widget::fixed_width() const { return m_fixed_width; }
std::optional<i16> Widget::fixed_height() const { return m_fixed_height; }

void Widget::set_anchor(Vec2i16 pos) { m_pos = pos; }
gui_id Widget::get_guid() const { return m_id; }
int Widget::get_id() const { return m_id.id; }

constexpr bool is_within_bounds(BoundingRect bounds, Vec2i16 pos) {
    auto res = (bounds.top_left.x <= pos.x && bounds.bottom_right.x >= pos.x) &&
           (bounds.top_left.y <= pos.y && bounds.bottom_right.y >= pos.y);
    return res;
}

}// namespace cx::widget
