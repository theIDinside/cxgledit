//
// Created by cx on 2021-03-25.
//

#include "editor_window.hpp"
#include "text_view.hpp"

#include "layout/box.hpp"
#include <ui/managers/font_library.hpp>
#include <ui/managers/widget_manager.hpp>

namespace cx::widget {
EditorWindow::~EditorWindow() {}
void EditorWindow::draw() {}

void EditorWindow::handle_mouse_click(Vec2i16 mousePos) {
    auto& meta_data = m_text_view->get_text_buffer()->meta_data;

    const auto row_clicked = std::floor(static_cast<float>(std::max(0, mousePos.y - m_status_bar->height())) /
                                        static_cast<float>(m_text_view->get_font()->get_pixel_row_advance())) +
                             static_cast<float>(m_text_view->get_cursor()->m_view_line_anchor);
    m_text_view->get_text_buffer()->step_cursor_to(static_cast<size_t>(
            meta_data.get(static_cast<size_t>(row_clicked)).value_or(meta_data.line_begins.back())));
}

void EditorWindow::set_view_colors(RGBColor bg, RGBColor whenActiveColor) {
    m_text_view->set_draw_colors(bg, whenActiveColor);
}
void EditorWindow::set_font(SimpleFont* pFont) {
    m_text_view->set_font_renderer(pFont);
    m_text_view->get_cursor()->set_size(
            {m_text_view->get_cursor()->width(), static_cast<i16>(pFont->max_glyph_height + 4)});
    m_status_bar->set_font_renderer(pFont);

}
void EditorWindow::set_caret_style(Configuration::Cursor style) {
    auto tv_cursor = m_text_view->get_cursor();
    tv_cursor->set_color(style.color);

    std::visit(
            [this, tv_cursor](auto&& style) {
                using T = std::decay_t<decltype(style)>;
                if constexpr (std::is_same_v<T, CaretStyleLine>) {
                    tv_cursor->set_width(static_cast<i16>(style.width));
                } else if constexpr (std::is_same_v<T, CaretStyleBlock>) {
                    tv_cursor->set_width(static_cast<i16>(this->m_text_view->get_font()->max_glyph_width - 2));
                } else {
                    static_assert(always_false_v<T>, "non exhaustive visitor");
                }
            },
            style.cursor_style);
}
FileContext EditorWindow::file_context() const { return m_text_view->get_text_buffer()->file_context(); }
const std::vector<Bookmark>& EditorWindow::get_bookmarks() const {
    return m_text_view->get_text_buffer()->meta_data.bookmarks;
}
void EditorWindow::set_bookmark() { m_text_view->get_text_buffer()->set_bookmark(); }
void EditorWindow::remove_bookmark(int index) {
    auto& md = m_text_view->get_text_buffer()->meta_data.bookmarks;
    md.erase(md.begin() + index);
}
void EditorWindow::set_configuration(const Configuration& configuration) {
    set_caret_style(configuration.cursor);
    set_view_colors(configuration.views.bg_color, configuration.views.active_bg);
}
EditorWindow::EditorWindow(int buffer_id, gui_id id, Widget* parent)
    : Widget(id, parent), m_buffer_id(buffer_id), m_status_bar(nullptr), m_text_view(nullptr), active(false) {}

EditorWindow* EditorWindow::create(Widget* parent, std::optional<TextData*> textData, Matrix projection) {
    if (textData) {
        auto handle = textData.value();
        auto& wm = WidgetManager::get_instance();
        auto ew = new EditorWindow{handle->id, wm.request_gui_id_for_child(parent), parent};
        auto bl = new cx::widget::BoxLayout{LayoutAxis::Vertical, 0, 0};
        ew->set_layout(bl);
        ew->m_status_bar = StatusBar::boxed_create(wm.request_gui_id_for_child(ew), ew, projection);
        ew->m_text_view = TextView::boxed_create_view(wm.request_gui_id_for_child(ew), ew, projection, textData);
        return ew;
    } else {

    }

    return nullptr;
}
void EditorWindow::set_size(Vec2i16 size) {
    Widget::set_size(size);
    this->m_status_bar->set_width(size.x);
    this->m_text_view->set_size({.x = size.x, .y = static_cast<short>(size.y - m_status_bar->height())});
}

EditorWindow* EditorWindow::create_with_size(Widget* parent, std::optional<TextData*> textData, Matrix projection,
                                             Vec2i16 size) {
    auto ew = create(parent, textData, projection);
    ew->set_size(size);
    return ew;
}

}// namespace cx::widget
