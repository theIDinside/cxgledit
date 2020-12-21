//
// Created by 46769 on 2020-12-20.
//

#include "vertex_buffer.hpp"
std::unique_ptr<VertexBufferObject> VertexBufferObject::create(GLenum type, const usize reservedSize) {
    auto id = 0u;
    glGenBuffers(1, &id);
    LocalStore data;
    data.reserve(reservedSize > 0 ? reservedSize : 1024);
    auto vbo = std::make_unique<VertexBufferObject>(id, type, std::move(data));
    vbo->created = true;
    return vbo;
}

VertexBufferObject::VertexBufferObject(GLuint id, GLenum bufferType, LocalStore &&reservedMemory)
    : id(id), type(bufferType), data(std::move(reservedMemory)) {

}
void VertexBufferObject::bind() {
    glBindBuffer(this->type, this->id);
}
void VAO::bind_all() {
    glBindVertexArray(vao_id);
    vbo->bind();
}
std::unique_ptr<VAO> VAO::make(GLenum VBOType, usize VBOReservedSize) {
    auto vaoID = 0u;
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
    auto vbo = VertexBufferObject::create(VBOType, VBOReservedSize);
    auto vao = std::make_unique<VAO>(VAO{.vao_id = vaoID, .vbo = std::move(vbo) });
    return vao;
}

auto TextVertices::bytes_size() const -> int {
    return sizeof(GLfloat) * 4 * vertex_count;
}

TextVertices::TextVertices(usize vertexCount) : data(new TextVertex[vertexCount]), vertex_count(vertexCount) {

}

void TextVertices::push_quad_then_delete(TextVertex v_data[6]) {
    auto curPtrPos = (data + (current_quad_index * 6));
    std::copy(v_data, v_data+6, curPtrPos);
    delete[] v_data;
    current_quad_index++;
}
bool TextVertices::complete() const {
    return (current_quad_index * 6) - vertex_count == 0;
}
void TextVertices::destroy() {
    delete[] data;
}
TextVertices TextVertices::init_from_string(const std::string &text) {

    return TextVertices{text.size() * 6};
}
TSTextVertices TSTextVertices::init_from_string(const std::string &text) {
    return TSTextVertices{text.size() * 4};
}
TSTextVertices::TSTextVertices(usize vertexCount) : data(new TextVertex[vertexCount]), vertex_count(vertexCount) {}
