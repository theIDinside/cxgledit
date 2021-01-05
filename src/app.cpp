
// Created by 46769 on 2020-12-22.
//

#include "app.hpp"
#include <core/data_manager.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ranges>
#include <utility>

#define get_app_handle(window) (App *) glfwGetWindowUserPointer(window)

static auto BUFFERS_COUNT = 0;

void framebuffer_callback(GLFWwindow *window, int width, int height) {
    fmt::print("New width: {}\t New height: {}\n", width, height);
    auto app = get_app_handle(window);
    auto dim = app->get_window_dimension();
    float wratio = float(width) / float(dim.w);
    float hratio = float(height) / float(dim.h);

    app->set_dimensions(width, height);
    app->update_views_dimensions(wratio, hratio);
    app->draw_all(true);
    glViewport(0, 0, width, height);
}

App *App::initialize(int app_width, int app_height, const std::string &title) {
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
    App::win_dimensions = WindowDimensions{app_width, app_height};

    instance->root_layout = new Layout{};
    instance->root_layout->parent = nullptr;

    instance->root_layout->dimInfo.w = App::win_dimensions.w;
    instance->root_layout->dimInfo.h = App::win_dimensions.h;
    instance->root_layout->dimInfo.x = 0;
    instance->root_layout->dimInfo.y = App::win_dimensions.h;
    instance->root_layout->id = 1;

    instance->scroll = 0;

    instance->projection = glm::ortho(0.0f, static_cast<float>(app_width), 0.0f, static_cast<float>(app_height));
    instance->title = title;
    instance->window = window;

    if (!instance->window) {
        glfwTerminate();
        PANIC("Failed to create GLFW Window");
    }

    FontConfig default_font_cfg{.name = "FreeMono",
                                .path = "assets/fonts/FreeMono.ttf",
                                .pixel_size = 24,
                                .char_range = CharacterRange{.from = 0, .to = SWEDISH_LAST_ALPHA_CHAR_UNICODE}};
    ShaderConfig text_shader{.name = "text",
                             .vs_path = "assets/shaders/textshader.vs",
                             .fs_path = "assets/shaders/textshader.fs"};
    ShaderConfig cursor_shader{.name = "cursor",
                               .vs_path = "assets/shaders/cursor.vs",
                               .fs_path = "assets/shaders/cursor.fs"};
    FontLibrary::get_instance().load_font(default_font_cfg);
    ShaderLibrary::get_instance().load_shader(text_shader);
    ShaderLibrary::get_instance().load_shader(cursor_shader);

    auto text_row_advance = FontLibrary::get_default_font()->get_row_advance() + 2;

    auto sb_height = text_row_advance;

    auto cv = CommandView::create("command", app_width, text_row_advance * 1, 0, text_row_advance * 1);
    cv->command_view->set_projection(instance->projection);

    instance->command_view = std::move(cv);

    instance->new_editor_window();
    instance->active_buffer->load_string("FOO");
    instance->new_editor_window(SplitStrategy::VerticalSplit);
    instance->active_buffer->load_string("BAR");
    instance->new_editor_window(SplitStrategy::VerticalSplit);
    instance->active_buffer->load_string("BAZ");

    glfwSetWindowUserPointer(window, instance);
    glfwSetCharCallback(window, [](auto window, auto codepoint) {
        auto app = get_app_handle(window);
        app->active_buffer->insert((char) codepoint);
        app->command_view->show_last_message = false;
        if (app->command_edit) {
            // TODO: let CommandInterpreter know to INvalidate any argument cycling of current command, and re-validate the input
            auto &ci = CommandInterpreter::get_instance();
            if (ci.has_command_waiting()) {
                auto cmd = ci.get_currently_edited_cmd()->actual_input();
                ci.destroy_current_command();
                ci.validate(app->active_buffer->to_std_string());
                ci.cycle_current_command_arguments(Cycle::Forward);
            }
        }
    });

    glfwSetMouseButtonCallback(window, [](auto window, auto button, auto action, auto mods) {
        double xpos, ypos;
        auto app = get_app_handle(window);
        glfwGetCursorPos(window, &xpos, &ypos);
        auto &[w, h] = App::win_dimensions;
        util::println("Mouse click: {}, {} inside [{} x {}]", xpos, ypos, w, h);

        auto translate_y = h - (int) ypos;
        auto layout = find_within_leaf_node(app->root_layout, int(xpos), translate_y);
        auto ew = std::find_if(app->editor_views.begin(), app->editor_views.end(), [id = layout->id](auto ew) {
            return ew->ui_layout_id == id;
        });

        if(ew != std::end(app->editor_views)) {
            EditorWindow* elem = *ew;
            app->editor_win_selected(elem);
        }
    });

    static constexpr auto PRESS_MASK = GLFW_PRESS | GLFW_REPEAT;
    static constexpr auto pressed_or_repeated = [](auto action) -> bool { return action & PRESS_MASK; };

    glfwSetKeyCallback(window, [](auto window, int key, int scancode, int action, int mods) {
        auto app = get_app_handle(window);
        if (pressed_or_repeated(action)) {
            if (mods & GLFW_MOD_CONTROL) {
                app->kb_command(key);
            } else {
                app->handle_edit_input(key, mods);
            }
        }
    });

    return instance;
}

