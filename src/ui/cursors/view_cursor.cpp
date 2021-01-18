//
// Created by 46769 on 2020-12-27.
//
#include "view_cursor.hpp"
#include "../view.hpp"

#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>

namespace ui {

// View Cursor

constexpr auto cursor_fill_color = Vec4f{1.0, 0.0, 0.2, .4};
constexpr auto line_shade_color = Vec4f{0.5,0.5,0.5,0.25};

std::unique_ptr<ViewCursor> ViewCursor::create_from(std::unique_ptr<View> &owning_view) {
    auto buf_curs = owning_view->get_text_buffer()->get_cursor();

    auto cursor_vao = CursorVAO::make(GL_ARRAY_BUFFER, 6 * 4096);
    auto line_shade_vao = CursorVAO::make(GL_ARRAY_BUFFER, 6 * 1024);

    auto vc = std::make_unique<ViewCursor>();

    auto shader = ShaderLibrary::get_instance().get_shader("cursor");
    auto font = FontLibrary::get_default_font();

    shader->setup();
    shader->setup_fillcolor_ids();
    vc->mvp = owning_view->mvp;

    vc->line = buf_curs.line;
    vc->index = buf_curs.pos;
    vc->cursor_data = std::move(cursor_vao);
    vc->line_shade_data = std::move(line_shade_vao);
    vc->view = owning_view.get();
    vc->shader = shader;
    vc->setup_dimensions(8, font->max_glyph_height + 4);

    return vc;
}

std::unique_ptr<ViewCursor> ViewCursor::create_from(View *view) {
    auto buf_curs = view->get_text_buffer()->get_cursor();

    auto cursor_vao = CursorVAO::make(GL_ARRAY_BUFFER, 6 * 1024);
    auto vc = std::make_unique<ViewCursor>();

    auto shader = ShaderLibrary::get_instance().get_shader("cursor");
    auto font = FontLibrary::get_default_font();
    auto line_shade_vao = CursorVAO::make(GL_ARRAY_BUFFER, 6 * 1024);

    shader->setup();
    shader->setup_fillcolor_ids();

    vc->mvp = view->mvp;
    vc->line = buf_curs.line;
    vc->index = buf_curs.pos;
    vc->cursor_data = std::move(cursor_vao);
    vc->line_shade_data = std::move(line_shade_vao);
    vc->view = view;
    vc->shader = shader;
    vc->setup_dimensions(4, font->max_glyph_height + 4);
    return vc;
}

void ViewCursor::draw(bool isActive) {
    shader->use();
    // shader->set_projection(projection);
    shader->set_projection(mvp);

    // draw highlight first, because we want cursor on top of it
    line_shade_data->bind_all();
    shader->set_fillcolor(line_shade_color);
    line_shade_data->draw();
    cursor_data->bind_all();
    shader->set_fillcolor(cursor_fill_color);
    cursor_data->draw();
}

void ViewCursor::forced_draw() {
    shader->use();
    // shader->set_projection(projection);
    shader->set_projection(mvp);
    // draw highlight first, because we want cursor on top of it

    line_shade_data->bind_all();
    shader->set_fillcolor(line_shade_color);
    line_shade_data->flush_and_draw();

    cursor_data->bind_all();
    shader->set_fillcolor(cursor_fill_color);
    cursor_data->flush_and_draw();
}

void ViewCursor::update_cursor_data(GLfloat x, GLfloat y) {
    pos_x = AS(x, int);
    pos_y = AS(y, int);
    auto w = width;
    auto h = height;
    auto &c_data = cursor_data->vbo->data;
    auto &l_data = line_shade_data->vbo->data;

    auto view_width = view->width;

    cursor_data->vbo->pristine = false;
    line_shade_data->vbo->pristine = false;

    c_data.clear();
    c_data.emplace_back(x, y + h);
    c_data.emplace_back(x, y);
    c_data.emplace_back(x + w, y);
    c_data.emplace_back(x, y + h);
    c_data.emplace_back(x + w, y);
    c_data.emplace_back(x + w, y + h);

    auto vx = AS(view->x, float);
    l_data.clear();
    l_data.emplace_back(vx, y + h);
    l_data.emplace_back(vx, y);
    l_data.emplace_back(vx + view_width, y);
    l_data.emplace_back(vx, y + h);
    l_data.emplace_back(vx + view_width, y);
    l_data.emplace_back(vx + view_width, y + h);

}

void ViewCursor::set_line_rect(GLfloat x1, GLfloat x2, GLfloat y1) {
    auto w = x2 - x1;
    auto h = height;
    auto x = x1;
    auto y = y1;
    auto &data = this->cursor_data->vbo->data;
    this->cursor_data->vbo->pristine = false;
    data.clear();
    data.emplace_back(x, y + h);
    data.emplace_back(x, y);
    data.emplace_back(x + w, y);
    data.emplace_back(x, y + h);
    data.emplace_back(x + w, y);
    data.emplace_back(x + w, y + h);
}



void ViewCursor::setup_dimensions(int Width, int Height) {
    this->width = Width;
    this->height = Height;
}
// void ViewCursor::set_projection(glm::mat4 orthoProjection) { this->projection = orthoProjection; }

void ViewCursor::set_line_rect(GLfloat x1, GLfloat x2, GLfloat y1, int rectHeight) {
    auto w = x2 - x1;
    auto h = rectHeight;
    auto x = x1;
    auto y = y1;
    auto &data = cursor_data->vbo->data;
    cursor_data->vbo->pristine = false;
    data.clear();
    data.emplace_back(x, y + h);
    data.emplace_back(x, y);
    data.emplace_back(x + w, y);
    data.emplace_back(x, y + h);
    data.emplace_back(x + w, y);
    data.emplace_back(x + w, y + h);
}
void ViewCursor::set_projection(Matrix orthoProjection) {
    this->mvp = orthoProjection;
}

}// namespace ui