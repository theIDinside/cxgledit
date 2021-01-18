//
// Created by 46769 on 2021-01-11.
//

#include "modal.hpp"
#include <ui/managers/font_library.hpp>
#include <ui/view.hpp>
#include <ranges>

/*
ui::ModalPopup *ui::ModalPopup::create(glm::mat4 projection) {
    auto input_buffer = StdStringBuffer::make_non_owning();
    input_buffer->info = BufferTypeInfo::Modal;
    auto view = View::create(input_buffer, "modal_popup", 0, 0, 0, 0, ViewType::Modal);
    view->bg_color = Vec3f{0.4f, 0.3f, 0.3f};
    auto m = new ModalPopup{};
    view->set_projection(projection);
    m->view = view;
    m->type = ModalContentsType::List;
    m->dimInfo = {0, 0, 0, 0};
    m->data = input_buffer;
    return m;
}
*/

ui::ModalPopup *ui::ModalPopup::create(Matrix projection) {
    auto input_buffer = StdStringBuffer::make_non_owning();
    input_buffer->info = BufferTypeInfo::Modal;
    auto view = View::create(input_buffer, "modal_popup", 0, 0, 0, 0, ViewType::Modal);
    view->bg_color = Vec3f{0.4f, 0.3f, 0.3f};
    auto m = new ModalPopup{};
    view->set_projection(projection);
    m->view = view;
    m->type = ModalContentsType::List;
    m->dimInfo = {0, 0, 0, 0};
    m->data = input_buffer;
    return m;
}

void ui::ModalPopup::show(ui::core::DimInfo dimension) {
    auto &[x, y, w, h] = dimension;
    this->dimInfo = dimension;
    view->set_dimensions(w, h);
    view->anchor_at(x, y);
}

void ui::ModalPopup::draw() {
    auto [x, y, w, h] = this->dimInfo;
    auto first = y - view->get_font()->max_glyph_height;
    std::vector<TextDrawable> drawables{};
    auto lineNo = 0;
    for(const auto& i : dialogData) {
        drawables.push_back(TextDrawable{x, first - (lineNo * view->get_font()->get_row_advance()), i.displayable, {}});
        lineNo++;
    }
    view->cursor->width = w;
    view->draw_modal_view(selected, drawables);
}

void ui::ModalPopup::cycle_choice(ui::Scroll scroll) {
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
void ui::ModalPopup::register_actions(std::vector<PopupItem> item) {
    dialogData.clear();
    std::ranges::copy(item, std::back_inserter(dialogData));
    choices = item.size();
    selected = 0;

    const auto f = view->get_font();
    auto tRng = std::views::all(dialogData) | std::views::transform([&](auto e) {
      auto result = f->calculate_text_width(e.displayable);
      return result;
    });
    auto dialogHeight = dialogData.size() * (FontLibrary::get_default_font()->get_row_advance() + 2);
    auto dialogWidth = std::ranges::max(tRng);
    this->dimInfo.w = dialogWidth;
    this->dimInfo.h = dialogHeight;
    view->width = dialogWidth;
    view->height = dialogHeight;

}

void ui::ModalPopup::anchor_to(int x, int y) {
    dimInfo.x = x;
    dimInfo.y = y;
    view->x = x;
    view->y = y;
}

ui::PopupItem ui::ModalPopup::get_choice() { return dialogData[selected]; }

