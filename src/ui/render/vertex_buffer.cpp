//
// Created by 46769 on 2020-12-20.
//

#include "vertex_buffer.hpp"
#include <core/core.hpp>

std::unique_ptr<VertexBufferObject> VertexBufferObject::create(GLuint vboId, GLenum type, const usize reservedSize) {
    LocalStore data;
    data.reserve(reservedSize > 0 ? reservedSize : 1024);
    auto vbo = std::make_unique<VertexBufferObject>(vboId, type, std::move(data));
    vbo->reservedGPUMemory = reservedSize * sizeof(TextVertex);
    vbo->created = true;
    return vbo;
}

VertexBufferObject::VertexBufferObject(GLuint id, GLenum bufferType, LocalStore &&reservedMemory)
    : id(id), type(bufferType), data(std::move(reservedMemory)) {}
void VertexBufferObject::bind() { glBindBuffer(this->type, this->id); }

int VertexBufferObject::upload_to_gpu(bool clear_on_upload) {
    auto vertices = data.size();
    glBufferSubData(GL_ARRAY_BUFFER, 0, this->data.size() * sizeof(TextVertex), data.data());
    if (clear_on_upload) { data.clear(); }
    return vertices;
}
void VertexBufferObject::reserve_gpu_memory(std::size_t text_character_count) {

    glBufferData(GL_ARRAY_BUFFER, gpu_mem_required(text_character_count), nullptr, GL_DYNAMIC_DRAW);
    reservedGPUMemory = gpu_mem_required(text_character_count);// text_character_count * sizeof(TextVertex) * 6;
}
void VAO::bind_all() {
    glBindVertexArray(vao_id);
    vbo->bind();
}
std::unique_ptr<VAO> VAO::make(GLenum VBOType, usize reservedVertexSpace) {
    auto vaoID = 0u;
    auto vboID = 0u;
    glGenVertexArrays(1, &vaoID);
    glGenBuffers(1, &vboID);
    glBindVertexArray(vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, reservedVertexSpace, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *) offsetof(TextVertex, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TextVertex), (void *) offsetof(TextVertex, r));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    auto vbo = VertexBufferObject::create(vboID, VBOType, reservedVertexSpace);
    auto vao = std::make_unique<VAO>(VAO{.vao_id = vaoID, .vbo = std::move(vbo)});
    return vao;
}
void VAO::flush_and_draw() {
    bind_all();
    auto count = vbo->upload_to_gpu(true);
    this->last_items_rendered = count;
    glDrawArrays(GL_TRIANGLES, 0, count);
}
void VAO::reserve_gpu_size(std::size_t text_character_count) {
    bind_all();
    vbo->reserve_gpu_memory(text_character_count);
}
void VAO::draw() {
    bind_all();
    glDrawArrays(GL_TRIANGLES, 0, last_items_rendered);
}

auto TextVertices::bytes_size() const -> int { return sizeof(TextVertex) * vertex_count; }

TextVertices::TextVertices(usize vertexCount) : data(new TextVertex[vertexCount]), vertex_count(vertexCount) {}

bool TextVertices::complete() const { return (current_quad_index * 6) - vertex_count == 0; }
void TextVertices::destroy() { delete[] data; }
TextVertices TextVertices::init_from_string(const std::string &text) {
    return TextVertices{text.size() * sizeof(TextVertex)};
}
TSTextVertices TSTextVertices::init_from_string(const std::string &text) {
    return TSTextVertices{text.size() * sizeof(TextVertex)};
}
TSTextVertices::TSTextVertices(usize vertexCount) : data(new TextVertex[vertexCount]), vertex_count(vertexCount) {}