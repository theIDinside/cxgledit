//
// Created by 46769 on 2020-12-22.
//

#pragma once
#include <GLFW/glfw3.h>

#include "cursors/view_cursor.hpp"
#include "view_enums.hpp"
#include <core/buffer/text_data.hpp>

#include <core/math/vector.hpp>
#include <core/math/matrix.hpp>

#include <ui/core/layout.hpp>
#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>
#include <ui/render/vertex_buffer.hpp>

/// ---- Forward declarations
struct ColorizeTextRange;
struct BufferCursor;
/// !!!! Forward declarations

auto convert_to_gl_anchor(int item_top_y, int item_height) -> int;

namespace ui {

struct View {
    ~View();
    static constexpr auto TEXT_LENGTH_FROM_EDGE = 4u;
    static std::unique_ptr<View> create_managed(TextData *data, const std::string &name, int w, int h, int x, int y,
                                                ViewType type = ViewType::Text);

    static View *create(TextData *data, const std::string &name, int w, int h, int x, int y,
                        ViewType type = ViewType::Text);

    void draw(bool isActive = false);
    /// This forces the View to re-create all the vertex data, and update some of it's dimension info
    /// this becomes useful when we have resized the window, and/or the view, as suddenly, the amount of lines that can be displayed changes, etc
    void forced_draw(bool isActive = false);
    void draw_command_view(const std::string &prefix, std::optional<std::vector<ColorizeTextRange>> colorInfo);
    void draw_statusbar();
    void draw_modal_view(int selected, std::vector<TextDrawable>& drawables);
    void scroll_to(int line);

    void set_projection(Matrix projection);
    void set_dimensions(int w, int h);
    void anchor_at(int x, int y);

    SimpleFont *get_font();
    void set_font(SimpleFont *new_font);
    [[nodiscard]] TextData *get_text_buffer() const;

    ViewCursor *get_cursor();
    std::string name{};
    int width{}, height{}, x{}, y{};
    int lines_displayable = -1;
    std::unique_ptr<VAO> vao{nullptr};// the graphical representation
    Vec3f fg_color{1.0f, 1.0f, 1.0f};
    Vec3f bg_color{0.05f, 0.052f, 0.0742123f};
    Vec3f when_active_bg_color{0.05f, 0.052f, 0.0742123f};
    Matrix mvp;
    /// Anonymous for now, will pull out this later on
    struct {
        TextData *data = nullptr;
        int td_id;
    };

    SimpleFont *font = nullptr;
    Shader *shader = nullptr;
    std::size_t vertexCapacity{0};
    int scrolled = 0;
    int lines_scrolled = 0;
    Boxed<ViewCursor> cursor;
    ViewType type = ViewType::Text;

    std::pair<std::string_view, std::string_view> debug_print_boundary_lines();
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

    void draw_current();
    void draw_error_message(std::string &&msg);
    void draw_error_message();
    void draw_message(std::string &&msg);
};
}// namespace ui