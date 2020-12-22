//
// Created by 46769 on 2020-12-22.
//

#pragma once
#include <filesystem>
#include <GLFW/glfw3.h>
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
    void load_file(const fs::path& file);
    void draw_all();
    void update_views_projections();
    void update_views_dimensions();

private:
    void cleanup();
    GLFWwindow *window;
    std::string title;
    int win_height;
    int win_width;
    glm::mat4 projection;

    std::vector<std::unique_ptr<TextData>> data;
    std::vector<std::unique_ptr<View>> views;
    TextData* active;
    bool no_close_condition();
};