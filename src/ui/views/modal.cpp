//
// Created by 46769 on 2021-01-11.
//

#include "modal.hpp"
#include <core/buffer/std_string_buffer.hpp>
#include <ranges>
#include <algorithm>
#include <ui/managers/font_library.hpp>
#include <ui/views/view.hpp>

#undef max
#undef min

ui::ModalPopup *ui::ModalPopup::create(Matrix projection) {
    auto input_buffer = StdStringBuffer::make_non_owning();
    input_buffer->info = BufferTypeInfo::Modal;
    auto view = View::create(input_buffer, "modal_popup", 0, 0, 0, 0, ViewType::Modal);
    view->bg_color = Vec3f{0.4f, 0.3f, 0.3f};
    auto m = new ModalPopup{};
    view->set_projection(projection);
    m->view = view;
    m->type = ModalContentsType::ActionList;
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
    for (const auto &i : dialogData) {
        drawables.push_back(TextDrawable{x, first - (lineNo * view->get_font()->get_pixel_row_advance()), i.displayable, {}});
        lineNo++;
    }
    view->cursor->width = w;
    view->draw_modal_view(selected, drawables);
}

void ui::ModalPopup::cycle_choice(ui::Scroll scroll) {
    if (dialogData.empty()) { return; }
    switch (scroll) {
        case ui::Scroll::Up: {
            if (selected - 1 < 0) {
                selected = (int) dialogData.size() - 1;
            } else {
                selected--;
            }
        } break;
        case ui::Scroll::Down: {
            if (selected + 1 >= (int) dialogData.size()) {
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
    choices = (int) item.size();
    selected = 0;

    const auto f = view->get_font();

    auto dialogWidth = 0;
    for(const auto& dd : dialogData) {
        dialogWidth = std::max(f->calculate_text_width(dd.displayable), dialogWidth);
    }

    const auto dialogHeight = dialogData.size() * (view->get_font()->get_pixel_row_advance());

    dimInfo.w       = (int) dialogWidth;
    dimInfo.h       = (int) dialogHeight;
    view->width     = (int) dialogWidth;
    view->height    = (int) dialogHeight;
}

void ui::ModalPopup::anchor_to(int x, int y) {
    dimInfo.x = x;
    dimInfo.y = y;
    view->x = x;
    view->y = y;
}

std::optional<ui::PopupItem> ui::ModalPopup::get_choice() {
    if (not dialogData.empty()) {
        return dialogData[selected];
    } else {
        return {};
    }
}

void ui::ModalPopup::maybe_delete_selected() {
    if (not dialogData.empty()) {
        dialogData.erase(dialogData.begin() + selected);
        auto updateIndex = 0;
        for (auto &item : dialogData) {
            item.item_index = updateIndex;
            updateIndex++;
        }
        if (dialogData.empty()) {
            selected = 0;
        } else {
            selected = std::min(selected, static_cast<int>(dialogData.size() - 1));
        }
    }
}

std::vector<ui::PopupItem> ui::PopupItem::make_action_list_from_context(const FileContext &context) {
    std::vector<ui::PopupItem> result;
    auto index = 0;
    switch (context.type) {
        case CPPHeader: {
            result.push_back(ui::PopupItem{index++, "Goto Implementation file", ui::PopupActionType::AppCommand,
                                           Commands::GotoSource});
        } break;
        case CPPSource: {
            result.push_back(
                    ui::PopupItem{index++, "Goto header", ui::PopupActionType::AppCommand, Commands::GotoHeader});
        } break;
        case Config: {
            result.push_back(ui::PopupItem{index++, "Load this configuration", ui::PopupActionType::AppCommand,
                                           Commands::ReloadConfiguration});
        } break;
        case Unhandled: {

        } break;
        default: {
            // This is really the _only_ useful usecase for default. Because we want to know if did it all
            PANIC("All cases for PopupItem based on FileContext not yet implemented");
        }
    }
    result.push_back(ui::PopupItem{index++, "Goto", ui::PopupActionType::AppCommand, Commands::GotoLine});
    result.push_back(ui::PopupItem{index++, "Open file", ui::PopupActionType::AppCommand, Commands::OpenFile});
    result.push_back(ui::PopupItem{index++, "Find in file", ui::PopupActionType::AppCommand, Commands::Search});
    result.push_back(ui::PopupItem{index++, "Save file", ui::PopupActionType::AppCommand, Commands::WriteFile});
    result.push_back(ui::PopupItem{index++, "Save all", ui::PopupActionType::AppCommand, Commands::WriteAllFiles});
    return result;
}
