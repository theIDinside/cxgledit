//
// Created by 46769 on 2021-01-08.
//

#include "status_bar.hpp"
#include <app.hpp>
#include <core/buffer/data_manager.hpp>
#include <core/buffer/text_data.hpp>
#include <ui/view.hpp>

namespace ui {
//! -----------------------------------------------------

StatusBar *StatusBar::create(int width, int height, int x, int y) {
    auto status_bar_text = DataManager::get_instance().create_free_buffer(BufferType::StatusBar);
    auto ui_view = View::create_managed(status_bar_text, "status_bar", width, height, x, y);
    BufferCursor *cursor_info = &status_bar_text->cursor;
    auto sb = new StatusBar{};
    sb->ui_view = std::move(ui_view);
    sb->buffer_cursor = cursor_info;
    return sb;
}

void StatusBar::set_buffer_cursor(BufferCursor *cursor) { buffer_cursor = cursor; }

void StatusBar::draw(View *view) {
    glEnable(GL_SCISSOR_TEST);
    auto dims = ::App::get_window_dimension();
    glScissor(ui_view->x, dims.h - (ui_view->height + 4), ui_view->width, ui_view->height + 4);
    glClearColor(bg_color.x, bg_color.y, bg_color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto buf = view->get_text_buffer();
    auto fName = buf->meta_data.buf_name;

    if (fName.empty()) { fName = "*unnamed buffer*"; }

    auto output = fmt::format("{} - [{}, {}]", fName, buffer_cursor->line, buffer_cursor->col_pos);
    ui_view->get_text_buffer()->clear();
    ui_view->get_text_buffer()->insert_str(output);
    ui_view->draw_statusbar();
    glDisable(GL_SCISSOR_TEST);
}
void StatusBar::print_debug_info() const {
    auto output = fmt::format("[{}, {}]", buffer_cursor->line, buffer_cursor->col_pos);
    util::println("Text to display: '{}'", output);
}
StatusBar::~StatusBar() {
    util::println("Destroying status bar");
}
}// namespace ui
