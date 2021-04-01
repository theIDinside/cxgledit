//
// Created by 46769 on 2021-01-11.
//

#pragma once
#include "popup_types.hpp"
#include "view_enums.hpp"
#include <core/buffer/file_context.hpp>
#include <core/commands/command_interpreter.hpp>
#include <core/commands/command_type_info.hpp>
#include <core/math/matrix.hpp>
#include <ui/core/layout.hpp>
#include <vector>

class TextData;

namespace ui {
struct View;

struct PopupItem {
    int item_index = 0;
    std::string displayable;
    PopupActionType type;
    std::optional<Commands> command{};
    static std::vector<PopupItem> make_action_list_from_context(const FileContext& context);
};

struct ModalPopup {
    View* view;
    ModalContentsType type;
    core::DimInfo dimInfo;
    TextData* data;
    std::vector<PopupItem> dialogData;

    int selected = 0;
    int choices = 0;
    int character_width = 0;
    int lines = 0;
    static ModalPopup* create(Matrix projection);
    void show(core::DimInfo dimension);
    void anchor_to(int x, int y);
    void register_actions(std::vector<PopupItem> item);
    void draw();
    void cycle_choice(Scroll scroll);
    std::optional<PopupItem> get_choice();
    void maybe_delete_selected();
};

}// namespace ui
