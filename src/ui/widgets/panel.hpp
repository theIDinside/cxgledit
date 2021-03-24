//
// Created by 46769 on 2021-01-13.
//

#pragma once
#include "widget.hpp"
namespace cx::widget {
class Panel : public Widget {
public:
    Panel(gui_id id, Widget* parent);
    ~Panel() override;
    void draw() override;
};
}// namespace cx::widget