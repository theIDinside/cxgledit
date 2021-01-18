//
// Created by 46769 on 2021-01-08.
//

#pragma once
#include <glm/glm.hpp>
#include <core/vec_3.hpp>
#include <optional>
#include <ui/core/layout.hpp>

class TextData;
using Color = Vec3f;


namespace ui {
    // Forward declarations
    class View;
    class StatusBar;

    struct EditorWindow {
        View *view;
        StatusBar *status_bar;
        int ui_layout_id;
        ui::core::DimInfo dimInfo;

        void draw(bool force_redraw = false);
        ~EditorWindow();

        [[nodiscard]] TextData *get_text_buffer() const;
        static EditorWindow *create(std::optional<TextData *> textData, glm::mat4 projection, int layout_id,
                                    core::DimInfo dimInfo);
        void update_layout(core::DimInfo dim_info);
        void set_projection(glm::mat4 projection) const;
        bool active = false;
        void handle_click(int x, int y);

        void set_view_colors(Color bg, Color fg);
    };
}