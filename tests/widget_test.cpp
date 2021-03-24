//
// Created by cx on 2021-03-24.
//

#include <ui/widgets/layout/box.hpp>
#include <ui/widgets/panel.hpp>

using Panel = cx::widget::Panel;
using gui_id = cx::widget::gui_id;
using BoxLayout = cx::widget::BoxLayout;
using BoxType = cx::widget::LayoutAxis;

int main() {
    const auto margin = 2;
    const auto spacing = 5;
    const auto main_panel_height = 400;
    const auto main_panel_width = 400;

    auto p = new Panel{gui_id{1}, nullptr};
    auto pA = new Panel{{2}, nullptr};
    auto pB = new Panel{{3}, nullptr};
    auto assertions_passed = 0;

    p->set_size({main_panel_width, main_panel_height});
    p->set_layout(new BoxLayout{BoxType::Horizontal, margin, spacing});
    p->add_child(pA);
    p->add_child(pB);
    p->layout_widget();
    {
        auto assert_element_width = (main_panel_width - (2 * margin + spacing)) / 2;
        auto assert_element_height = main_panel_height - 2 * margin;


        for (auto c : p->children()) {
            assert(c->width() == assert_element_width);
            assert(c->height() == assert_element_height);
            auto pos = c->get_relative_position();
            util::println("Pos: ({}, {}) - Size: {}x{}", pos.x, pos.y, c->width(), c->height());
            assertions_passed++;
        }
        util::println("\n");
    }

    auto pC = new Panel{{4}, nullptr};
    pC->resize(200, {});

    p->add_child(pC);
    p->layout_widget();

    {
        auto assert_element_width = (main_panel_width - 200 - (2 * margin + 2 * spacing)) / 2;
        auto assert_element_height = main_panel_height - 2 * margin;

        for (auto c : p->children()) {
            if(!c->is_fixed_size()) {
                assert(c->width() == assert_element_width);
                assert(c->height() == assert_element_height);
                assertions_passed++;
            }
            auto pos = c->get_relative_position();
            util::println("Pos: ({}, {}) - Size: {}x{}", pos.x, pos.y, c->width(), c->height());
        }
    }
    util::println("\n");

    auto pD = new Panel{{4}, nullptr};
    p->add_child(pD);
    p->layout_widget();

    {
        auto assert_element_width = (main_panel_width - 200 - (2 * margin + 3 * spacing)) / 3;
        auto assert_element_height = main_panel_height - 2 * margin;

        for (auto c : p->children()) {
            if(!c->is_fixed_size()) {
                assert(c->width() == assert_element_width);
                assert(c->height() == assert_element_height);
                assertions_passed++;
            } else {
                assert(c->width() == 200);
                assertions_passed++;
            }
            auto pos = c->get_relative_position();
            util::println("Pos: ({}, {}) - Size: {}x{}", pos.x, pos.y, c->width(), c->height());
        }
    }
    util::println("\nAfter resize:");
    pB->resize(100, {});
    for (auto c : p->children()) {
        auto pos = c->get_relative_position();
        util::println("Pos: ({}, {}) - Size: {}x{}", pos.x, pos.y, c->width(), c->height());
    }



    util::println("Assertions passed: {}", assertions_passed);
}