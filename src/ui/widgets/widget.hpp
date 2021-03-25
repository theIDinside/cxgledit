//
// Created by cx on 2021-03-22.
//

#pragma once

#include <core/math/vector.hpp>
#include <ui/widgets/layout/layout.hpp>

namespace cx::widget {
/// This is wrapped inside a struct, because I *think* I might add other fields here. And if I ever have to remove it
/// it will be pretty easy
struct gui_id {
    int id;
    constexpr auto operator<=>(const gui_id& rhs) const = default;
};

struct BoundingRect {
    Vec2i16 top_left;
    Vec2i16 bottom_right;
};

constexpr inline bool is_within_bounds(BoundingRect bounds, Vec2i16 pos);

/// We want Widgets to kind of behave/feel like they do in GUI toolkits like Qt, only without the extra stuff
class Widget {
public:
    /// The user of this type, explicitly must managed how widget_id's are assigned and handed out.
    explicit Widget(gui_id widget_id, Widget* parent = nullptr);
    virtual ~Widget();

    [[nodiscard]] gui_id get_guid() const;
    [[nodiscard]] int get_id() const;

    [[nodiscard]] Widget* parent() const;
    void set_parent(Widget* parentWidget);

    [[nodiscard]] Layout* get_layout() const;
    void set_layout(Layout* layout);

    [[nodiscard]] const Vec2i16& get_relative_position() const;
    [[nodiscard]] Vec2i16 calculate_absolute_position() const;
    [[nodiscard]] Vec2i16 size() const;
    [[nodiscard]] bool is_fixed_size() const;

    void set_size(Vec2i16 size);
    [[nodiscard]] i16 width() const;
    [[nodiscard]] std::optional<i16> fixed_width() const;

    /// Calling resize() on a widget, triggers a layout action, layout out the children of that widget according to the new size
    /// which is what differs this member function from set_size() which does no such thing.
    void resize(std::optional<i16> maybe_width, std::optional<i16> maybe_height);
    void set_width(i16 width);
    [[nodiscard]] i16 height() const;
    [[nodiscard]] std::optional<i16> fixed_height() const;
    void set_height(int height);

    std::optional<gui_id> find_widget(Vec2i16 pos);
    [[nodiscard]] inline std::vector<Widget*> children() const;
    void add_child(Widget* widget);

    void set_anchor(Vec2i16 pos);

    [[nodiscard]] BoundingRect bounding_rect() const;
    [[nodiscard]] bool is_visible() const;

    // We don't need fancy massive-batching of draw calls, as we're only drawing a GUI for a text editor.
    // It's not like we need to squeeze out 200 FPS.
    virtual void draw() = 0;
    void show();
    void hide();

    /// Lays out this widget and it's children, according to whatever layout that is or is not registered with these
    /// widgets.
    void layout_widget();

private:
    gui_id m_id;
    Vec2i16 m_size;
    std::optional<i16> m_fixed_width;
    std::optional<i16> m_fixed_height;
    Vec2i16 m_pos;
    Layout* p_layout;
    Widget* p_parent;
    std::vector<Widget*> m_children;
    bool m_visible;
};
}// namespace cx::widget