//
// Created by cx on 2021-03-25.
//

#include "status_bar.hpp"
#include <core/buffer/data_manager.hpp>
#include <ui/widgets/text_view.hpp>
#include <ui/managers/widget_manager.hpp>
#include "layout/single_fill.hpp"

cx::widget::StatusBar::~StatusBar() {}
void cx::widget::StatusBar::draw() {}

cx::widget::StatusBar* cx::widget::StatusBar::create(gui_id id, Widget* parent, Matrix mvp) {
    auto sb_text = DataManager::get_instance().create_free_buffer(BufferType::StatusBar);
    auto sb = new StatusBar{id, parent};
    sb->set_layout(new SingleFill{});
    auto ui_view = std::unique_ptr<TextView>(TextView::create_view(WidgetManager::get_instance().request_gui_id_for_child(sb), sb, mvp, sb_text));
    sb->m_fixed_height = sb->m_text_view->get_font()->get_pixel_row_advance() + 2;
    sb->m_text_view = std::move(ui_view);
    sb->m_buf_cursor = &sb_text->cursor;
    return sb;
}

cx::widget::StatusBar::StatusBar(gui_id id, Widget* parent) : Widget(id, parent) {}

Boxed<cx::widget::StatusBar> cx::widget::StatusBar::boxed_create(gui_id id, Widget* parent, Matrix mvp) {
    return std::unique_ptr<StatusBar>(create(id, parent, mvp));
}

void cx::widget::StatusBar::set_font_renderer(SimpleFont* font) {
    m_text_view->set_font_renderer(font);
    m_fixed_height = font->get_pixel_row_advance() + 2;
}
