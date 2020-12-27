//
// Created by 46769 on 2020-12-20.
//

#pragma once
#include <glad/glad.h>
#include <memory>
#include <string>
#include <vector>

using usize = std::size_t;

struct TextVertex {
    GLfloat x{}, y{}, u{}, v{};
    GLfloat r{}, g{}, b{};
};

constexpr auto gpu_mem_required(std::size_t characterCount) -> std::size_t {
    return sizeof(TextVertex) * 6 * characterCount;
}

struct TextVertices {
    explicit TextVertices(usize vertexCount);

    static TextVertices init_from_string(const std::string &text);
    void destroy();

    TextVertex *data;
    usize vertex_count;
    usize current_quad_index{0};
    void push_quad_then_delete(TextVertex *data);
    [[nodiscard]] bool complete() const;
    [[nodiscard]] auto bytes_size() const -> int;
};

struct TSTextVertices {
    explicit TSTextVertices(usize vertexCount);

    static TSTextVertices init_from_string(const std::string &text);
    void destroy();

    TextVertex *data;
    usize vertex_count;
    usize current_quad_index{0};
    void push_quad_then_delete(TextVertex *data);
    [[nodiscard]] bool complete() const;
    [[nodiscard]] auto bytes_size() const -> int;
};

using byte = unsigned char;
using usize = std::size_t;
using LocalStore = std::vector<TextVertex>;

struct VertexBufferObject {
    VertexBufferObject(GLuint id, GLenum bufferType, LocalStore &&reservedMemory);
    static std::unique_ptr<VertexBufferObject> create(GLuint vboId, GLenum bufferType, usize reservedSize = 0);
    void bind();
    int upload_to_gpu(bool clear_on_upload = true);
    void reserve_gpu_memory(std::size_t text_character_count);
    GLuint id{0};
    GLenum type;
    LocalStore data;
    usize reservedGPUMemory{0};
    bool created{false};
    /// flag indicating whether GPU has equal representation of data uploaded. if !pristine, GPU needs an upload to have the same
    bool pristine{false};
};

struct VAO {
    static std::unique_ptr<VAO> make(GLenum VBOType, usize reservedVertexSpace = 0);
    void bind_all();
    void flush_and_draw();
    void draw();
    void reserve_gpu_size(std::size_t text_character_count);
    GLuint vao_id;
    std::unique_ptr<VertexBufferObject> vbo;
    int last_items_rendered;
};