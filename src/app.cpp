//
// Created by 46769 on 2020-12-22.
//

#include "app.hpp"
#include <glm/gtc/matrix_transform.hpp>

#define get_app_handle(window) (App *) glfwGetWindowUserPointer(window)

void framebuffer_callback(GLFWwindow *window, int width, int height) {
    fmt::print("New width: {}\t New height: {}\n", width, height);
    auto app = get_app_handle(window);
    app->set_dimensions(width, height);
    app->update_views_dimensions();
    app->update_views_projections();
    app->draw_all(true);
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
    instance->scroll = 0;
    instance->projection = glm::ortho(0.0f, static_cast<float>(app_width), 0.0f, static_cast<float>(app_height));
    instance->title = title;
    instance->window = window;

    if (!instance->window) {
        glfwTerminate();
        PANIC("Failed to create GLFW Window");
    }

    // "assets/fonts/Ubuntu-R.ttf", 24, CharacterRange{.from = 0, .to = SWEDISH_LAST_ALPHA_CHAR_UNICODE }

    FontConfig default_font_cfg{.name = "Ubuntu-R",
                                .path = "assets/fonts/Ubuntu-R.ttf",
                                .pixel_size = 24,
                                .char_range = CharacterRange{.from = 0, .to = SWEDISH_LAST_ALPHA_CHAR_UNICODE}};
    // "assets/shaders/textshader.vs", "assets/shaders/textshader.fs"
    ShaderConfig text_shader{.name = "text",
                             .vs_path = "assets/shaders/textshader.vs",
                             .fs_path = "assets/shaders/textshader.fs"};

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
    instance->active_buffer = instance->data.back().get();

    instance->load_file("main_2.cpp");

    auto v = View::create(instance->active_buffer, "main", app_width, app_height, 0, app_height);
    v->set_projection(instance->projection);
    instance->views.push_back(std::move(v));
    instance->active_view = instance->views.back().get();

    glfwSetWindowUserPointer(window, instance);

    glfwSetCharCallback(window, [](auto win, auto codepoint) {
        auto app = get_app_handle(win);
        app->active_buffer->insert(codepoint);
        // text_buffer.push_back(codepoint);
    });

    static constexpr auto PRESS_MASK = GLFW_PRESS | GLFW_REPEAT;
    static constexpr auto pressed_or_repeated = [](auto action) -> bool { return action & PRESS_MASK; };

    glfwSetKeyCallback(window, [](auto win, int key, int scancode, int action, int mods) {
        auto app = get_app_handle(win);

        if (pressed_or_repeated(action)) {
            if (mods & GLFW_MOD_CONTROL) {
                app->kb_command(key);
            } else {
                switch (key) {
                    case GLFW_KEY_ENTER:
                        app->active_buffer->insert('\n');
                        break;
                    case GLFW_KEY_TAB:
                        app->active_buffer->insert("    ");
                        break;
                    case GLFW_KEY_DOWN: {
                        auto scrolled_lines = app->scroll / app->active_view->get_font()->get_row_advance();
                        if (!(std::abs(scrolled_lines) >=
                              app->get_active_view()->get_text_buffer()->lines_count() - 1)) {
                            app->scroll -= app->active_view->get_font()->get_row_advance();
                            app->active_view->set_projection(glm::ortho(
                                    0.0f, static_cast<float>(app->win_width), static_cast<float>(app->scroll),
                                    static_cast<float>(app->win_height + app->scroll)));
                        }
                    } break;
                    case GLFW_KEY_UP:
                        auto scroll_pos = app->scroll + app->active_view->get_font()->get_row_advance();
                        app->scroll = std::min(scroll_pos, 0);
                        app->active_view->set_projection(glm::ortho(0.0f, static_cast<float>(app->win_width),
                                                                    static_cast<float>(app->scroll),
                                                                    static_cast<float>(app->win_height + app->scroll)));
                        break;
                }
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
    while (this->no_close_condition()) {
        // TODO: do stuff.
        draw_all();
        // glfwPollEvents();
        glfwWaitEventsTimeout(1);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
}
bool App::no_close_condition() { return (!glfwWindowShouldClose(window) && !exit_command_requested); }
void App::load_file(const fs::path &file) {
    if (!fs::exists(file)) { PANIC("File {} doesn't exist. Forced exit.", file.string()); }
    if (active_buffer->empty()) {
        util::println("Active text buffer is empty. Loading data into it.");
        std::string tmp;
        std::ifstream f{file};
        tmp.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        util::println("Loaded {} bytes from file {}", tmp.size(), file.string());
        active_buffer->load_string(std::move(tmp));
        assert(!active_buffer->empty());
    } else {
        PANIC("WE CAN'T HANDLE MULTIPLE FILES OR VIEWS RIGHT NOW.");
        auto bufHandle = StdStringBuffer::make_handle();
        std::string tmp;
        std::ifstream f{file};
        tmp.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        bufHandle->load_string(std::move(tmp));
        data.push_back(std::move(bufHandle));
        active_buffer = data.back().get();
    }
}

/**
 * If the application window changes dimensions, the alignment, the text placement, everything might get out of sync,
 * and to ameliorate that, one can call draw_all(true), thus forcing a recalculation of the projection matrix,
 * and upload new adjusted vertex data to the GPU, displaying the views & window correctly.
 * @param force_redraw
 */
void App::draw_all(bool force_redraw) {
    if (force_redraw) {
        for (auto &view : views) view->forced_draw();
    } else {
        for (auto &view : views) view->draw();
    }
    glfwSwapBuffers(this->window);
}
void App::update_views_projections() {
    for (auto &view : views) { view->set_projection(projection); }
}
void App::update_views_dimensions() {
    views[0]->set_dimensions(win_width, win_height);
    views[0]->set_projection(projection);
    views[0]->anchor_at(0, win_height);
}

View *App::get_active_view() const { return active_view; }

void App::kb_command(int key) {
    switch (key) {
        case GLFW_KEY_ENTER:
            active_buffer->insert('\n');
            break;
        case GLFW_KEY_TAB:
            active_buffer->insert("    ");
            break;
        case GLFW_KEY_DOWN: {
            auto scrolled_lines = scroll / active_view->get_font()->get_row_advance();
            if (!(std::abs(scrolled_lines) >= get_active_view()->get_text_buffer()->lines_count() - 1)) {
                auto scroll_factor = 10;
                scroll -= (scroll_factor * active_view->get_font()->get_row_advance());
                active_view->set_projection(glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scroll),
                                                       static_cast<float>(win_height + scroll)));
            }
        } break;
        case GLFW_KEY_UP: {
            auto scroll_factor = 10;
            auto scroll_pos = scroll + (scroll_factor * active_view->get_font()->get_row_advance());
            scroll = std::min(scroll_pos, 0);
            active_view->set_projection(glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scroll),
                                                   static_cast<float>(win_height + scroll)));
        } break;
        case GLFW_KEY_Q: {
            util::println("Ctrl+Q was pressed");
            this->graceful_exit();
        } break;
    }
}



void App::graceful_exit() {
    // TODO: ask user to save / discard unsaved changes
    // TODO: clean up GPU memory resources
    // TODO: clean up CPU memory resources
    exit_command_requested = true;
}
