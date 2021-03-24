//
// Created by cx on 2021-03-24.
//

#include <ui/widgets/layout/box.hpp>
#include <ui/widgets/panel.hpp>

using Panel = cx::widget::Panel;
using gui_id = cx::widget::gui_id;
using BoxLayout = cx::widget::BoxLayout;
using BoxType = cx::widget::LayoutAxis;

static auto assertions = 0;

#define _NAME_MAKER(a, b) a##b
#define ASSERT_NAME(name, line) _NAME_MAKER(name, line)

#define ADD_ASSERTION(assertion_expression, err_msg)                                                                   \
    {                                                                                                                  \
        assertions++;                                                                                                  \
        if (!(assertion_expression)) {                                                                                 \
            util::println("Assertion {} failed ({}:{})- {}", assertions, __FILE_NAME__, __LINE__, err_msg);            \
            std::abort();                                                                                              \
        }                                                                                                              \
    }

int main() {
    const auto margin = 2;
    const auto spacing = 5;
    const auto main_panel_height = 400;
    const auto main_panel_width = 400;

    auto p = new Panel{gui_id{1}, nullptr};
    auto pA = new Panel{{2}, nullptr};

    auto assertions_passed = 0;

    p->set_size({main_panel_width, main_panel_height});
    p->set_anchor(
            {200, 0});// anchored at (200, 0), thus any of it's childrens' absolute positions will be c_p + (200, 0)
    p->set_layout(new BoxLayout{BoxType::Horizontal, margin, spacing});
    p->add_child(pA);
    auto pB = new Panel{{3}, p};
    // p->add_child(pB);
    p->layout_widget();
    {
        auto assert_element_width = (main_panel_width - (2 * margin + spacing)) / 2;
        auto assert_element_height = main_panel_height - 2 * margin;

        for (auto c : p->children()) {
            ADD_ASSERTION(c->width() == assert_element_width,
                          fmt::format("child width {} != {}", c->width(), assert_element_width));
            ADD_ASSERTION(c->height() == assert_element_height,
                          fmt::format("child height {} != {}", c->height(), assert_element_height));
            auto pos = c->get_relative_position();
            auto abs_pos = c->calculate_absolute_position();
            util::println("#Wid({}): Rel Pos: ({}, {})/Abs pos: ({}, {}) - Size: {}x{}", c->get_id(), pos.x, pos.y,
                          abs_pos.x, abs_pos.y, c->width(), c->height());
            assertions_passed++;
            assertions_passed++;
        }
        util::println("\n");
    }

    auto pC = new Panel{{4}, nullptr};
    pC->resize(200, {});


    auto pCA = new Panel{{41}, nullptr};
    auto pCB = new Panel{{42}, nullptr};
    pC->add_child(pCA);
    pC->add_child(pCB);

    p->add_child(pC);
    pC->set_layout(new BoxLayout{BoxType::Horizontal, margin, 1});
    p->layout_widget();

    // pC->layout_widget();
    for(auto c : pC->children()) {
        auto pos = c->get_relative_position();
        auto abs_pos = c->calculate_absolute_position();
        util::println("#Wid({}): Rel Pos: ({}, {})/Abs pos: ({}, {}) - Size: {}x{}", c->get_id(), pos.x, pos.y,
                      abs_pos.x, abs_pos.y, c->width(), c->height());
    }

    util::println("Hiding pcB");
    pCB->hide();
    p->layout_widget();
    for (auto c : p->children()) {
        auto pos = c->get_relative_position();
        auto abs_pos = c->calculate_absolute_position();
        util::println("#Wid({}): Rel Pos: ({}, {})/Abs pos: ({}, {}) - Size: {}x{}", c->get_id(), pos.x, pos.y,
                      abs_pos.x, abs_pos.y, c->width(), c->height());
    }
    for(auto c : pC->children()) {
        auto pos = c->get_relative_position();
        auto abs_pos = c->calculate_absolute_position();
        util::println("#Wid({}): Rel Pos: ({}, {})/Abs pos: ({}, {}) - Size: {}x{}", c->get_id(), pos.x, pos.y,
                      abs_pos.x, abs_pos.y, c->width(), c->height());
    }
    util::println("");
    pCB->show();
    p->layout_widget();
    {
        auto assert_element_width = (main_panel_width - 200 - (2 * margin + 2 * spacing)) / 2;
        auto assert_element_height = main_panel_height - 2 * margin;

        for (auto c : p->children()) {
            if (!c->is_fixed_size()) {
                ADD_ASSERTION(c->width() == assert_element_width,
                              fmt::format("child width {} != {}", c->width(), assert_element_width));
                ADD_ASSERTION(c->height() == assert_element_height,
                              fmt::format("child height {} != {}", c->height(), assert_element_height));
                assertions_passed++;
                assertions_passed++;
            }
            auto pos = c->get_relative_position();
            auto abs_pos = c->calculate_absolute_position();
            util::println("#Wid({}): Rel Pos: ({}, {})/Abs pos: ({}, {}) - Size: {}x{}", c->get_id(), pos.x, pos.y,
                          abs_pos.x, abs_pos.y, c->width(), c->height());
        }
    }
    util::println("\n");

    auto pD = new Panel{{5}, nullptr};
    p->add_child(pD);
    p->layout_widget();

    {
        auto assert_element_width = (main_panel_width - 200 - (2 * margin + 3 * spacing)) / 3;
        auto assert_element_height = main_panel_height - 2 * margin;

        for (auto c : p->children()) {
            if (!c->is_fixed_size()) {
                ADD_ASSERTION(c->width() == assert_element_width,
                              fmt::format("child width {} != {}", c->width(), assert_element_width));
                ADD_ASSERTION(c->height() == assert_element_height,
                              fmt::format("child height {} != {}", c->height(), assert_element_height));
                assertions_passed++;
                assertions_passed++;
            } else {
                ADD_ASSERTION(c->width() == 200, fmt::format("fixed sized child width {} != {}", c->width(), 200));
                assertions_passed++;
            }
            auto pos = c->get_relative_position();
            auto abs_pos = c->calculate_absolute_position();
            util::println("#Wid({}): Rel Pos: ({}, {})/Abs pos: ({}, {}) - Size: {}x{}", c->get_id(), pos.x, pos.y,
                          abs_pos.x, abs_pos.y, c->width(), c->height());
        }
    }
    util::println("\nAfter resize:");
    pB->resize(100, {});
    for (auto c : p->children()) {
        auto pos = c->get_relative_position();
        auto abs_pos = c->calculate_absolute_position();
        util::println("#Wid({}): Rel Pos: ({}, {})/Abs pos: ({}, {}) - Size: {}x{}", c->get_id(), pos.x, pos.y,
                      abs_pos.x, abs_pos.y, c->width(), c->height());
    }

    ADD_ASSERTION(4 == p->find_widget({155, 3})->id,
                  fmt::format("Found guiid {} != 4", p->find_widget({155, 3}).value_or(gui_id{-10}).id));
    assertions_passed++;

    util::println("\nLooking for child widget 42");
    ADD_ASSERTION(42 == p->find_widget({265, 5})->id,
                  fmt::format("Found guiid {} != 4", p->find_widget({265, 5}).value_or(gui_id{-10}).id));

    util::println("Assertions passed: {}", assertions_passed);
}