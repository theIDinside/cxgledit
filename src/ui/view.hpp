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
#include <ui/core/layout.hpp>

/// ---- Forward declarations
struct ColorizeTextRange;
struct BufferCursor;
/// !!!! Forward declarations

auto convert_to_gl_anchor(int item_top_y, int item_height) -> int;

namespace ui {

    enum class ViewType {
        Text,
        Command,
        List,
    };

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
                                               std::optional<std::vector<ColorizeTextRange>> colorInfo,
                                               bool isActive = false);
        void draw_command_view(const std::string &prefix, std::optional<std::vector<ColorizeTextRange>> colorInfo);
        void draw_statusbar();
        void set_projection(glm::mat4 projection);
        void set_dimensions(int w, int h);
        void anchor_at(int x, int y);
        void scroll(Scroll direction, int linesToScroll);
        void set_fill(float w, float h, int parent_w, int parent_h);

        SimpleFont *get_font();
        [[nodiscard]] TextData *get_text_buffer() const;

        ViewCursor *get_cursor();
        std::string name{};
        int width{}, height{}, x{}, y{};
        int lines_displayable = -1;
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
        void draw_error_message();
    };
}