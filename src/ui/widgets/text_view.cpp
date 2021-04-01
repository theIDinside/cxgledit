//
// Created by cx on 2021-03-25.
//

#include "text_view.hpp"
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>
#include <ui/managers/widget_manager.hpp>
#include <z3++.h>

namespace cx::widget {
TextView::~TextView() {
    delete m_active_cursor;
    m_active_cursor = nullptr;
    for (auto p : m_cursors) { delete p; }
}
void TextView::draw() {}

TextView::TextView(gui_id id, Widget* parent, Matrix mvp)
    : Widget(id, parent), m_lines_displayable{0}, m_buf_handle{nullptr}, m_active_cursor{nullptr}, m_cursors{},
      p_font_renderer{nullptr}, p_shader{nullptr}, m_mvp(mvp) {}

void TextView::set_shader(Shader* shader) { p_shader = shader; }
void TextView::set_font_renderer(SimpleFont* font) { p_font_renderer = font; }

TextView* TextView::create_view(gui_id id, Widget* parent, Matrix mvp, std::optional<TextData*> maybe_text_buffer) {
    auto textView = new TextView{id, parent, mvp};
    textView->set_font_renderer(FontLibrary::get_default_font());
    textView->set_shader(ShaderLibrary::get_text_shader());
    textView->view_buffer(maybe_text_buffer.value_or(nullptr));
}

void TextView::view_buffer(TextData* data) {
    if (data != nullptr) {
        m_buf_handle = data;
        if (m_active_cursor == nullptr) {
            m_buf_handle = data;
            auto vc = TextCursor::create(this);
            vc->set_mvp(&m_mvp);
            m_active_cursor = vc;
        }
    }
}
void TextView::set_size(Vec2i16 size) {
    const auto [w, h] = size;
    const auto diff_height = h != m_size.y;
    Widget::set_size(size);
    if (diff_height) { calculate_lines_displayable(); }
    util::println("lines displayable updated to: {}", m_lines_displayable);
}

void TextView::calculate_lines_displayable() {
    m_lines_displayable =
            int_ceil(float(m_size.y) / float(p_font_renderer->get_pixel_row_advance())) - LINES_DISPLAYABLE_DIFF;
}
TextData* TextView::get_text_buffer() const { return m_buf_handle; }

void TextView::set_draw_colors(RGBColor background, RGBColor whenActive) {
    m_bg_color = background;
    m_active_bg_color = whenActive;
}

Boxed<TextView> TextView::boxed_create_view(gui_id id, Widget* parent, Matrix mvp,
                                            std::optional<TextData*> maybe_text_buffer) {
    return Boxed<TextView>{TextView::create_view(id, parent, mvp, maybe_text_buffer)};
}
TextCursor* TextView::get_cursor() {
    return this->m_active_cursor;
}
SimpleFont* TextView::get_font() { return p_font_renderer; }

void TextView::scroll_to(int line) {
    int linesInBuffer = AS(get_text_buffer()->meta_data.line_begins.size(), int);
    int maxScrollableTopLine = std::max(0, linesInBuffer - m_lines_displayable + (m_lines_displayable / 2));
    if (line < maxScrollableTopLine) {
        m_active_cursor->m_view_line_anchor = std::max(line, 0);
    } else {
        m_active_cursor->m_view_line_anchor = maxScrollableTopLine;
    }
}

TextCursor::~TextCursor() = default;
void TextCursor::draw() {}

TextCursor::TextCursor(gui_id id, Widget* parent)
    : Widget(id, parent), m_view_line_anchor{0}, p_view{nullptr}, p_shader{nullptr}, p_mvp{nullptr},
      m_cursor_data{nullptr}, m_line_shade_data{nullptr}, m_buf_index{0} {}

void TextCursor::set_anchor(Vec2i16 pos) {
    m_pos = pos;
    auto screen_pos = calculate_absolute_position();
    const auto [x_, y_] = screen_pos;
    const float x = x_;
    const float y = y_;

    const auto [w, h] = this->m_size;

    auto& c_data = m_cursor_data->vbo->data;
    auto& l_data = m_line_shade_data->vbo->data;
    const auto view_width = static_cast<float>(p_parent->width());

    m_cursor_data->vbo->pristine = false;
    m_line_shade_data->vbo->pristine = false;

    c_data.clear();
    c_data.emplace_back(Vertex{x, y + static_cast<float>(h)});
    c_data.emplace_back(Vertex{x, y});
    c_data.emplace_back(Vertex{x + static_cast<float>(w), y});
    c_data.emplace_back(Vertex{x, y + static_cast<float>(h)});
    c_data.emplace_back(Vertex{x + static_cast<float>(w), y});
    c_data.emplace_back(Vertex{x + static_cast<float>(w), y + static_cast<float>(h)});

    auto vx = AS(screen_pos.x, float);
    l_data.clear();
    l_data.emplace_back(Vertex{vx, y + static_cast<float>(h)});
    l_data.emplace_back(Vertex{vx, y});
    l_data.emplace_back(Vertex{vx + view_width, y});
    l_data.emplace_back(Vertex{vx, y + static_cast<float>(h)});
    l_data.emplace_back(Vertex{vx + view_width, y});
    l_data.emplace_back(Vertex{vx + view_width, y + static_cast<float>(h)});
}

TextCursor* TextCursor::create(TextView* view) {
    auto buf_curs = view->get_text_buffer()->get_cursor();
    auto cursor_vao = CursorVAO::make(GL_ARRAY_BUFFER, 6 * 1024);
    auto vc = new TextCursor{WidgetManager::get_instance().request_gui_id_for_child(view), view};

    auto shader = ShaderLibrary::get_instance().get_shader("cursor");
    auto font = FontLibrary::get_default_font();
    auto line_shade_vao = CursorVAO::make(GL_ARRAY_BUFFER, 6 * 1024);

    shader->setup();
    shader->setup_fillcolor_ids();

    vc->m_view_line_anchor = buf_curs.line;
    vc->m_buf_index = buf_curs.pos;
    vc->m_cursor_data = std::move(cursor_vao);
    vc->m_line_shade_data = std::move(line_shade_vao);
    vc->p_view = view;
    vc->p_shader = shader;
    vc->set_size({4, static_cast<short>(font->max_glyph_height + 4)});
    return vc;
}
void TextCursor::set_mvp(Matrix* mvp) { p_mvp = mvp; }

void TextCursor::set_line_rect(float x1, float x2, float y1) {
    const auto w = x2 - x1;
    const auto h = static_cast<float>(height());
    const auto x = x1;
    const auto y = y1;
    auto& data = m_cursor_data->vbo->data;
    m_cursor_data->vbo->pristine = false;
    data.clear();
    data.emplace_back(Vertex{x, y + h});
    data.emplace_back(Vertex{x, y});
    data.emplace_back(Vertex{x + w, y});
    data.emplace_back(Vertex{x, y + h});
    data.emplace_back(Vertex{x + w, y});
    data.emplace_back(Vertex{x + w, y + h});
}
void TextCursor::set_color(RGBAColor rgbaColor) { m_caret_color = rgbaColor; }

}// namespace cx::widget
