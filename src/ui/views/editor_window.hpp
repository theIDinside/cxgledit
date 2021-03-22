//
// Created by 46769 on 2021-01-08.
//

#pragma once
// Sys headers
#include <optional>
// Local
#include "modal.hpp"
#include <app.hpp>
#include <cfg/configuration.hpp>
#include <cfg/types/cursor_options.hpp>
#include <core/math/vector.hpp>
#include <ui/core/layout.hpp>
#include <ui/render/font.hpp>

class Bookmark;
class TextData;
using RGBColor = Vec3f;

namespace ui {
// Forward declarations
struct View;
struct StatusBar;

struct EditorWindow {
    View *view = nullptr;
    StatusBar *status_bar = nullptr;
    int ui_layout_id;
    ui::core::DimInfo dimInfo;
    bool active = false;

    void draw(bool force_redraw = false);
    ~EditorWindow();
    TextData *get_text_buffer() const;
    // static EditorWindow *create(std::optional<TextData *> textData, glm::mat4 projection, int layout_id, core::DimInfo dimInfo);
    static EditorWindow *create(std::optional<TextData *> textData, Matrix projection, int layout_id,
                                core::DimInfo dimInfo);

    void update_layout(core::DimInfo dim_info);
    void set_projections(Matrix matrix);


    void handle_click(int xPos, int yPos);
    void set_view_colors(RGBColor bg, RGBColor fg, RGBColor whenActiveColor);
    void set_font(SimpleFont *pFont);

    void set_caret_style(Configuration::Cursor style);
    FileContext file_context() const;
    const std::vector<Bookmark>& get_bookmarks() const;
    void set_bookmark();
    void remove_bookmark(int index);
    void set_configuration(const Configuration& configuration);

};
}// namespace ui