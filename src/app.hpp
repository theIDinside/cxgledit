//
// Created by 46769 on 2020-12-22.
//

#pragma once
#include <windows.h>

#include <GLFW/glfw3.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <stack>
#include <string>
#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>

#include <bindingslib/keybindings.hpp>
#include <core/commands/command_interpreter.hpp>
#include <core/text_data.hpp>
#include <ui/core/layout.hpp>
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>
#include <ui/modal.hpp>

typedef Action(__cdecl *KeyBindingFn)(CXMode, KeyInput);

namespace ui {
class View;
class EditorWindow;
class StatusBar;
class CommandView;
}// namespace ui

namespace fs = std::filesystem;
enum class Cycle;

struct WindowDimensions {
    int w, h;
};

enum class SplitStrategy { Stack, VerticalSplit, HorizontalSplit };

struct DataCopy {
    std::size_t begin;
    std::size_t len;
};

struct Register {
    Register() : copies{}, store{} { store.reserve(2000); }
    std::vector<DataCopy> copies;
    std::vector<char> store;
    void push_view(std::string_view data);
    std::optional<std::string_view> get(std::size_t index);
    std::optional<std::string_view> get_last();
};

void reload_keybinding_library();

class App {
public:
    static App *initialize(int app_width, int app_height, const std::string &title = "cxedit");
    ~App();
    void run_loop();
    void set_dimensions(int w, int h);
    void load_file(const fs::path &file);
    void draw_all(bool force_redraw = false);
    void update_views_dimensions(float wRatio, float hRatio);
    void toggle_command_input(const std::string &prefix, Commands commandInput);
    void disable_command_input();
    void set_error_message(const std::string &msg);
    void new_editor_window(SplitStrategy ss = SplitStrategy::Stack);
    void editor_win_selected(ui::EditorWindow *window);

    static WindowDimensions get_window_dimension();
    [[nodiscard]] ui::CommandView *get_command_view() const;
    [[nodiscard]] ui::EditorWindow *get_active_window() const;
    void editor_window_goto(int line);
    void restore_input();

    void fwrite_active_to_disk(const std::string &path);

    int win_height;
    int win_width;
    KeyBindingFn bound_action = nullptr;
    void reload_keybindings();
    void print_debug_info();
    void kb_command(KeyInput input);
    /**
     * Handle keyboard input, when we are in "edit" mode, meaning when we are editing actual text.
     * @param i
     * @param i1
     */
    void handle_edit_input(KeyInput input);

private:
    void cleanup();
    GLFWwindow *window;
    std::string title;
    int scroll;
    bool exit_command_requested;
    bool command_edit = false;
    glm::mat4 projection;
    std::vector<ui::EditorWindow *> editor_views;
    ui::EditorWindow *active_window;
    std::unique_ptr<ui::CommandView> command_view;
    TextData *active_buffer{nullptr};
    ui::View *active_view{nullptr};
    ui::Modal* modal_popup;
    Register copy_register{};
    ui::core::Layout *root_layout{nullptr};
    HMODULE kb_library = nullptr;
    CXMode mode = CXMode::Normal;
    bool no_close_condition();
    void graceful_exit();
    void update_all_editor_windows();
    void input_command_or_newline();
    void cycle_command_or_move_cursor(Cycle cycle);
    static WindowDimensions win_dimensions;
    void toggle_modal_popup();
    bool modal_shown;
};