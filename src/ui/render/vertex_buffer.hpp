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
};

struct TextVertices {
    explicit TextVertices(usize vertexCount);


    static TextVertices init_from_string(const std::string& text);
    void destroy();

    TextVertex* data;
    usize vertex_count;
    usize current_quad_index{0};
    void push_quad_then_delete(TextVertex* data);
    [[nodiscard]] bool complete() const;
    [[nodiscard]] auto bytes_size() const -> int;
};

struct TSTextVertices {
    explicit TSTextVertices(usize vertexCount);


    static TSTextVertices init_from_string(const std::string& text);
    void destroy();

    TextVertex* data;
    usize vertex_count;
    usize current_quad_index{0};
    void push_quad_then_delete(TextVertex* data);
    [[nodiscard]] bool complete() const;
    [[nodiscard]] auto bytes_size() const -> int;
};


using byte = unsigned char;
using usize = std::size_t;
using LocalStore = std::vector<byte>;
class VertexBufferObject {
public:
    VertexBufferObject(GLuint id, GLenum bufferType, LocalStore&& reservedMemory);
    static std::unique_ptr<VertexBufferObject> create(GLenum bufferType, usize reservedSize = 0);
    void bind();
    void add_data(const void* data, usize data_length, int repeat = 1);
private:
    GLuint id{0};
    GLenum type;
    LocalStore data;
    usize bytes_added{0};
    usize bytes_uploaded{0};
    bool created{false};
    /// flag indicating whether GPU has equal representation of data uploaded. if !pristine, GPU needs an upload to have the same
    bool pristine{false};

};

struct VAO {
    static std::unique_ptr<VAO> make(GLenum VBOType, usize VBOReservedSize = 0);
    void bind_all();

    GLuint vao_id;
    std::unique_ptr<VertexBufferObject> vbo;
};