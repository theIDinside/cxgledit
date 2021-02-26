//
// Created by 46769 on 2020-12-22.
//

#pragma once
#include <windows.h>
#include <stack>
#include <string>

#include <GLFW/glfw3.h>
#include <filesystem>

#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>
#include <bindingslib/keybindings.hpp>

#include <core/math/matrix.hpp>
#include <core/buffer/text_data.hpp>
#include <core/commands/command_interpreter.hpp>

#include <ui/core/layout.hpp>
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>
#include <ui/modal.hpp>

#include <cfg/configuration.hpp>

typedef Action(__cdecl *KeyBindingFn)(CXMode, KeyInput);

namespace ui {
struct View;
struct EditorWindow;
struct StatusBar;
struct CommandView;
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
    bool modal_shown;
    CXMode mode = CXMode::Normal;
    ui::ModalPopup * modal_popup;
    std::string last_searched{};

    void reload_keybindings();
    void app_debug();

    void update_all_editor_windows();
    void input_command_or_newline();
    void cycle_command_or_move_cursor(Cycle cycle);

    void toggle_modal_popup(ui::ModalContentsType = ui::ModalContentsType::ActionList);
    void find_next_in_active(const std::string& search);
    void reload_configuration(fs::path cfg_path = "./assets/cxconfig.cxe");
    void handle_mouse_scroll(float xOffset, float yOffset);

    void handle_text_input(int codepoint);
    void handle_key_input(KeyInput input, int action);

    // Mode handlers
    void handle_normal_input(KeyInput input, int action);
    void handle_actions_input(KeyInput input, int action);
    void handle_command_input(KeyInput input, int action);
    void handle_popup_input(KeyInput input, int action);
    void handle_macro_record_input(KeyInput input, int action);

private:
    void cleanup();
    GLFWwindow *window;
    std::string title;
    int scroll;
    bool exit_command_requested;
    Matrix mvp;
    std::vector<ui::EditorWindow *> editor_views;
    ui::EditorWindow *active_window;
    std::unique_ptr<ui::CommandView> command_view;
    TextData *active_buffer{nullptr};
    ui::View *active_view{nullptr};

    Register copy_register{};
    ui::core::Layout *root_layout{nullptr};
    HMODULE kb_library = nullptr;

    Configuration config;

    bool no_close_condition();
    void graceful_exit();

    static WindowDimensions win_dimensions;
    void close_active();
    void handle_modal_selection(const std::optional<ui::PopupItem>& selected);
    void handle_command_input(KeyInput input);
    void start_command_input(const std::string &prefix, Commands commandInput);
    void set_last_as_active_editor_window();
};