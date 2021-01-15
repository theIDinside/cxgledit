//
// Created by 46769 on 2020-12-20.
//

#include "vertex_buffer.hpp"
#include <core/core.hpp>

std::unique_ptr<TextVertexBufferObject> TextVertexBufferObject::create(GLuint vboId, GLenum type,
                                                                       const usize reservedSize) {
    LocalStore<TextVertex> data;
    data.reserve(reservedSize > 0 ? reservedSize : 1024);
    auto vbo = std::make_unique<TextVertexBufferObject>(vboId, type, std::move(data));
    vbo->reservedMemInQuadCount = reservedSize * sizeof(TextVertex);
    vbo->created = true;
    return vbo;
}

TextVertexBufferObject::TextVertexBufferObject(GLuint id, GLenum bufferType, LocalStore<TextVertex> &&reservedMemory)
    : id(id), type(bufferType), data(std::move(reservedMemory)) {}

void TextVertexBufferObject::bind() { glBindBuffer(this->type, this->id); }

int TextVertexBufferObject::upload_to_gpu(bool clear_on_upload) {
    auto vertices = data.size();
    if(vertices == 0) {
        reserve_gpu_memory(1024);
    } else {
        auto vertSize = sizeof(TextVertex);
        auto quadsize = vertSize * 6;
        if(reservedMemInQuadCount == vertices * quadsize) {
            PANIC("BOOM");
        }
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, this->data.size() * sizeof(TextVertex), data.data());
    if (clear_on_upload) { data.clear(); }
    return vertices;
}
void TextVertexBufferObject::reserve_gpu_memory(std::size_t text_character_count) {
    auto vertexCountReserved = gpu_mem_required_for_quads<TextVertex>(text_character_count);
    glBufferData(GL_ARRAY_BUFFER, vertexCountReserved, nullptr,
                 GL_DYNAMIC_DRAW);
    reservedMemInQuadCount = vertexCountReserved;// text_character_count * sizeof(TextVertex) * 6;
}
TextVertexBufferObject::~TextVertexBufferObject() {
    destroy();
}
void TextVertexBufferObject::destroy() {
    auto ID = this->id;
    glDeleteBuffers(1, &this->id);
    util::println("Deleted VBO {}", ID);
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

    auto vbo = TextVertexBufferObject::create(vboID, VBOType, reservedVertexSpace);

    auto vao = std::make_unique<VAO>(vaoID, std::move(vbo));
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
void VAO::push_quad(std::array<TextVertex, 4> quad) {
    vbo->data.push_back(quad[0]);
    vbo->data.push_back(quad[1]);
    vbo->data.push_back(quad[2]);
    vbo->data.push_back(quad[0]);
    vbo->data.push_back(quad[2]);
    vbo->data.push_back(quad[3]);
}

VAO::~VAO() {

}

VAO::VAO(GLuint VAO_ID, std::unique_ptr<TextVertexBufferObject>&& VBO) : vao_id(VAO_ID), vbo(std::move(VBO)) {

}

/// ------------------------------------ CURSOR ------------------------------------

CursorVertexBufferObject::CursorVertexBufferObject(GLuint id, GLenum bufferType,
                                                   LocalStore<CursorVertex> &&reservedMemory)
    : id(id), type(bufferType), data(std::move(reservedMemory)) {}

std::unique_ptr<CursorVertexBufferObject> CursorVertexBufferObject::create(GLuint vboId, GLenum bufferType,
                                                                           usize reservedSize) {
    LocalStore<CursorVertex> data;
    data.reserve(reservedSize > 0 ? reservedSize : 1024);
    auto vbo = std::make_unique<CursorVertexBufferObject>(vboId, bufferType, std::move(data));
    vbo->reservedGPUMemory = reservedSize * sizeof(CursorVertex);
    vbo->created = true;
    return vbo;
}
void CursorVertexBufferObject::bind() { glBindBuffer(this->type, this->id); }
int CursorVertexBufferObject::upload_to_gpu(bool clear_on_upload) {
    auto vertices = data.size();
    glBufferSubData(GL_ARRAY_BUFFER, 0, this->data.size() * sizeof(CursorVertex), data.data());
    if (clear_on_upload) { data.clear(); }
    return vertices;
}
void CursorVertexBufferObject::reserve_gpu_memory(std::size_t quadsCount) {
    glBufferData(GL_ARRAY_BUFFER, gpu_mem_required_for_quads<CursorVertex>(quadsCount), nullptr,
                 GL_DYNAMIC_DRAW);
    reservedGPUMemory = gpu_mem_required_for_quads<CursorVertex>(quadsCount);// text_character_count * sizeof(TextVertex) * 6;
}
std::unique_ptr<CursorVAO> CursorVAO::make(GLenum VBOType, usize reservedVertexSpace) {
    auto vaoID = 0u;
    auto vboID = 0u;
    glGenVertexArrays(1, &vaoID);
    glGenBuffers(1, &vboID);
    glBindVertexArray(vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, reservedVertexSpace, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(CursorVertex), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    auto vbo = CursorVertexBufferObject::create(vboID, VBOType, reservedVertexSpace);
    auto vao = std::make_unique<CursorVAO>(CursorVAO{.vao_id = vaoID, .vbo = std::move(vbo)});
    return vao;
}
void CursorVAO::bind_all() {
    glBindVertexArray(vao_id);
    vbo->bind();
}
void CursorVAO::flush_and_draw() {
    bind_all();
    auto count = vbo->upload_to_gpu(false);
    this->last_items_rendered = count;
    glDrawArrays(GL_TRIANGLES, 0, count);
}
void CursorVAO::draw() {
    bind_all();
    glDrawArrays(GL_TRIANGLES, 0, last_items_rendered);
}

void CursorVAO::reserve_gpu_size(std::size_t quads_count) {
    bind_all();
    vbo->reserve_gpu_memory(quads_count);
}
