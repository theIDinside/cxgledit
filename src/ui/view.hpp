//
// Created by 46769 on 2020-12-22.
//

#pragma once

#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>
#include <ui/render/vertex_buffer.hpp>
#include <core/text_data.hpp>


class View {
public:
    static constexpr auto TEXT_LENGTH_FROM_EDGE = 4u;
    static std::unique_ptr<View> create(TextData* data, const std::string& name, int w, int h, int x, int y);
    void draw();
    void forced_draw();
    void set_projection(glm::mat4 projection);
    void set_dimensions(int w, int h);
    void anchor_at(int x, int y);
    void set_scroll(int scroll_pos);
    SimpleFont* get_font();
    [[nodiscard]] TextData* get_text_buffer() const;
private:
    int width{}, height{}, x{}, y{};
    std::unique_ptr<VAO> vao{nullptr}; // the graphical representation
    SimpleFont* font = nullptr;
    Shader* shader = nullptr;
    std::string name{};
    TextData* data = nullptr;
    glm::mat4 projection;
    int scroll = 0;
};
