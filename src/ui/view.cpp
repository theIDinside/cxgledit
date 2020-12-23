//
// Created by 46769 on 2020-12-22.
//

#include "view.hpp"
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>

std::unique_ptr<View> View::create(TextData *data, const std::string &name, int w, int h, int x, int y) {
    auto reserveMemory = gpu_mem_required(1024);// reserve GPU memory for at least 1024 characters.
    if (!data->empty()) { reserveMemory = gpu_mem_required(data->size()); }
    auto vao = VAO::make(GL_ARRAY_BUFFER, reserveMemory);
    auto font = FontLibrary::get_default_font();
    auto shader = ShaderLibrary::get_text_shader();
    auto v = std::make_unique<View>();

    v->width = w;
    v->height = h;
    v->x = x;
    v->y = y;
    v->font = font;
    v->shader = shader;
    v->vao = std::move(vao);
    v->data = data;

    return v;
}
void View::draw() {
    vao->bind_all();
    shader->use();
    font->t->bind();
    // projection = glm::ortho(0.0f, static_cast<float>(ww), 0.0f, static_cast<float>(wh));
    shader->set_projection(projection);
    auto vao_ = vao.get();

    if(data->is_pristine()) {
        vao->draw();
    } else {
        auto data_view = data->to_string_view();
        font->emplace_gpu_data(vao_, data_view, this->x + View::TEXT_LENGTH_FROM_EDGE, this->y - font->get_row_advance());
        vao->flush_and_draw();
    }
}
void View::set_projection(glm::mat4 view_projection) {
    this->projection = std::move(view_projection);
}
void View::set_dimensions(int w, int h) {
    this->width = w;
    this->height = h;
}
void View::anchor_at(int x, int y) {
    this->x = x;
    this->y = y;
}
SimpleFont *View::get_font() {
    return font;
}
TextData *View::get_text_buffer() const { return data; }
void View::forced_draw() {
    FN_MICRO_BENCH();
    vao->bind_all();
    shader->use();
    font->t->bind();
    auto data_view = data->to_string_view();
    font->emplace_gpu_data(vao.get(), data_view, this->x + View::TEXT_LENGTH_FROM_EDGE, this->y - font->get_row_advance());
    vao->flush_and_draw();
}
void View::set_scroll(int scrollPos) {
    this->scroll = scrollPos;
}
