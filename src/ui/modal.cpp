//
// Created by 46769 on 2021-01-11.
//

#include "modal.hpp"
#include <ui/managers/font_library.hpp>
#include <ui/view.hpp>

ui::Modal *ui::Modal::create(glm::mat4 projection) {
    auto input_buffer = StdStringBuffer::make_non_owning();
    input_buffer->info = BufferTypeInfo::Modal;
    auto view = View::create(input_buffer, "modal_popup", 0, 0, 0, 0, ViewType::Modal);
    view->bg_color = Vec3f{0.4f, 0.3f, 0.3f};
    auto m = new Modal{};
    view->set_projection(projection);
    m->view = view;
    m->type = ModalContentsType::List;
    m->dimInfo = {0, 0, 0, 0};
    m->data = input_buffer;
    return m;
}

void ui::Modal::show(ui::core::DimInfo dimension) {
    auto &[x, y, w, h] = dimension;
    this->dimInfo = dimension;
    view->set_dimensions(w, h);
    view->anchor_at(x, y);
}

void ui::Modal::draw() {
    // util::println("Draw modal");
    auto heightOfModalPixels = (FontLibrary::get_default_font()->get_row_advance() + 2) * lines;
    data->rebuild_metadata();
    auto v = data->to_string_view();
    auto widthOfModalPixels = view->get_font()->calculate_text_width(v);
    auto [x, y, w, h] = this->dimInfo;
    std::vector<TextDrawable> drawables{};
    auto max_width = view->get_font()->calculate_text_width(v);
    auto lineNo = 0;
    std::vector<int> newLinePos{};
    for (auto pos = 0; pos < v.size(); pos++) {
        if (v[pos] == '\n') { newLinePos.push_back(pos); }
    }

    auto b = 0;
    auto first = y - view->get_font()->max_glyph_height;
    for (auto nl : newLinePos) {
        auto strview = v.substr(b, nl - b);
        drawables.push_back(TextDrawable{x, first - (lineNo * view->get_font()->get_row_advance()), strview, {}});
        b = nl + 1;
        lineNo++;
    }
    auto rest = v.substr(newLinePos.back() + 1);
    drawables.push_back(TextDrawable{x, first - (lineNo * view->get_font()->get_row_advance()), rest, {}});
    view->cursor->width = widthOfModalPixels;
    view->set_dimensions(widthOfModalPixels, dimInfo.h);
    view->anchor_at(dimInfo.x, dimInfo.y);
    view->draw_modal_view(selected, drawables);
}

void ui::Modal::set_data(const std::string &text) {
    std::string_view tmp{text};
    auto max_item_text_length = 0;
    auto pos = tmp.find('\n');
    lines = 1;
    // Data consists of 1 line only
    if (pos == std::string_view::npos) {
        character_width = text.size();
        data->clear();
        data->insert_str(text);
    } else {
        while (pos != std::string_view::npos) {
            lines++;
            max_item_text_length = std::max(max_item_text_length, AS(pos + 1, int));
            tmp.remove_prefix(pos + 1);
            pos = tmp.find('\n');
        }
        character_width = max_item_text_length;
        data->clear();
        data->insert_str(text);
    }
    choices = lines;
    selected = 0;
}

void ui::Modal::cycle_choice(ui::Scroll scroll) {
    switch (scroll) {
        case Scroll::Up: {
            if(selected - 1 < 0) {
                selected = choices - 1;
            } else {
                selected--;
            }
        } break;
        case Scroll::Down: {
            if(selected + 1 >= choices) {
                selected = 0;
            } else {
                selected++;
            }
        } break;
    }
}