void App::editor_win_selected(EditorWindow *elem) {
    active_window->active = false;
    editor_views.erase(std::find(editor_views.begin(), editor_views.end(), elem));
    editor_views.push_back(elem);
    active_window = editor_views.back();
    active_buffer = editor_views.back()->get_text_buffer();
    active_view = active_window->view;
    active_window->active = true;
}

App::~App() { cleanup(); }
void App::cleanup() {}

void App::set_dimensions(int w, int h) {
    this->win_width = w;
    this->win_height = h;
    root_layout->dimInfo.y = h;
    win_dimensions = WindowDimensions{w, h};
    this->projection = glm::ortho(0.0f, static_cast<float>(w), 0.0f, static_cast<float>(h));
}
void App::run_loop() {
    while (this->no_close_condition()) {
        // TODO: do stuff.
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        draw_all();
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
    }
}

/**
 * If the application window changes dimensions, the alignment, the text placement, everything might get out of sync,
 * and to ameliorate that, one can call draw_all(true), thus forcing a recalculation of the projection matrix,
 * and upload new adjusted vertex data to the GPU, displaying the views & window correctly.
 * @param force_redraw
 */
void App::draw_all(bool force_redraw) {
    for (auto &ew : editor_views) ew->draw(force_redraw);
    this->command_view->draw();
    glViewport(0, 0, this->win_width, this->win_height);
    glfwSwapBuffers(this->window);
}

void App::update_views_dimensions(float wRatio, float hRatio) {
    update_layout_tree(root_layout, wRatio, hRatio);
    util::println("New root dimension: {}, {}", root_layout->dimInfo.w, root_layout->dimInfo.h);

    for (auto e : editor_views) {
        auto dim = e->dimInfo;
        auto scaledDim = scale(dim, wRatio, hRatio);
        auto ew_layout = find_by_id(root_layout, e->ui_layout_id);

        /* TODO: this is what it SHOULD look like, since update_layout_tree actually updates all layouts
         *
         */
        e->update_layout(ew_layout->dimInfo);
        // e->update_layout(scaledDim);
        e->set_projection(projection);
    }

    command_view->w = win_width;
    command_view->command_view->set_dimensions(win_width, command_view->h);
    command_view->command_view->set_projection(glm::ortho(0.0f, float(win_width), 0.0f, float(win_height)));
}

View *App::get_active_view() const { return active_view; }

