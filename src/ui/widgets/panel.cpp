//
// Created by 46769 on 2021-01-13.
//

#include "panel.hpp"
#include <utils/utils.hpp>

namespace cx::widget {
Panel::~Panel() {}

void Panel::draw() { util::println("Panel drawing not implemented"); }
Panel::Panel(cx::widget::gui_id id, cx::widget::Widget* parent) : Widget(id, parent) {}

}// namespace cx::widget
