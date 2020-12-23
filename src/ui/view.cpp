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
    glm::vec3 col{1.0, 0.5, 0.0};
    vao->bind_all();
    shader->use();
    font->t->bind();
    // projection = glm::ortho(0.0f, static_cast<float>(ww), 0.0f, static_cast<float>(wh));
    // glUniform3f(glGetUniformLocation(shader.ID, "textColor"), col.x, col.y, col.z);
    shader->setVec3("textColor", col);
    shader->setMat4("projection", projection);
    auto vao_ = vao.get();
    auto data_ = data->to_string_view();


    font->emplace_gpu_data(
            vao_,
            data_,
            this->x + 3, this->y - 10);
    vao->flush_and_draw();
    // glBufferData(GL_ARRAY_BUFFER, vertexData.bytes_size(), vertexData.data, GL_DYNAMIC_DRAW);
}
void View::set_projection(glm::mat4 projection) {
    this->projection = projection;
}
void View::set_dimensions(int w, int h) {
    this->width = w;
    this->height = h;
}
void View::anchor_at(int x, int y) {
    this->x = x;
    this->y = y;
}
