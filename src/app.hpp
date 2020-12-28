//
// Created by 46769 on 2020-12-22.
//

#pragma once
#include <GLFW/glfw3.h>
#include <filesystem>
#include <glm/glm.hpp>
#include <string>
#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>

#include <core/text_data.hpp>
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>
#include <ui/view.hpp>

namespace fs = std::filesystem;

class App {
public:
    static App *create(int app_width, int app_height, const std::string &title = "cxedit");
    ~App();
    void run_loop();
    void set_dimensions(int w, int h);
    void load_file(const fs::path &file);
    void draw_all(bool force_redraw = false);
    void update_views_projections();
    void update_views_dimensions();
    void update_projection();
    [[nodiscard]] View *get_active_view() const;
    void set_input_to_command_view(bool toggleOn);

private:
    void cleanup();
    GLFWwindow *window;
    std::string title;
    int win_height;
    int win_width;
    int scroll;
    glm::mat4 projection;

    std::vector<std::unique_ptr<TextData>> data;
    std::vector<std::unique_ptr<View>> views;
    std::unique_ptr<CommandView> command_view;

    TextData *active_buffer;
    View *active_view;
    bool reading_comand{false};

    bool no_close_condition();
    void kb_command(int i);
    void graceful_exit();
    bool exit_command_requested;
    bool edit_command = false;
    void input_char(char ch);
    void handle_input(int i, int i1);
};