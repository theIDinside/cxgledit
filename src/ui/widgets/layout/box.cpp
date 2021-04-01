//
// Created by cx on 2021-03-22.
//

#include "box.hpp"
#include "../widget.hpp"
#include <numeric>

void cx::widget::BoxLayout::arrange_horizontally(Widget* parent) const {
    auto children = parent->children();
    assert(!children.empty());
    auto remaining_width = parent->width() - (m_spacing * static_cast<i16>(children.size() - 1)) - (m_margin * 2);
    const auto fixedWidthInfo =
            std::accumulate(children.cbegin(), children.cend(), std::pair<i16, i16>{0, 0}, [](auto acc, Widget* child) {
                auto& [childrenWithFS, totalFS] = acc;
                totalFS += child->fixed_width().value_or(i16{0});
                if (child->fixed_width().has_value()) childrenWithFS++;
                return acc;
            });
    const auto [fixed_width_widgets, total_requested_fixed_width] = fixedWidthInfo;

    const auto widgets_to_resize = [](const auto children, const auto fixedWidgetCount) {
      auto hiddenCount = 0;
      for(const auto& c : children) {
          if(!c->is_visible()) hiddenCount++;
      }
      return static_cast<i16>(children.size()) - fixedWidgetCount - hiddenCount;
    }(children, fixed_width_widgets);

    remaining_width -= total_requested_fixed_width;
    const auto perElementWidth = remaining_width / widgets_to_resize;
    auto remaining_fixed_width = total_requested_fixed_width;
    auto xOffset = m_margin;
    auto yOffset = m_margin;

    const auto elements_height = parent->height() - m_margin * 2;

    for (auto c : children) {
        if(c->is_visible()) {
            if (auto fw = c->fixed_width(); fw) {
                c->set_size({*fw, c->fixed_height().value_or(elements_height)});
                c->set_anchor({xOffset, yOffset});
                xOffset += *fw + m_spacing;
                remaining_fixed_width -= *fw;
            } else {
                c->set_size({static_cast<i16>(perElementWidth), c->fixed_height().value_or(elements_height)});
                c->set_anchor({xOffset, yOffset});
                xOffset += perElementWidth + m_spacing;
                remaining_width -= perElementWidth;
            }
        }
    }
    assert(remaining_width < widgets_to_resize);
}

cx::widget::BoxLayout::BoxLayout(cx::widget::LayoutAxis layoutAxis, i16 margin, i16 spacing)
    : m_layout_type(layoutAxis), m_margin(margin), m_spacing(spacing) {}

void cx::widget::BoxLayout::arrange_widget(cx::widget::Widget* w) {
    switch (m_layout_type) {
        case LayoutAxis::Horizontal:
            arrange_horizontally(w);
            break;
        case LayoutAxis::Vertical:
            arrange_vertically(w);
            break;
    }
}

void cx::widget::BoxLayout::arrange_vertically(cx::widget::Widget* parent) const {
    auto children = parent->children();
    assert(!children.empty());
    auto remainingHeight = parent->height() - (m_spacing * static_cast<i16>(children.size() - 1)) - (m_margin * 2);
    const auto fixedHeightInfo =
            std::accumulate(children.cbegin(), children.cend(), std::pair<i16, i16>{0, 0}, [](auto acc, Widget* child) {
              auto& [childrenWithFS, totalFS] = acc;
              totalFS += child->fixed_height().value_or(i16{0});
              if (child->fixed_height().has_value()) childrenWithFS++;
              return acc;
            });
    const auto [fixed_height_widgets, total_requested_fixed_height] = fixedHeightInfo;

    const auto widgets_to_resize = [](const auto children, const auto fixedWidgetCount) {
      auto hiddenCount = 0;
      for(const auto& c : children) {
          if(!c->is_visible()) hiddenCount++;
      }
      return static_cast<i16>(children.size()) - fixedWidgetCount - hiddenCount;
    }(children, fixed_height_widgets);

    remainingHeight -= total_requested_fixed_height;
    const auto perElementHeight = remainingHeight / widgets_to_resize;
    auto remaining_fixed_height = total_requested_fixed_height;
    auto xOffset = m_margin;
    auto yOffset = m_margin;

    const auto elements_width = parent->width() - m_margin * 2;

    for (auto c : children) {
        if(c->is_visible()) {
            if (auto fw = c->fixed_height(); fw) {
                c->set_size({ c->fixed_width().value_or(elements_width), *fw});
                c->set_anchor({xOffset, yOffset});
                yOffset += *fw + m_spacing;
                remaining_fixed_height -= *fw;
            } else {
                c->set_size({c->fixed_width().value_or(elements_width), static_cast<i16>(perElementHeight)});
                c->set_anchor({xOffset, yOffset});
                yOffset += perElementHeight + m_spacing;
                remainingHeight -= perElementHeight;
            }
        }
    }
    assert(remainingHeight < widgets_to_resize);
}
