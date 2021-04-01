//
// Created by cx on 2021-03-22.
//

#pragma once
#include <vector>


namespace cx::widget {
class Widget;
struct gui_id;

class Layout {
public:
    virtual ~Layout() = default;
    virtual void arrange_widget(Widget* w) = 0;
    virtual void set_container(Widget* w) { widget_container = w; }
protected:
    Widget* widget_container;
};

}// namespace cx::widget
