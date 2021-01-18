//
// Created by 46769 on 2021-01-11.
//

#pragma once
#include "view.hpp"
#include <ui/core/layout.hpp>
#include <core/commands/command.hpp>

class TextData;

namespace ui {
class View;


enum ModalContentsType {
    List,
    Item
};

enum class PopupActionType {
    Insert,     // for when selecting an item in the popup list, to insert that text at the cursor
    AppCommand, // for when the users selects an item meant to run a command that the App class understands
};

struct PopupItem {
    std::string displayable;
    PopupActionType type;
    std::optional<Commands> command{};
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
    // static ModalPopup * create(glm::mat4 projection);
    static ModalPopup* create(Matrix projection);
    void show(core::DimInfo dimension);
    void anchor_to(int x, int y);
    void register_actions(std::vector<PopupItem> item);
    void draw();
    void cycle_choice(ui::Scroll scroll);
    PopupItem get_choice();
};

}// namespace ui
