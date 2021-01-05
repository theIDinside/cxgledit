//
// Created by 46769 on 2020-12-22.
//

#pragma once

#include "view_cursor.hpp"
#include <GLFW/glfw3.h>
#include <core/text_data.hpp>
#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>
#include <ui/render/vertex_buffer.hpp>
#include <ui/layout.hpp>

/// ---- Forward declarations
struct ColorizeTextRange;
struct BufferCursor;
/// !!!! Forward declarations

auto convert_to_gl_anchor(int item_top_y, int item_height) -> int;

enum class ViewType {
    Text,
    Command,
    List,
};

/// Boxed = owned & RAII'd
template<typename T>
using Boxed = std::unique_ptr<T>;

enum class Scroll {
    Up = GLFW_KEY_UP,
    Down = GLFW_KEY_DOWN,
};

struct View {
    ~View() { util::println("Destroying View {} and it's affiliated resources", name); }

    static constexpr auto TEXT_LENGTH_FROM_EDGE = 4u;
    static std::unique_ptr<View> create_managed(TextData *data, const std::string &name, int w, int h, int x, int y,
                                                ViewType type = ViewType::Text);

    static View *create(TextData *data, const std::string &name, int w, int h, int x, int y,
                        ViewType type = ViewType::Text);
    void draw(bool isActive = false);
    void forced_draw(bool isActive = false);

    void forced_draw_with_prefix_colorized(const std::string &prefix,
                                           std::optional<std::vector<ColorizeTextRange>> colorInfo, bool isActive=false);
    void draw_command_view(const std::string &prefix, std::optional<std::vector<ColorizeTextRange>> colorInfo);
    void draw_statusbar();
    void set_projection(glm::mat4 projection);
    void set_dimensions(int w, int h);
    void update_layout(DimInfo dimInfo);
    void anchor_at(int x, int y);
    void scroll(Scroll direction, int linesToScroll);
    void set_fill(float w, float h, int parent_w, int parent_h);
    void set_name(const std::string &name);
    SimpleFont *get_font();
    [[nodiscard]] TextData *get_text_buffer() const;

    ViewCursor *get_cursor();
    std::string name{};
    int width{}, height{}, x{}, y{};
    float width_fill{1.0f};
    float height_fill{1.0f};

private:
    /// Anonymous for now, will pull out this later on
    struct {
        TextData *data = nullptr;
        int td_id;
    };

    std::unique_ptr<VAO> vao{nullptr};// the graphical representation
    SimpleFont *font = nullptr;
    Shader *shader = nullptr;
    glm::mat4 projection;
    std::size_t vertexCapacity{0};
    int scrolled = 0;
    Boxed<ViewCursor> cursor;
    ViewType type = ViewType::Text;
    friend class CommandView;
    friend class ViewCursor;
    friend class StatusBar;
    friend class App;
};

class CommandView {
public:
    int x, y;
    int w, h;
    std::string name;
    std::string infoPrefix;
    Boxed<View> command_view;
    Boxed<TextData> input_buffer;
    bool show_last_message = false;
    std::string last_message{};

    void draw();
    void set_prefix(const std::string &prefix);
    static Boxed<CommandView> create(const std::string &name, int width, int height, int x, int y);
    static CommandView *create_not_managed(const std::string &name, int width, int height, int x, int y);
    bool active;
    bool has_changed{false};
};

struct StatusBar {
    Boxed<View> ui_view;
    TextData::BufferCursor *active_buffer_cursor;
    glm::vec3 bg_color{0.25f, 0.25f, 0.27f};
    glm::vec3 font_color{0.9f, 0.8f, 0.6f};

    void draw(View *view);
    void set_buffer_cursor(TextData::BufferCursor *cursor);
    static Boxed<StatusBar> create_managed(int width, int height, int x, int y);
    static StatusBar *create(int width, int height, int x, int y);
    void print_debug_info() const;

    void update_layout(DimInfo dimInfo);
};

struct EditorWindow {
    View *view;
    StatusBar *status_bar;
    int ui_layout_id;
    DimInfo dimInfo;

    void draw(bool force_redraw = false);
    ~EditorWindow();

    TextData *get_text_buffer();
    static EditorWindow* create(std::optional<TextData *> textData, glm::mat4 projection, int layout_id, DimInfo dimInfo);
    void update_layout(DimInfo dimInfo);
    void set_projection(glm::mat4 projection);
    bool active = false;
};