void App::kb_command(int key) {
    if (!command_edit) {
        switch (key) {
            case GLFW_KEY_ENTER:
                active_buffer->insert('\n');
                break;
            case GLFW_KEY_TAB:
                active_buffer->insert_str("    ");
                break;
            case GLFW_KEY_UP:
            case GLFW_KEY_DOWN: {
                auto scrolled_lines = active_view->scrolled / active_view->get_font()->get_row_advance();
                if (!(std::abs(scrolled_lines) >= get_active_view()->get_text_buffer()->lines_count() - 2)) {
                    active_view->scroll(static_cast<Scroll>(key), 3);
                }
            } break;
            case GLFW_KEY_RIGHT:
                active_buffer->move_cursor(Movement::Word(1, CursorDirection::Forward));
                break;
            case GLFW_KEY_LEFT:
                active_buffer->move_cursor(Movement::Word(1, CursorDirection::Back));
                break;
            case GLFW_KEY_DELETE:
                active_buffer->remove(Movement::Word(1, CursorDirection::Forward));
                break;
            case GLFW_KEY_BACKSPACE:
                active_buffer->remove(Movement::Word(1, CursorDirection::Back));
                break;
            case GLFW_KEY_Q: {
                util::println("Ctrl+Q was pressed");
                auto active_buf = active_view->get_text_buffer();
                if (!active_buf->empty()) {
                    auto id = active_buf->id;
                    DataManager::get_instance().request_close(id);
                    if (editor_views.back() == active_window) {
                        assert(active_window->view->td_id == id);
                        editor_views.pop_back();
                        delete active_window;
                        active_window = editor_views.back();
                        active_window->active = true;
                    } else {
                        PANIC("Somehow non-active window was deleted");
                    }

                    auto layout = find_by_id(root_layout, active_window->ui_layout_id);
                    if (layout == layout->parent->left) {
                        if (layout->parent == root_layout) {
                            auto new_root = layout->parent->right;
                            set_new_root(root_layout, new_root);
                        } else {
                            promote_node(layout->parent->right);
                        }
                    } else {
                        if (layout->parent == root_layout) {
                            auto new_root = layout->parent->left;
                            set_new_root(root_layout, new_root);
                        } else {
                            promote_node(layout->parent->left);
                        }
                    }
                    active_view = active_window->view;
                    active_buffer = active_view->get_text_buffer();
                    update_all_editor_windows();

                    dump_layout_tree(root_layout);
                    draw_all(true);
                } else {
                    this->graceful_exit();
                }
            } break;
            case GLFW_KEY_N: {
                auto current_window = active_window;
                current_window->active = false;
                rotate_container(editor_views, 1);
                active_window = editor_views.back();
                active_window->active = true;
                active_buffer = editor_views.back()->get_text_buffer();
                active_view = active_window->view;
                break;
            }
            case GLFW_KEY_D:
                print_debug_info();
                break;
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
    if (toggleOn) {
        util::println("Pushing {} to active view stack order", command_view->command_view->name);
        active_buffer = command_view->command_view->get_text_buffer();
        active_buffer->clear();
        if (!command_edit) { command_view->set_prefix("command"); }
        command_edit = toggleOn;
        command_view->active = toggleOn;
    } else {
        active_buffer->clear();
        active_buffer = active_view->get_text_buffer();
        util::println("Buffer id: {}", active_buffer->id);
        command_edit = toggleOn;
        command_view->active = toggleOn;
        // EXECUTE INPUT COMMAND & DESTROY IT
        CommandInterpreter::get_instance().destroy_current_command();
    }
}
void App::input_char(char i) {}
void App::handle_edit_input(int key, int modifier) {
    switch (key) {
        case GLFW_KEY_HOME:
            active_buffer->step_to_line_begin(Boundary::Inside);
            break;
        case GLFW_KEY_END:
            active_buffer->step_to_line_end(Boundary::Outside);
            break;
        case GLFW_KEY_ENTER:
            this->input_command_or_newline();
            break;
        case GLFW_KEY_TAB: {
            if (command_edit) {
                auto &ci = CommandInterpreter::get_instance();
                if (ci.has_command_waiting()) {
                    ci.auto_complete();
                    active_buffer->clear();
                    active_buffer->insert_str(ci.get_currently_edited_cmd()->as_auto_completed());
                } else {
                    ci.validate(active_buffer->to_std_string());
                    ci.cycle_current_command_arguments(Cycle::Forward);
                }
            } else {
                active_buffer->insert_str("    ");
            }
            break;
        }
        case GLFW_KEY_DOWN:
        case GLFW_KEY_UP:
            cycle_command_or_move_cursor(static_cast<Cycle>(key));
            break;
        case GLFW_KEY_RIGHT:
            active_buffer->move_cursor(Movement::Char(1, CursorDirection::Forward));
            break;
        case GLFW_KEY_LEFT:
            active_buffer->move_cursor(Movement::Char(1, CursorDirection::Back));
            break;
        case GLFW_KEY_ESCAPE:
            set_input_to_command_view(!command_edit);
            break;
        case GLFW_KEY_BACKSPACE:
            active_buffer->remove(Movement::Char(1, CursorDirection::Back));
            if (command_edit) {
                auto &ci = CommandInterpreter::get_instance();
                ci.destroy_current_command();
            }
            break;
    }
    command_view->active = command_edit;
}
void App::set_error_message(std::string msg) {
    command_view->command_view->get_text_buffer()->clear();
    command_view->command_view->get_text_buffer()->insert_str(command_view->last_message);
    command_view->show_last_message = true;
    command_view->active = false;
}

void App::input_command_or_newline() {
    if (command_edit) {
        auto cmd_input = active_buffer->to_std_string();
        active_buffer->clear();
        active_buffer = active_view->get_text_buffer();

        // EXECUTE INPUT COMMAND & DESTROY IT
        if (CommandInterpreter::get_instance().has_command_waiting()) {
            auto cmd = CommandInterpreter::get_instance().finalize();
            if (cmd) cmd->exec(this, active_view->get_text_buffer());
            CommandInterpreter::get_instance().destroy_current_command();
        } else {
            CommandInterpreter::get_instance().validate(cmd_input);
            auto cmd = CommandInterpreter::get_instance().finalize();
            if (cmd) {
                if (!cmd->exec(this, active_view->get_text_buffer())) {
                    auto &ci = CommandInterpreter::get_instance();
                    auto error_cmd = dynamic_cast<ErrorCommand *>(ci.get_currently_edited_cmd());
                    if (error_cmd) {
                        error_cmd->exec(this, command_view->input_buffer.get());
                    } else {
                        if (ci.has_command_waiting()) {
                            ci.auto_complete();
                            active_buffer->clear();
                            active_buffer->insert_str(ci.get_currently_edited_cmd()->as_auto_completed());
                        }
                    }
                }
            }
            CommandInterpreter::get_instance().destroy_current_command();
        }
        /// Setting active input to text editor again
        active_buffer = active_view->get_text_buffer();
        command_edit = false;
    } else {
        active_buffer->insert('\n');
    }
}

void App::cycle_command_or_move_cursor(Cycle cycle) {
    auto curs_direction = cycle == Cycle::Forward ? CursorDirection::Forward : CursorDirection::Back;
    if (command_edit) {
        auto &ci = CommandInterpreter::get_instance();
        if (ci.has_command_waiting()) {
            ci.cycle_current_command_arguments(cycle);
        } else {
            ci.validate(active_buffer->to_std_string());
            ci.cycle_current_command_arguments(cycle);
        }
    } else {
        active_buffer->move_cursor(Movement::Line(1, curs_direction));
    }
}

void App::print_debug_info() {
    util::println("----- View Debug Info -----");
    util::println("Views currently open: {}", editor_views.size());
    assert(active_window->view == active_view);
    util::println("Active window: {} - Active view name {} - Buffer id: {} - UI ID: {}", active_window->ui_layout_id,
                  active_view->name, active_buffer->id, active_window->ui_layout_id);
    active_window->status_bar->print_debug_info();
    active_buffer->print_cursor_info();
    active_buffer->print_line_meta_data();
    fmt::print("\n");
    util::println("---- Layout tree (negative id's are branch nodes, positive leaf nodes ---- ");
    dump_layout_tree(root_layout);
    util::println("---------------------------\n");
}
WindowDimensions App::get_window_dimension() { return win_dimensions; }

WindowDimensions App::win_dimensions{0, 0};

static auto layout_id = 1;

void App::new_editor_window(SplitStrategy splitStrategy) {
    if (splitStrategy == SplitStrategy::VerticalSplit) {
        auto active_layout_id = active_window->ui_layout_id;
        auto l = find_by_id(root_layout, active_layout_id);
        push_node(l, layout_id, LayoutType::Horizontal);
        auto active_editor_win = active_window;
        auto ew = EditorWindow::create({}, projection, layout_id, l->right->dimInfo);
        ew->view->set_projection(projection);
        ew->status_bar->ui_view->set_projection(projection);
        editor_views.push_back(ew);
        auto &e = editor_views.back();
        active_buffer = e->get_text_buffer();
        active_view = e->view;
        active_window = editor_views.back();
        active_editor_win->update_layout(l->left->dimInfo);
    } else {
        auto ew = EditorWindow::create({}, projection, layout_id, DimInfo{0, win_height, win_width, win_height});
        ew->view->set_projection(projection);
        ew->status_bar->ui_view->set_projection(projection);
        editor_views.push_back(ew);
        auto &e = editor_views.back();
        active_buffer = e->get_text_buffer();
        active_view = e->view;
    }
    active_window = editor_views.back();
    util::println("Editor window created");
    layout_id++;
}

/*
 *
 */

void App::update_all_editor_windows() {
    update_layout_tree(root_layout, 1.0, 1.0);
    for (auto e : editor_views) {
        auto dim = e->dimInfo;
        auto ew_layout = find_by_id(root_layout, e->ui_layout_id);
        if (ew_layout == nullptr) {
            util::println("Could not find {}", e->ui_layout_id);
            dump_layout_tree(root_layout);
        }
        util::println("Updating EW DimInfo for {}: {} \t -> \t {}", e->ui_layout_id, dim.debug_str(),
                      ew_layout->dimInfo.debug_str());
        e->update_layout(ew_layout->dimInfo);
        e->set_projection(projection);
    }
}

