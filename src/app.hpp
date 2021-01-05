//
// Created by 46769 on 2020-12-22.
//

#pragma once
#include <GLFW/glfw3.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <stack>
#include <string>
#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>

#include <core/commands/command_interpreter.hpp>
#include <core/text_data.hpp>
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>
#include <ui/view.hpp>
#include <ui/layout.hpp>


namespace fs = std::filesystem;

enum class Cycle;

struct WindowDimensions {
    int w, h;
};

enum class SplitStrategy {
    Stack,
    VerticalSplit,
    HorizontalSplit
};

class App {
public:
    static App *initialize(int app_width, int app_height, const std::string &title = "cxedit");
    ~App();
    void run_loop();
    void set_dimensions(int w, int h);
    void load_file(const fs::path &file);
    void draw_all(bool force_redraw = false);

    void update_views_dimensions(float wRatio, float hRatio);
    [[nodiscard]] View *get_active_view() const;
    void set_input_to_command_view(bool toggleOn);
    void set_error_message(std::string msg);
    void new_editor_window(SplitStrategy ss = SplitStrategy::Stack);

    void editor_win_selected(EditorWindow* window);
    static WindowDimensions get_window_dimension();
private:
    void cleanup();
    GLFWwindow *window;
    std::string title;
    int win_height;
    int win_width;
    int scroll;
    bool exit_command_requested;
    bool command_edit = false;
    glm::mat4 projection;
    std::vector<EditorWindow*> editor_views;
    EditorWindow* active_window;
    std::unique_ptr<CommandView> command_view;

    TextData *active_buffer;
    View *active_view;

    Layout* root_layout;

    bool no_close_condition();
    void kb_command(int i);
    void graceful_exit();
    void input_char(char ch);
    void update_all_editor_windows();


    /**
     * Handle keyboard input, when we are in "edit" mode, meaning when we are editing actual text.
     * @param i
     * @param i1
     */
    void handle_edit_input(int i, int i1);
    void input_command_or_newline();
    void cycle_command_or_move_cursor(Cycle cycle);
    void print_debug_info();


    static WindowDimensions win_dimensions;

};