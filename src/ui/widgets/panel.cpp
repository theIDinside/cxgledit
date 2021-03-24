//
// Created by 46769 on 2021-01-13.
//

#include "panel.hpp"
#include <utils/utils.hpp>

cx::widget::Panel::~Panel() {

}

void cx::widget::Panel::draw() {
    util::println("Panel drawing not implemented");
}
cx::widget::Panel::Panel(cx::widget::gui_id id, cx::widget::Widget* parent) : Widget(id, parent) {

}
