//
// Created by 46769 on 2021-01-11.
//

#pragma once
#include <ui/core/layout.hpp>

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

    Modal* create();
    void set_position(int x, int y);
    void set_dimensions(int x, int y);

};

}// namespace ui
