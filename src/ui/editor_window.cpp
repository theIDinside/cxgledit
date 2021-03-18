//
// Created by 46769 on 2021-01-08.
//

#include "editor_window.hpp"
#include <core/buffer/data_manager.hpp>
#include <ui/status_bar.hpp>
#include <ui/view.hpp>

/// The optional<TD*> is *ONLY* to express the purpose that this value can be nil
/// just having a pointer here, could *very* well lead me to believe at some point that null can't be passed here
/// when it can.

#undef min
#undef max

namespace ui {

EditorWindow *EditorWindow::create(std::optional<TextData *> textData, Matrix projection, int layout_id,
                                   core::DimInfo dimInfo) {
    auto &[x, y, width, height] = dimInfo;
    auto sb_height = FontLibrary::get_default_font()->get_pixel_row_advance() + 2;
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

    ew->set_projections(projection);
    ew->status_bar->set_buffer_cursor(&ew->view->get_text_buffer()->cursor);
    return ew;
}

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

    const auto &[x, y, width, height] = dim_info;
    const auto sb_height = FontLibrary::get_default_font()->get_pixel_row_advance() + 2;
    // we have to make room for status bar & command view spanning across entire bottom, both of which are equal in height
    const auto text_editor_height = height - (sb_height * 2);
    const auto text_editor_y_pos = height - sb_height;
    view->set_dimensions(width, text_editor_height);
    view->anchor_at(x, text_editor_y_pos);
    status_bar->ui_view->set_dimensions(width, sb_height);
    status_bar->ui_view->anchor_at(x, height);
    this->dimInfo = dim_info;
}

void EditorWindow::handle_click(int xPos, int yPos) {
    if (dimInfo.is_inside(xPos, yPos)) {
        auto &meta_data = view->get_text_buffer()->meta_data;

        const auto row_clicked = std::floor(std::max(0, yPos - status_bar->ui_view->height) /
                                      float(view->get_font()->get_pixel_row_advance())) +
                           view->get_cursor()->views_top_line;
        if (meta_data.line_begins.size() > row_clicked) {
            const auto bufIdx = meta_data.line_begins[int(row_clicked)];
            view->get_text_buffer()->step_cursor_to(bufIdx);
        } else {
            const auto bufIdx = meta_data.line_begins.back();
            view->get_text_buffer()->step_cursor_to(bufIdx);
        }
    } else {
        util::println("({}, {}) was clicked. Window dimInfo: {}", xPos, yPos, dimInfo.debug_str());
        PANIC("WHOA! We should NOT end up here. This function is only called when we have verified that x & y _is_ "
              "inside this region");
    }
}

void EditorWindow::set_view_colors(RGBColor bg, RGBColor fg, RGBColor whenActiveColor) {
    view->fg_color = fg;
    view->bg_color = bg;
    view->when_active_bg_color = whenActiveColor;
}

void EditorWindow::set_font(SimpleFont *pFont) {
    view->font = pFont;
    view->cursor->setup_dimensions(view->cursor->width, pFont->max_glyph_height + 4);
}

void EditorWindow::set_caret_style(Configuration::Cursor style) {
    view->cursor->caret_color = style.color;
    std::visit(
            [this](auto &&style) {
                using T = std::decay_t<decltype(style)>;
                if constexpr (std::is_same_v<T, CaretStyleLine>) {
                    fmt::print("Line width to set: {}", style.width);
                    this->view->cursor->width = style.width;
                } else if constexpr (std::is_same_v<T, CaretStyleBlock>) {
                    this->view->cursor->width = this->view->font->max_glyph_width - 2;
                } else {
                    static_assert(always_false_v<T>, "non exhaustive visitor");
                }
            },
            style.cursor_style);
}

FileContext EditorWindow::file_context() const { return get_text_buffer()->file_context(); }

const std::vector<Bookmark> &EditorWindow::get_bookmarks() const { return get_text_buffer()->meta_data.bookmarks; }

void EditorWindow::set_bookmark() {
    auto& buf = this->view->data;
    buf->set_bookmark();
}

void EditorWindow::remove_bookmark(int index) {
    get_text_buffer()->meta_data.bookmarks.erase(get_text_buffer()->meta_data.bookmarks.begin() + index);
}
void EditorWindow::set_configuration(const Configuration &config) {
    set_caret_style(config.cursor);
    set_view_colors(config.views.bg_color, config.views.fg_color, config.views.active_bg);
}

void EditorWindow::set_projections(Matrix mvp) {
    view->set_projection(mvp);
    status_bar->ui_view->set_projection(mvp);
}

}// namespace ui