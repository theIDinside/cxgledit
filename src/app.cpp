
// Created by 46769 on 2020-12-22.
//

#include "app.hpp"
#include <core/data_manager.hpp>
#include <ui/editor_window.hpp>
#include <ui/status_bar.hpp>
#include <ui/view.hpp>
#include <ui/core/opengl.hpp>
#include <utils/fileutil.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <ranges>
#include <utility>

#define get_app_handle(window) (App *) glfwGetWindowUserPointer(window)

static auto BUFFERS_COUNT = 0;

using ui::CommandView;
using ui::EditorWindow;
using ui::View;
using ui::core::DimInfo;
using ui::core::Layout;

/// FIXME(urgent): When window is resized slowly, there seems to be a big within GLFW
///  because it doesn't update correctly and starts stretching my internal data layout for whatever
///  reason. When I min/max it works, when I resize with a fast pull on an edge, no problems show up
///  but if one slowly pulls the edge, problems pop up. Looks horrible. It could very well have something
///  to do with how I resize in this function (a ratio) which doesn't update correctly between
///  subsequent calls to framebuffer_callback
void framebuffer_callback(GLFWwindow *window, int width, int height) {
    fmt::print("New width: {}\t New height: {}\n", width, height);
    // if w & h == 0, means we minimized. Do nothing. Because when we un-minimize, means we restore the prior size
    if (width != 0 && height != 0) {
        auto app = get_app_handle(window);
        float wratio = float(width) / float(app->win_width);
        float hratio = float(height) / float(app->win_height);
        app->set_dimensions(width, height);
        app->update_views_dimensions(wratio, hratio);
        glViewport(0, 0, width, height);
    }
}

auto size_changed_callback(GLFWwindow *window, int width, int height) { framebuffer_callback(window, width, height); }

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
    glfwSetWindowSizeCallback(window, size_changed_callback);
    glfwSetWindowRefreshCallback(window, [](auto window) {
        auto app = get_app_handle(window);
        app->draw_all(true);
    });
    if (not gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) { PANIC("Failed to initialize GLAD\n"); }

    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        util::println("Enabled debug output for OpenGL");
    }

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
    auto cv = CommandView::create("command", app_width, text_row_advance * 1, 0, text_row_advance * 1);
    cv->command_view->set_projection(instance->projection);

    instance->command_view = std::move(cv);

    // TODO: remove these, these are just for simplicity when testing UI
    instance->new_editor_window();
    // instance->load_file("main.cpp");

    auto char_input_callback = [](auto window, auto codepoint) {
        auto app = get_app_handle(window);
        util::println("Char value: {}", codepoint);
        // TODO(feature, major, huge maybe): Unicode support. But why would we want that? Source code should and can only use ASCII. If you plan on using something else? Well fuck off then.
        if (codepoint >= 32 && codepoint <= 126) {
            app->active_buffer->insert((char) codepoint);
            app->command_view->show_last_message = false;
            if (app->command_edit) {
                // TODO: let CommandInterpreter know to INvalidate any argument cycling of current command, and re-validate the input
                auto &ci = CommandInterpreter::get_instance();
                ci.evaluate_current_input();
            }
        }
    };
    glfwSetWindowUserPointer(window, instance);
    glfwSetCharCallback(window, char_input_callback);

    glfwSetMouseButtonCallback(window, [](auto window, auto button, auto action, auto mods) {
        double xpos, ypos;
        auto app = get_app_handle(window);
        glfwGetCursorPos(window, &xpos, &ypos);
        auto &[w, h] = App::win_dimensions;

        auto translate_y = h - (int) ypos;
        auto layout = find_within_leaf_node(app->root_layout, int(xpos), translate_y);
        if (layout) {
            auto ew = std::find_if(app->editor_views.begin(), app->editor_views.end(),
                                   [id = layout->id](auto ew) { return ew->ui_layout_id == id; });

            if (ew != std::end(app->editor_views)) {
                EditorWindow *elem = *ew;
                app->editor_win_selected(elem);
                auto x = AS(xpos, int);
                auto y = AS(ypos, int);
                elem->handle_click(x, y);
            }
        } else {
            PANIC("MOUSE CLICKING FEATURES NOT YET DONE GOOD.");
        }
    });

    static constexpr auto PRESS_MASK = GLFW_PRESS | GLFW_REPEAT;
    static constexpr auto pressed_or_repeated = [](auto action) -> bool { return action & PRESS_MASK; };

    glfwSetKeyCallback(window, [](auto window, int key, int scancode, int action, int mods) {
        auto app = get_app_handle(window);
        // app->command_view->show_last_message = false;
        if (pressed_or_repeated(action)) {
            if (mods & GLFW_MOD_CONTROL) {
                app->kb_command(key);
            } else {
                app->handle_edit_input(key, mods);
            }
        }
    });

    auto &ci = CommandInterpreter::get_instance();
    ci.register_application(instance);

    return instance;
}

