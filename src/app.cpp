
// Created by 46769 on 2020-12-22.
//

#include "app.hpp"
// #include <core/commands/command_interpreter.hpp>
#include <core/commands/command_interpreter.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ranges>

#define get_app_handle(window) (App *) glfwGetWindowUserPointer(window)

static auto BUFFERS_COUNT = 0;

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

    FontConfig default_font_cfg{.name = "FreeMono",
                                .path = "assets/fonts/FreeMono.ttf",
                                .pixel_size = 24,
                                .char_range = CharacterRange{.from = 0, .to = SWEDISH_LAST_ALPHA_CHAR_UNICODE}};
    // "assets/shaders/textshader.vs", "assets/shaders/textshader.fs"
    ShaderConfig text_shader{.name = "text",
                             .vs_path = "assets/shaders/textshader.vs",
                             .fs_path = "assets/shaders/textshader.fs"};
    ShaderConfig cursor_shader{.name = "cursor",
            .vs_path = "assets/shaders/cursor.vs",
            .fs_path = "assets/shaders/cursor.fs"};
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
    ShaderLibrary::get_instance().load_shader(cursor_shader);

    auto bufHandle = StdStringBuffer::make_handle();
    BUFFERS_COUNT++;
    bufHandle->id = BUFFERS_COUNT;
    instance->data.push_back(std::move(bufHandle));
    instance->active_buffer = instance->data.back().get();

    // instance->load_file("main_2.cpp");

    auto v = View::create(instance->active_buffer, "main", app_width, app_height, 0, app_height);
    auto cv = CommandView::create("command", app_width, FontLibrary::get_default_font()->get_row_advance() + 2, 0,
                                  FontLibrary::get_default_font()->get_row_advance() + 2);
    cv->command_view->set_projection(instance->projection);
    v->set_projection(instance->projection);
    instance->views.push_back(std::move(v));
    instance->active_view = instance->views.back().get();
    instance->command_view = std::move(cv);
    glfwSetWindowUserPointer(window, instance);

    glfwSetCharCallback(window, [](auto win, auto codepoint) {
        auto app = get_app_handle(win);

        util::println("QUOTE KEY WAS PRESSED: [{}]", (char)codepoint);

        app->active_buffer->insert((char)codepoint);
        if (app->edit_command) {
            // TODO: let CommandInterpreter know to INvalidate any argument cycling of current command, and re-validate the input
            auto &ci = CommandInterpreter::get_instance();
            if (ci.has_command_waiting()) {
                auto cmd = ci.get_currently_edited_cmd()->actual_input();
                ci.destroy_current_command();
                ci.validate(app->active_buffer->to_std_string());
                ci.cycle_current_command_arguments(Cycle::Forward);
            }
        }
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
                app->handle_input(key, mods);
            }
        }
    });

    instance->update_views_projections();

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
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        draw_all();
        // glfwPollEvents();
        glfwWaitEventsTimeout(1);
    }
}
bool App::no_close_condition() { return (!glfwWindowShouldClose(window) && !exit_command_requested); }
void App::load_file(const fs::path &file) {
    util::println("Loading file into buffer {}", active_buffer->id);
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
    this->command_view->draw();
    glViewport(0, 0, this->win_width, this->win_height);
    glfwSwapBuffers(this->window);
}
void App::update_views_projections() {
    for (auto &view : views) { view->set_projection(projection); }
}
void App::update_views_dimensions() {
    views[0]->set_dimensions(win_width, win_height);
    views[0]->set_projection(projection);
    views[0]->anchor_at(0, win_height);
    command_view->w = win_width;
    command_view->command_view->set_dimensions(win_width, command_view->h);
    command_view->command_view->set_projection(projection);
}

View *App::get_active_view() const { return active_view; }

void App::kb_command(int key) {
    if (!edit_command) {
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
                    active_view->set_projection(glm::ortho(0.0f, static_cast<float>(win_width),
                                                           static_cast<float>(scroll),
                                                           static_cast<float>(win_height + scroll)));
                }
            } break;
            case GLFW_KEY_RIGHT:
                active_buffer->move_cursor(Movement::Word(1, CursorDirection::Forward));
                break;
            case GLFW_KEY_LEFT:
                active_buffer->move_cursor(Movement::Word(1, CursorDirection::Back));
                break;
            case GLFW_KEY_UP: {
                auto scroll_factor = 10;
                auto scroll_pos = scroll + (scroll_factor * active_view->get_font()->get_row_advance());
                scroll = std::min(scroll_pos, 0);
                active_view->set_projection(glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scroll),
                                                       static_cast<float>(win_height + scroll)));
            } break;
            case GLFW_KEY_Q: {
                util::println("Ctrl+Q was pressed");
                if (!active_buffer->empty()) {
                    active_buffer->clear();
                } else {
                    this->graceful_exit();
                }
            } break;
        }
    } else {
    }
}

