//
// Created by 46769 on 2020-12-22.
//

#include "app.hpp"
#include <core/core.hpp>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <utils/utils.hpp>

#define get_app_handle(window) (App*)glfwGetWindowUserPointer(window)

void framebuffer_callback(GLFWwindow *window, int width, int height) {
    fmt::print("New width: {}\t New height: {}\n", width, height);
    auto app = get_app_handle(window);
    app->set_dimensions(width, height);
    app->update_views_projections();
    glViewport(0, 0, width, height);
}

App *App::create(int app_width, int app_height, const std::string &title) {
    util::printmsg("Initializing application");
    auto instance = new App{};
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    auto window = glfwCreateWindow(app_width, app_height, title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        PANIC("Failed to create GLFW window\n");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_callback);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) { PANIC("Failed to initialize GLAD\n"); }
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    instance->win_height = app_height;
    instance->win_width = app_width;
    instance->projection = glm::ortho(0.0f, static_cast<float>(app_width), 0.0f, static_cast<float>(app_height));
    instance->title = title;
    instance->window = window;

    if(!instance->window) {
        glfwTerminate();
        PANIC("Failed to create GLFW Window");
    }

    // "assets/fonts/Ubuntu-R.ttf", 24, CharacterRange{.from = 0, .to = SWEDISH_LAST_ALPHA_CHAR_UNICODE }

    FontConfig default_font_cfg {
            .name = "Ubuntu-R",
            .path = "assets/fonts/Ubuntu-R.ttf",
            .pixel_size = 24,
            .char_range = CharacterRange{.from = 0, .to = SWEDISH_LAST_ALPHA_CHAR_UNICODE }
    };
    // "assets/shaders/textshader.vs", "assets/shaders/textshader.fs"
    ShaderConfig text_shader {
            .name = "text",
            .vs_path = "assets/shaders/textshader.vs",
            .fs_path = "assets/shaders/textshader.fs"
    };

    /*
    ShaderConfig cursor_shader {
            .name = "cursor",
            .vs_path = "",
            .fs_path = ""
    };

    ShaderConfig view_shader {
            .name = "view",
            .vs_path = "",
            .fs_path = ""
    };
    */

    FontLibrary::get_instance().load_font(default_font_cfg);
    ShaderLibrary::get_instance().load_shader(text_shader);
    auto bufHandle = StdStringBuffer::make_handle();
    instance->data.push_back(std::move(bufHandle));
    instance->active = instance->data.back().get();
    instance->load_file("main_2.cpp");

    auto v = View::create(instance->active, "main", app_width, app_height, 0, app_height);
    v->set_projection(instance->projection);
    instance->views.push_back(std::move(v));

    glfwSetWindowUserPointer(window, instance);

    glfwSetCharCallback(window, [](auto win, auto codepoint) {
        auto app = get_app_handle(win);
        app->active->insert(codepoint);
        // text_buffer.push_back(codepoint);
    });

    static constexpr auto PRESS_MASK = GLFW_PRESS | GLFW_REPEAT;
    static constexpr auto pressed_or_repeated = [](auto action) -> bool { return action & PRESS_MASK; };

    glfwSetKeyCallback(window, [](auto win, int key, int scancode, int action, int mods) {
      auto app = get_app_handle(win);

      if (pressed_or_repeated(action)) {
          switch(key) {
              case GLFW_KEY_ENTER:
                  app->active->insert('\n');
                  break;
              case GLFW_KEY_TAB:
                  app->active->insert("    ");
                  break;
          }
      }
    });

    return instance;
}

App::~App() { cleanup(); }
void App::cleanup() {}

void App::set_dimensions(int w, int h) {
    win_width = w;
    win_height = h;
    this->projection = glm::ortho(0.0f, static_cast<float>(w), 0.0f, static_cast<float>(h));
}
void App::run_loop() {
    while(this->no_close_condition()) {
        // TODO: do stuff.
        draw_all();
        glfwWaitEventsTimeout(1);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}
bool App::no_close_condition() {
    if(glfwWindowShouldClose(window)) {
        return false;
    }
    return true;
}
void App::load_file(const fs::path& file) {
    if(!fs::exists(file)) {
        PANIC("File {} doesn't exist. Forced exit.", file.string());
    }
    if(active->empty()) {
        util::println("Active text buffer is empty. Loading data into it.");
        std::string tmp;
        std::ifstream f{file};
        tmp.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        util::println("Loaded {} bytes from file {}", tmp.size(), file.string());
        active->load_string(std::move(tmp));
        assert(!active->empty());
    } else {
        PANIC("WE CAN'T HANDLE MULTIPLE FILES OR VIEWS RIGHT NOW.");
        auto bufHandle = StdStringBuffer::make_handle();
        std::string tmp;
        std::ifstream f{file};
        tmp.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        bufHandle->load_string(std::move(tmp));
        data.push_back(std::move(bufHandle));
        active = data.back().get();
    }
}
void App::draw_all() {
    for(auto& view : views) {
        view->draw();
    }
    glfwSwapBuffers(this->window);
}
void App::update_views_projections() {
    for(auto& view : views) {
        view->set_projection(projection);
    }
}
