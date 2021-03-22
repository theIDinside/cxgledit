//
// Created by 46769 on 2020-12-20.
//

#pragma once
#include <core/core.hpp>
#include <glad/glad.h>
#include <memory>
#include <string>
#include <vector>

using usize = std::size_t;

struct Vertex {
    GLfloat x, y;
};

struct TextVertex {
    GLfloat x{}, y{}, u{}, v{};
    GLfloat r{}, g{}, b{};
};

template<typename T, Integral IntType>
constexpr auto gpu_mem_required_for_quads(IntType quads) -> std::size_t {
    return sizeof(T) * 6 * quads;
}

template <typename T>
using LocalStore = std::vector<T>;

struct CursorVertexBufferObject {
    CursorVertexBufferObject(GLuint id, GLenum bufferType, LocalStore<Vertex> &&reservedMemory);
    static std::unique_ptr<CursorVertexBufferObject> create(GLuint vboId, GLenum bufferType, usize reservedSize = 0);
    void bind();
    int upload_to_gpu(bool clear_on_upload = true);
    void reserve_gpu_memory(std::size_t quadsCount);
    GLuint id{0};
    GLenum type;
    LocalStore<Vertex> data;
    usize reservedGPUMemory{0};
    bool created{false};
    /// flag indicating whether GPU has equal representation of data uploaded. if !pristine, GPU needs an upload to have the same
    bool pristine{false};
};

struct CursorVAO {
    static std::unique_ptr<CursorVAO> make(GLenum VBOType, usize reservedVertexSpace = 0);
    void bind_all();
    void flush_and_draw();
    void draw();
    void reserve_gpu_size(std::size_t quads_count);
    GLuint vao_id;
    std::unique_ptr<CursorVertexBufferObject> vbo;
    int last_items_rendered;
};

using byte = unsigned char;
using usize = std::size_t;


struct TextVertexBufferObject {
    TextVertexBufferObject(GLuint id, GLenum bufferType, LocalStore<TextVertex> &&reservedMemory);
    TextVertexBufferObject(TextVertexBufferObject&&) = default;
    ~TextVertexBufferObject();


    TextVertexBufferObject(const TextVertexBufferObject&) = delete;
    static std::unique_ptr<TextVertexBufferObject> create(GLuint vboId, GLenum bufferType, usize reservedSize = 0);
    void bind();
    usize upload_to_gpu(bool clear_on_upload = true);
    void reserve_gpu_memory(std::size_t text_character_count);

    void destroy();
    GLuint id{0};
    GLenum type;
    LocalStore<TextVertex> data;
    usize reservedMemInQuadCount{0};
    usize reservedForcharacter = 1024;
    bool created{false};
    /// flag indicating whether GPU has equal representation of data uploaded. if !pristine, GPU needs an upload to have the same
    bool pristine{false};
};

struct TextVertexArrayObject {
    ~TextVertexArrayObject();
    TextVertexArrayObject(GLuint vao_id, std::unique_ptr<TextVertexBufferObject>&& vbo);
    TextVertexArrayObject(const TextVertexArrayObject &) = delete;
    TextVertexArrayObject(TextVertexArrayObject &&) = default;
    static std::unique_ptr<TextVertexArrayObject> make(GLenum VBOType, usize reservedVertexSpace = 0);
    void bind_all();
    void flush_and_draw();
    void draw();
    void reserve_gpu_size(std::size_t text_character_count);
    void push_quad(std::array<TextVertex, 4> quad);
    GLuint vao_id;
    std::unique_ptr<TextVertexBufferObject> vbo;
    int last_items_rendered{};
};