//
// Created by cx on 2021-03-25.
//

#pragma once
#include "widget.hpp"
#include <core/buffer/text_data.hpp>
#include <ui/render/font.hpp>
#include <ui/render/renderable_object_parameter.hpp>
#include <ui/render/shader.hpp>

// TODO: Implement something that handles this dymanically. Hard coded magic constants are icky.
constexpr auto LINES_DISPLAYABLE_DIFF = 2;

namespace cx::widget {

class TextView;

class TextCursor : public Widget {
    friend class TextView;

public:
    explicit TextCursor(gui_id, Widget* parent = nullptr);
    static TextCursor* create(TextView* view);
    ~TextCursor() override;
    void draw() override;
    void set_anchor(Vec2i16 pos) override;
    void set_mvp(Matrix* mvp);
    void set_line_rect(float x1, float x2, float y1);
    /**
     * Which line in the text buffer, that the top-most line in the TextView, is currently anchored to,
     * i.e. the line in the text buffer, which is the first line in the TextView
     */
    int m_view_line_anchor;
    void set_color(RGBAColor rgbaColor);
private:
    TextView* p_view;
    Shader* p_shader;
    Matrix* p_mvp;
    std::unique_ptr<CursorVAO> m_cursor_data;
    std::unique_ptr<CursorVAO> m_line_shade_data;
    RGBAColor m_caret_color = {1.0f, 0.0f, 0.2f, .4f};

    int m_buf_index;
};

class TextView : public Widget {
public:
    TextView(gui_id id, Widget* parent, Matrix mvp);
    ~TextView() override;
    void draw() override;
    void scroll_to(int line);
    SimpleFont* get_font();
    TextCursor* get_cursor();

    /// Setup & Initialize functions
    void view_buffer(TextData* data);
    void set_shader(Shader* shader);
    void set_font_renderer(SimpleFont* font);

    static TextView* create_view(gui_id id, Widget* parent, Matrix mvp, std::optional<TextData*> maybe_text_buffer);
    static Boxed<TextView> boxed_create_view(gui_id id, Widget* parent, Matrix mvp, std::optional<TextData*> maybe_text_buffer);
    void set_size(Vec2i16 size) override;
    TextData* get_text_buffer() const;

    void set_draw_colors(RGBColor background, RGBColor whenActive);

private:
    int m_lines_displayable{};
    std::unique_ptr<TextVertexArrayObject> vao{nullptr};// the graphical representation
    Vec3f m_bg_color{0.05f, 0.052f, 0.0742123f};
    Vec3f m_active_bg_color{0.05f, 0.052f, 0.0742123f};
    TextData* m_buf_handle;

    TextCursor* m_active_cursor;
    std::vector<TextCursor*> m_cursors;

    SimpleFont* p_font_renderer;
    Shader* p_shader;
    Matrix m_mvp{};
    inline void calculate_lines_displayable();
};
}// namespace cx::widget