void App::editor_win_selected(ui::EditorWindow *elem) {
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
    util::println("New dimension set to {} x {}", w, h);
}
void App::run_loop() {
    double nowTime = glfwGetTime();
    auto since_last_update = glfwGetTime();
    while (this->no_close_condition()) {
        // TODO: do stuff.
        nowTime = glfwGetTime();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        draw_all();
        glfwWaitEventsTimeout(1);
        if ((nowTime - since_last_update) >= 3) {
            since_last_update = nowTime;
            active_window->get_text_buffer()->rebuild_metadata();
        }
    }
}
bool App::no_close_condition() { return (!glfwWindowShouldClose(window) && !exit_command_requested); }
void App::load_file(const fs::path &file) {
    if (!fs::exists(file)) { PANIC("File {} doesn't exist. Forced exit.", file.string()); }
    if (active_window->view->get_text_buffer()->empty()) {
        std::string tmp;
        std::ifstream f{file};
        tmp.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        active_window->get_text_buffer()->load_string(std::move(tmp));
        active_window->get_text_buffer()->set_file(file);
        assert(!active_buffer->empty());
    } else {
        new_editor_window(SplitStrategy::VerticalSplit);
        std::string tmp;
        std::ifstream f{file};
        tmp.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
        active_window->get_text_buffer()->set_string(tmp);
        active_window->get_text_buffer()->load_string(std::move(tmp));
        active_window->get_text_buffer()->set_file(file);
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
        auto ew_layout = find_by_id(root_layout, e->ui_layout_id);
        e->update_layout(ew_layout->dimInfo);
        e->set_projection(projection);
    }

    command_view->w = win_width;
    command_view->command_view->set_dimensions(win_width, command_view->h);
    command_view->command_view->set_projection(glm::ortho(0.0f, float(win_width), 0.0f, float(win_height)));
}

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
                active_view->scroll(static_cast<ui::Scroll>(key), 3);
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
                auto active_buf = active_view->get_text_buffer();
                if (!active_buf->empty()) {
                    auto id = active_buf->id;
                    DataManager::get_instance().request_close(id);

                    auto layoutToDestroy = find_by_id(root_layout, active_window->ui_layout_id);
                    if (layoutToDestroy == layoutToDestroy->parent->left) {
                        if (layoutToDestroy->parent == root_layout) {
                            auto new_root = layoutToDestroy->parent->right;
                            set_new_root(root_layout, new_root);
                        } else {
                            promote_node(layoutToDestroy->parent->right);
                        }
                    } else {
                        if (layoutToDestroy->parent == root_layout) {
                            auto new_root = layoutToDestroy->parent->left;
                            set_new_root(root_layout, new_root);
                        } else {
                            promote_node(layoutToDestroy->parent->left);
                        }
                    }

                    if (editor_views.back() == active_window) {
                        assert(active_window->view->td_id == id);
                        editor_views.pop_back();
                        delete active_window;
                        active_window = editor_views.back();
                        active_window->active = true;
                    } else {
                        PANIC("Somehow non-active window was deleted");
                    }

                    active_view = active_window->view;
                    active_buffer = active_view->get_text_buffer();
                    update_all_editor_windows();
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
            case GLFW_KEY_C: { // cmdcopy
                if (active_buffer->mark_set) {
                    auto view = active_buffer->copy_range(active_buffer->get_cursor_rect());
                    copy_register.push_view(view);
                }
            } break;
            case GLFW_KEY_V: { // cmdpaste
                auto data = copy_register.get_last();
                if(data) {
                    active_buffer->insert_str(*data);
                }
            } break;
            case GLFW_KEY_G:
                command_input("goto", Commands::GotoLine);
                break;
            case GLFW_KEY_O:
                command_input("open", Commands::OpenFile);
                break;
            case GLFW_KEY_D:
                print_debug_info();
                break;
            case GLFW_KEY_W:
                command_input("write", Commands::WriteFile);
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

void App::command_input(const std::string &prefix, Commands commandInput) {
    auto &ci = CommandInterpreter::get_instance();
    ci.set_current_command_read(commandInput);
    active_buffer = command_view->command_view->get_text_buffer();
    active_buffer->clear();
    if (!command_edit) {
        command_view->set_prefix(prefix);
        active_buffer->insert_str(ci.current_input());
    }
    command_edit = true;
    command_view->active = true;
}

void App::disable_command_input() {
    auto &ci = CommandInterpreter::get_instance();
    ci.clear_state();
    command_view->command_view->get_text_buffer()->clear();
    active_buffer = active_window->get_text_buffer();
    active_view = active_window->view;
    command_edit = false;
    command_view->active = false;
}

void App::handle_edit_input(int key, int modifier) {

    switch (key) {
        case GLFW_KEY_HOME:
            active_buffer->step_to_line_begin(Boundary::Inside);
            break;
        case GLFW_KEY_END:
            active_buffer->step_to_line_end(Boundary::Outside);
            break;
        case GLFW_KEY_ENTER:
            input_command_or_newline();
            break;
        case GLFW_KEY_TAB: {
            if (command_edit) {
                CommandInterpreter::get_instance().auto_complete();
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
            if (modifier & GLFW_MOD_SHIFT) {
                if (not active_buffer->mark_set) { active_buffer->set_mark_at_cursor(); }
            } else {
                if (active_buffer->mark_set) { active_buffer->clear_marks(); }
            }
            active_buffer->move_cursor(Movement::Char(1, CursorDirection::Forward));
            break;
        case GLFW_KEY_LEFT:
            if (modifier & GLFW_MOD_SHIFT) {
                if (not active_buffer->mark_set) { active_buffer->set_mark_at_cursor(); }
            } else {
                if (active_buffer->mark_set) { active_buffer->clear_marks(); }
            }
            active_buffer->move_cursor(Movement::Char(1, CursorDirection::Back));
            break;
        case GLFW_KEY_ESCAPE:
            disable_command_input();
            break;
        case GLFW_KEY_BACKSPACE:
            active_buffer->remove(Movement::Char(1, CursorDirection::Back));
            if (command_edit) {
                auto &ci = CommandInterpreter::get_instance();
                ci.evaluate_current_input();
            }
            break;
        case GLFW_KEY_DELETE:
            active_buffer->remove(Movement::Char(1, CursorDirection::Forward));
            if (command_edit) {
                auto &ci = CommandInterpreter::get_instance();
                ci.evaluate_current_input();
            }
            break;
        case GLFW_KEY_INSERT:
        case GLFW_KEY_PAGE_UP:
        case GLFW_KEY_PAGE_DOWN:
            break;
    }
    // N.B. this was moved from charCallback because typing in ÖÄÅ right now, crashes the application as the textual data lives inside a std::string
    // which gets passed around through layers of structures, and these characters can't be represented as single bytes.
    command_view->active = command_edit;
}

void App::set_error_message(const std::string &msg) {
    command_view->last_message = msg;
    command_view->command_view->get_text_buffer()->clear();
    command_view->command_view->get_text_buffer()->insert_str(msg);
    command_view->show_last_message = true;
    command_view->active = false;
}

void App::input_command_or_newline() {
    if (command_edit) {
        CommandInterpreter::get_instance().execute_command();
        /// Setting active input to text editor again
        active_buffer = active_window->get_text_buffer();
        active_view = active_window->view;
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
        if (active_buffer->mark_set) { active_buffer->clear_marks(); }
    }
}

void App::print_debug_info() {
    util::println("----- App Debug Info -----");
    util::println("Window Size: {} x {}", win_width, win_height);
    util::println("---------------------------");

    util::println("----- Buffer contents -----");
    util::println("{}", active_buffer->to_string_view());
    util::println("---------------------------");
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
        active_window->active = false;
        auto active_layout_id = active_window->ui_layout_id;
        auto l = find_by_id(root_layout, active_layout_id);
        push_node(l, layout_id, ui::core::LayoutType::Horizontal);
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
        active_window->active = true;
    } else {
        auto ew = EditorWindow::create({}, projection, layout_id, DimInfo{0, win_height, win_width, win_height});
        ew->view->set_projection(projection);
        ew->status_bar->ui_view->set_projection(projection);
        editor_views.push_back(ew);
        auto &e = editor_views.back();
        active_buffer = e->get_text_buffer();
        active_view = e->view;
        ew->active = true;
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
            PANIC("This must not fail");
        } else {
            util::println("Updating EW DimInfo for {}: {} \t -> \t {}", e->ui_layout_id, dim.debug_str(),
                          ew_layout->dimInfo.debug_str());
            e->update_layout(ew_layout->dimInfo);
            e->set_projection(projection);
        }
    }
}

ui::CommandView *App::get_command_view() const { return command_view.get(); }

void App::editor_window_goto(int line) {
    util::println("Goto line {}", line);
    auto l = active_view->get_cursor()->line;

    auto buf = active_view->get_text_buffer();
    if (line > buf->meta_data.line_begins.size()) {
        buf->step_cursor_to(buf->size());
        auto scroll_amt = buf->meta_data.line_begins.size() - l;
        active_view->scroll(ui::Scroll::Down, scroll_amt);
    } else {
        auto pos = buf->meta_data.line_begins[line];
        buf->step_cursor_to(pos);
        if (l < line) {
            auto scroll_amt = line - l;
            active_view->scroll(ui::Scroll::Down, scroll_amt);
        } else {
            auto scroll_amt = l - line;
            active_view->scroll(ui::Scroll::Up, scroll_amt);
        }
    }
}
void App::restore_input() {
    active_view = active_window->view;
    active_buffer = active_view->get_text_buffer();
    command_view->input_buffer->clear();
}
ui::EditorWindow *App::get_active_window() const { return active_window; }
void App::fwrite_active_to_disk(const std::string &path) {
    auto p = fs::path{path};
    // this is just wrapped for now, since the if/else branch are identical
    auto write_impl = [&](auto path) {
        auto buf_view = active_window->get_text_buffer()->to_string_view();
        auto bytes_written = sv_write_file(p, buf_view);
        if (bytes_written) {// success
            // TODO: this could be printed on the command view
            util::println("Wrote {} bytes to file {}", bytes_written.value(), path.string());
        } else {
            util::println("Could not retrieve file size");
        }
    };

    if (not fs::exists(p)) {
        write_impl(p);
    } else {
        // possibly request a "y/N" from the user, for now we just write to disk
        write_impl(p);
    }
}

void Register::push_view(std::string_view data) {
    if (not copies.empty()) [[likely]] {
        auto last = copies.back();
        auto taken = (last.begin + last.len) + data.size();
        if (taken > store.capacity()) { store.reserve(taken * 2); }
        auto begin = last.begin + last.len;
        auto len = data.size();
        std::memcpy(store.data() + begin, data.data(), data.size());
        copies.emplace_back(begin, len);
    } else [[unlikely]] {
        if (data.size() > store.capacity()) { store.reserve(data.size() * 2); }
        std::memcpy(store.data(), data.data(), data.size());
        copies.emplace_back(0, data.size());
    }
}
std::optional<std::string_view> Register::get(std::size_t index) {
    if (index < copies.size()) {
        auto copy = copies[index];

        return std::string_view{store.data() + copy.begin, copy.len};
    } else {
        return {};
    }
}
std::optional<std::string_view> Register::get_last() {
    if (copies.empty()) return {};
    return get(copies.size() - 1);
}
