//
// Created by 46769 on 2021-01-08.
//

#include "editor_window.hpp"
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>
#include <core/data_manager.hpp>
#include <ui/status_bar.hpp>
#include <ui/view.hpp>

/// The optional<TD*> is *ONLY* to express the purpose that this value can be nil
/// just having a pointer here, could *very* well lead me to believe at some point that null can't be passed here
/// when it can.

namespace ui {

EditorWindow *EditorWindow::create(std::optional<TextData *> textData, Matrix projection, int layout_id,
                                   core::DimInfo dimInfo) {
    auto &[x, y, width, height] = dimInfo;
    auto sb_height = FontLibrary::get_default_font()->get_row_advance() + 2;
    // we have to make room for status bar & command view spanning across entire bottom, both of which are equal in height
    auto text_editor_height = height - (sb_height * 2);
    auto ew = new EditorWindow{};
    ew->ui_layout_id = layout_id;
    ew->dimInfo = dimInfo;
    if (textData) {
        ew->status_bar = StatusBar::create(width, sb_height, x, height);
        ew->view = View::create(*textData, "unnamed buffer", width, text_editor_height, x, height - sb_height);
    } else {
        auto buf = DataManager::get_instance().create_managed_buffer(BufferType::CodeInput);
        ew->status_bar = StatusBar::create(width, sb_height, x, height);
        ew->view = View::create(buf, "unnamed buffer", width, text_editor_height, x, height - sb_height);
    }

    ew->view->set_projection(projection);
    ew->status_bar->ui_view->set_projection(projection);
    ew->status_bar->set_buffer_cursor(&ew->view->get_text_buffer()->cursor);
    return ew;
}
/*
EditorWindow *EditorWindow::create(std::optional<TextData *> textData, glm::mat4 projection, int layout_id,
                                   core::DimInfo dimInfo) {
    auto &[x, y, width, height] = dimInfo;
    auto sb_height = FontLibrary::get_default_font()->get_row_advance() + 2;
    // we have to make room for status bar & command view spanning across entire bottom, both of which are equal in height
    auto text_editor_height = height - (sb_height * 2);
    auto ew = new EditorWindow{};
    ew->ui_layout_id = layout_id;
    ew->dimInfo = dimInfo;
    if (textData) {
        ew->status_bar = StatusBar::create(width, sb_height, x, height);
        ew->view = View::create(*textData, "unnamed buffer", width, text_editor_height, x, height - sb_height);
    } else {
        auto buf = DataManager::get_instance().create_managed_buffer(BufferType::CodeInput);
        ew->status_bar = StatusBar::create(width, sb_height, x, height);
        ew->view = View::create(buf, "unnamed buffer", width, text_editor_height, x, height - sb_height);
    }

    ew->view->set_projection(mvp);
    ew->status_bar->ui_view->set_projection(projection);
    ew->status_bar->set_buffer_cursor(&ew->view->get_text_buffer()->cursor);
    return ew;
}*/

EditorWindow::~EditorWindow() {
    delete view;
    delete status_bar;
}

void EditorWindow::draw(bool force_redraw) {
    if (force_redraw) {
        view->forced_draw(this->active);
    } else {
        view->draw(this->active);
    }
    this->status_bar->draw(view);
}

TextData *EditorWindow::get_text_buffer() const { return view->get_text_buffer(); }

void EditorWindow::update_layout(core::DimInfo dim_info) {
    auto &[x, y, width, height] = dim_info;
    auto sb_height = FontLibrary::get_default_font()->get_row_advance() + 2;
    // we have to make room for status bar & command view spanning across entire bottom, both of which are equal in height
    auto text_editor_height = height - (sb_height * 2);
    auto text_editor_y_pos = height - sb_height;
    view->anchor_at(x, text_editor_y_pos);
    view->set_dimensions(width, text_editor_height);
    status_bar->ui_view->set_dimensions(width, sb_height);
    status_bar->ui_view->anchor_at(x, height);
    this->dimInfo = dim_info;
}
/*
void EditorWindow::set_projection(glm::mat4 projection) const {
    this->view->set_projection(projection);
    this->status_bar->ui_view->set_projection(projection);
}
 */

void EditorWindow::set_projection(Matrix projection) const {
    this->view->set_projection(projection);
    this->status_bar->ui_view->set_projection(projection);
}

void EditorWindow::handle_click(int x, int yPOS) {
    if (dimInfo.is_inside(x, yPOS)) {
        auto &meta_data = view->get_text_buffer()->meta_data;

        auto row_clicked = std::floor(std::max(0, yPOS - this->status_bar->ui_view->height) /
                                      float(view->get_font()->get_row_advance())) +
                           this->view->get_cursor()->line;
        if (meta_data.line_begins.size() > row_clicked) {
            auto bufIdx = meta_data.line_begins[int(row_clicked)];
            view->get_text_buffer()->step_cursor_to(bufIdx);
        } else {
            auto bufIdx = meta_data.line_begins.back();
            view->get_text_buffer()->step_cursor_to(bufIdx);
        }
    } else {
        util::println("({}, {}) was clicked. Window dimInfo: {}", x, yPOS, dimInfo.debug_str());
        PANIC("WHOA! We should NOT end up here. This function is only called when we have verified that x & y _is_ "
              "inside this region");
    }
}
void EditorWindow::set_view_colors(Color bg, Color fg) {
    view->fg_color = fg;
    view->bg_color = bg;
}
void EditorWindow::set_font(SimpleFont *pFont) {
    view->font = pFont;
    view->cursor->setup_dimensions(8, pFont->max_glyph_height + 4);
}


}// namespace ui