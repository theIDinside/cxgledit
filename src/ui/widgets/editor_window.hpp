//
// Created by cx on 2021-03-25.
//

#pragma once
// We don't need to include this.. It's included via status_bar and text_view
// #include "widget.hpp"

#include "status_bar.hpp"
#include "text_view.hpp"
#include <cfg/configuration.hpp>

#include <memory>

namespace cx::widget {

class EditorWindow : public Widget {
public:
    EditorWindow(int buffer_id, gui_id id, Widget* parent);
    ~EditorWindow() override;
    static EditorWindow* create(Widget* parent, std::optional<TextData *> textData, Matrix projection);
    static EditorWindow* create_with_size(Widget* parent, std::optional<TextData *> textData, Matrix projection, Vec2i16 size);

    void draw() override;
    void handle_mouse_click(Vec2i16 mousePos) override;
    void set_view_colors(RGBColor bg, RGBColor whenActiveColor);
    void set_font(SimpleFont *pFont);
    void set_caret_style(Configuration::Cursor style);

    void set_size(Vec2i16 size) override;

    [[nodiscard]] FileContext file_context() const;
    [[nodiscard]] const std::vector<Bookmark>& get_bookmarks() const;
    void set_bookmark();
    void remove_bookmark(int index);
    void set_configuration(const Configuration& configuration);
private:
    int m_buffer_id;
    Boxed<cx::widget::StatusBar> m_status_bar;
    Boxed<cx::widget::TextView> m_text_view;
    bool active = false;

};
}