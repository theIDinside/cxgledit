//
// Created by cx on 2021-03-26.
//

#pragma once
#include "layout.hpp"

namespace cx::widget {
class SingleFill : public Layout {
public:
    SingleFill();
    ~SingleFill() override = default;
    /// SingleFill layout, takes _one_ of Widget w's children, and lays it out, fully, to the size of w
    void arrange_widget(Widget* w) override;
    void set_container(Widget* w) override;
};
}// namespace cx::widget
