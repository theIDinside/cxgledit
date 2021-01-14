//
// Created by 46769 on 2021-01-11.
//

#pragma once
#include "view.hpp"
#include <glm/glm.hpp>
#include <ui/core/layout.hpp>

class TextData;

namespace ui {
class View;


enum ModalContentsType {
    List,
    Item
};

struct Modal {
    View* view;
    ModalContentsType type;
    core::DimInfo dimInfo;
    TextData* data;
    int selected = 0;
    int choices = 0;
    int character_width = 0;
    int lines = 0;
    static Modal* create(glm::mat4 projection);
    void show(core::DimInfo dimension);
    void set_data(const std::string& text);
    void draw();
    void cycle_choice(ui::Scroll scroll);
};

}// namespace ui
