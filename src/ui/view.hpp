//
// Created by 46769 on 2020-12-22.
//

#pragma once

#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>
#include <ui/render/vertex_buffer.hpp>
#include <core/text_data.hpp>


enum class ViewType {
    Text,
    Command,
    List,
};

class View {
public:
    static constexpr auto TEXT_LENGTH_FROM_EDGE = 4u;
    static std::unique_ptr<View> create(TextData* data, const std::string& name, int w, int h, int x, int y, ViewType type = ViewType::Text);
    void draw();
    void forced_draw();

    void forced_draw_with_prefix_colorized(const std::string& prefix, std::optional<std::vector<ColorizeTextRange>> colorInfo);
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
    std::size_t vertexCapacity{0};
    int scroll = 0;
    ViewType type = ViewType::Text;
    friend class CommandView;
};

class CommandView {
public:
    int x, y;
    int w, h;
    std::string name;
    std::string infoPrefix;
    std::unique_ptr<View> command_view;
    std::unique_ptr<TextData> input_buffer;
    void draw();
    void set_text(std::string_view& view);
    void set_prefix(const std::string& prefix);
    static std::unique_ptr<CommandView> create(const std::string& name, int width, int height, int x, int y);
    bool active;
    bool has_changed{false};
};
