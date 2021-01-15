//
// Created by 46769 on 2020-12-27.
//
#include "view_cursor.hpp"
#include "../view.hpp"

#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>

namespace ui {

// View Cursor


void ViewCursor::draw(bool isActive) {
    gpu_data->bind_all();
    shader->use();
    shader->set_projection(projection);
    shader->set_fillcolor(glm::vec3{1.0, 1.0, 1.0});
    gpu_data->draw();
}

void ViewCursor::forced_draw() {
    gpu_data->bind_all();
    shader->use();
    shader->set_projection(projection);
    shader->set_fillcolor(glm::vec3{1.0, 0.5, 1.0});
    gpu_data->flush_and_draw();
}

void ViewCursor::update_cursor_data(GLfloat x, GLfloat y) {
    pos_x = AS(x, int);
    pos_y = AS(y, int);
    auto w = width;
    auto h = height;
    auto &data = this->gpu_data->vbo->data;
    this->gpu_data->vbo->pristine = false;
    data.clear();
    data.emplace_back(x, y + h);
    data.emplace_back(x, y);
    data.emplace_back(x + w, y);
    data.emplace_back(x, y + h);
    data.emplace_back(x + w, y);
    data.emplace_back(x + w, y + h);
}

void ViewCursor::set_rect(GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2) {
    auto w = x2 - x1;
    auto h = y2 - y1;
    pos_x = x1;
    pos_y = y1;
    auto x = x1;
    auto y = y1;
    auto &data = this->gpu_data->vbo->data;
    this->gpu_data->vbo->pristine = false;
    data.clear();
    data.emplace_back(x, y + h);
    data.emplace_back(x, y);
    data.emplace_back(x + w, y);
    data.emplace_back(x, y + h);
    data.emplace_back(x + w, y);
    data.emplace_back(x + w, y + h);
}

void ViewCursor::set_line_rect(GLfloat x1, GLfloat x2, GLfloat y1) {
    auto w = x2 - x1;
    auto h = height;
    auto x = x1;
    auto y = y1;
    auto &data = this->gpu_data->vbo->data;
    this->gpu_data->vbo->pristine = false;
    data.clear();
    data.emplace_back(x, y + h);
    data.emplace_back(x, y);
    data.emplace_back(x + w, y);
    data.emplace_back(x, y + h);
    data.emplace_back(x + w, y);
    data.emplace_back(x + w, y + h);
}

std::unique_ptr<ViewCursor> ViewCursor::create_from(std::unique_ptr<View> &owning_view) {
    auto buf_curs = owning_view->get_text_buffer()->get_cursor();

    auto cursor_vao = CursorVAO::make(GL_ARRAY_BUFFER, 6 * 4096);
    auto vc = std::make_unique<ViewCursor>();

    auto shader = ShaderLibrary::get_instance().get_shader("cursor");
    auto font = FontLibrary::get_default_font();

    shader->setup();
    shader->setup_fillcolor_id();

    vc->projection = owning_view->projection;
    vc->col = buf_curs.col_pos;
    vc->line = buf_curs.line;
    vc->index = buf_curs.pos;
    vc->gpu_data = std::move(cursor_vao);
    vc->view = owning_view.get();
    vc->shader = shader;
    vc->setup_dimensions(8, font->max_glyph_height + 4);
    return vc;
}

std::unique_ptr<ViewCursor> ViewCursor::create_from(View* view) {
    auto buf_curs = view->get_text_buffer()->get_cursor();

    auto cursor_vao = CursorVAO::make(GL_ARRAY_BUFFER, 6 * 1024);
    auto vc = std::make_unique<ViewCursor>();

    auto shader = ShaderLibrary::get_instance().get_shader("cursor");
    auto font = FontLibrary::get_default_font();

    shader->setup();
    shader->setup_fillcolor_id();

    vc->projection = view->projection;
    vc->col = buf_curs.col_pos;
    vc->line = buf_curs.line;
    vc->index = buf_curs.pos;
    vc->gpu_data = std::move(cursor_vao);
    vc->view = view;
    vc->shader = shader;
    vc->setup_dimensions(2, font->max_glyph_height + 4);
    return vc;
}
void ViewCursor::setup_dimensions(int Width, int Height) {
    this->width = Width;
    this->height = Height;
}
void ViewCursor::set_projection(glm::mat4 orthoProjection) {
    this->projection = orthoProjection;
}

void ViewCursor::set_line_rect(GLfloat x1, GLfloat x2, GLfloat y1, int rectHeight) {
    auto w = x2 - x1;
    auto h = rectHeight;
    auto x = x1;
    auto y = y1;
    auto &data = this->gpu_data->vbo->data;
    this->gpu_data->vbo->pristine = false;
    data.clear();
    data.emplace_back(x, y + h);
    data.emplace_back(x, y);
    data.emplace_back(x + w, y);
    data.emplace_back(x, y + h);
    data.emplace_back(x + w, y);
    data.emplace_back(x + w, y + h);
}
void ViewCursor::set_line_rect(GLfloat x1, GLfloat x2, GLfloat y1, GLfloat y2) {
    auto w = x2 - x1;
    auto h = y1 - y2;
    auto x = x1;
    auto y = y1;
    auto &data = this->gpu_data->vbo->data;
    this->gpu_data->vbo->pristine = false;
    data.clear();
    data.emplace_back(x, y + h);
    data.emplace_back(x, y);
    data.emplace_back(x + w, y);
    data.emplace_back(x, y + h);
    data.emplace_back(x + w, y);
    data.emplace_back(x + w, y + h);
}

}// namespace ui