//
// Created by cx on 2021-03-22.
//

#pragma once
#include "layout.hpp"
#include <utils/utils.hpp>

namespace cx::widget {

    class BoundingRect;

    enum class LayoutAxis {
        Horizontal, Vertical
    };

    enum class ManageConstraints {
        FitAll, Relaxed
    };

    class BoxLayout : public Layout {
    public:
        BoxLayout(LayoutAxis layoutAxis, i16 margin, i16 spacing);
        void arrange_widget(Widget* w) override;

    private:
        /// Layout widgets along horizontal or vertical axis
        LayoutAxis m_layout_type;
        /// Margin from the edges of the widget -> this layout and the widgets laid out within
        i16 m_margin;
        /// Spacing _between_ widgets laid out by this layout
        i16 m_spacing;
        /// Whether or not the layout should cram all widgets to fit (FitAll), or let them run off of the edge
        /// and be clipped (Relaxed)
        ManageConstraints m_layout_constraints = ManageConstraints::FitAll;

        void arrange_horizontally(Widget* widget) const;
        void arrange_vertically(Widget* widget) const;
    };

}