void App::graceful_exit() {
    // TODO: ask user to save / discard unsaved changes
    // TODO: clean up GPU memory resources
    // TODO: clean up CPU memory resources
    exit_command_requested = true;
}
void App::set_input_to_command_view(bool toggleOn) {
    if(toggleOn) {
        active_buffer = command_view->input_buffer.get();
        if (!edit_command) { command_view->set_prefix("command"); }
        edit_command = toggleOn;
        command_view->active = toggleOn;
    } else {
        active_buffer->clear();
        active_buffer = active_view->get_text_buffer();
        edit_command = toggleOn;
        command_view->active = toggleOn;
        // EXECUTE INPUT COMMAND & DESTROY IT
        CommandInterpreter::get_instance().destroy_current_command();
    }
}
void App::input_char(char i) {}
void App::handle_input(int key, int modifier) {
    switch (key) {
        case GLFW_KEY_ENTER:
            if (edit_command) {
                auto cmd_input = active_buffer->to_std_string();
                active_buffer->clear();
                active_buffer = active_view->get_text_buffer();
                edit_command = false;
                // EXECUTE INPUT COMMAND & DESTROY IT

                if (CommandInterpreter::get_instance().has_command_waiting()) {
                    auto cmd = CommandInterpreter::get_instance().finalize();
                    if (cmd) cmd->exec(this);
                    CommandInterpreter::get_instance().destroy_current_command();
                } else {
                    util::println("No command waiting in list, validating before executing.");
                    CommandInterpreter::get_instance().validate(cmd_input);
                    auto cmd = CommandInterpreter::get_instance().finalize();
                    if (cmd) cmd->exec(this);
                    CommandInterpreter::get_instance().destroy_current_command();
                }
            } else {
                active_buffer->insert('\n');
            }
            break;
        case GLFW_KEY_TAB: {
            if (edit_command) {
                auto &ci = CommandInterpreter::get_instance();
                if (ci.has_command_waiting()) {
                    ci.auto_complete();
                    active_buffer->clear();
                    active_buffer->insert(ci.get_currently_edited_cmd()->as_auto_completed());
                } else {
                    ci.validate(active_buffer->to_std_string());
                    ci.cycle_current_command_arguments(Cycle::Forward);
                }
            } else {
                active_buffer->insert("    ");
            }
            break;
        }
        case GLFW_KEY_DOWN: {
            if (edit_command) {
                auto &ci = CommandInterpreter::get_instance();
                if (ci.has_command_waiting()) {
                    ci.cycle_current_command_arguments(static_cast<Cycle>(key));
                } else {
                    ci.validate(active_buffer->to_std_string());
                    ci.cycle_current_command_arguments(static_cast<Cycle>(key));
                }
            } else {
                auto scrolled_lines = scroll / active_view->get_font()->get_row_advance();
                if (!(std::abs(scrolled_lines) >= get_active_view()->get_text_buffer()->lines_count() - 1)) {
                    scroll -= active_view->get_font()->get_row_advance();
                    active_view->set_projection(glm::ortho(0.0f, static_cast<float>(win_width),
                                                           static_cast<float>(scroll),
                                                           static_cast<float>(win_height + scroll)));
                }
            }
        } break;
        case GLFW_KEY_UP: {
            if (!edit_command) {
                auto scroll_pos = scroll + active_view->get_font()->get_row_advance();
                scroll = std::min(scroll_pos, 0);
                active_view->set_projection(glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scroll),
                                                       static_cast<float>(win_height + scroll)));
            } else {
                auto &ci = CommandInterpreter::get_instance();
                if (ci.has_command_waiting()) {
                    ci.cycle_current_command_arguments(static_cast<Cycle>(key));
                } else {
                    ci.validate(active_buffer->to_std_string());
                    ci.cycle_current_command_arguments(static_cast<Cycle>(key));
                }
            }
        } break;
        case GLFW_KEY_RIGHT:
            active_buffer->move_cursor(Movement::Char(1, CursorDirection::Forward));
            break;
        case GLFW_KEY_LEFT:
            active_buffer->move_cursor(Movement::Char(1, CursorDirection::Back));
            break;
        case GLFW_KEY_ESCAPE:
            set_input_to_command_view(!edit_command);
            break;
        case GLFW_KEY_BACKSPACE:
            active_buffer->remove(Movement::Char(1, CursorDirection::Back));
            if (edit_command) {
                auto &ci = CommandInterpreter::get_instance();
                ci.destroy_current_command();
            }
            break;
    }
    command_view->active = edit_command;
}