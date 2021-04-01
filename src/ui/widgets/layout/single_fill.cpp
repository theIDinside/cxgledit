//
// Created by cx on 2021-03-26.
//

#include "single_fill.hpp"
#include "../widget.hpp"

#include <cassert>



void cx::widget::SingleFill::arrange_widget(cx::widget::Widget* w) {
    assert(w->children().size() == 1);
    auto single = w->children().back();
    single->set_size(w->size());
    single->set_anchor({0, 0});
}
void cx::widget::SingleFill::set_container(cx::widget::Widget* w) { Layout::set_container(w); }

cx::widget::SingleFill::SingleFill() : Layout() {}
