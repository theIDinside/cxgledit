//
// Created by 46769 on 2020-12-22.
//

#include <glm/gtc/matrix_transform.hpp>

#include "view.hpp"
#include <utility>
#include <vector>

#include <app.hpp>
#include <core/commands/command_interpreter.hpp>
#include <core/data_manager.hpp>
#include <core/text_data.hpp>
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>

/// ----------------- VIEW FACTORY FUNCTIONS -----------------

std::unique_ptr<View> View::create_managed(TextData *data, const std::string &name, int w, int h, int x, int y,
                                           ViewType type) {
    auto reserveMemory =
            gpu_mem_required_for_quads<TextVertex>(1024);// reserve GPU memory for at least 1024 characters.
    if (!data->empty()) { reserveMemory = gpu_mem_required_for_quads<TextVertex>(data->size()); }
    auto vao = VAO::make(GL_ARRAY_BUFFER, reserveMemory);
    auto v = std::make_unique<View>();
    v->td_id = data->id;
    v->td_id = data->id;
    v->type = type;
    v->width = w;
    v->height = h;
    v->x = x;
    v->y = y;
    v->font = FontLibrary::get_default_font();
    v->shader = ShaderLibrary::get_text_shader();
    v->vao = std::move(vao);
    v->data = data;
    v->vertexCapacity = reserveMemory / sizeof(TextVertex);
    v->cursor = ViewCursor::create_from(v);
    v->name = name;

    return v;
}
std::unique_ptr<CommandView> CommandView::create(const std::string &name, int width, int height, int x, int y) {
    auto cview = std::make_unique<CommandView>();
    cview->name = name;
    cview->x = x;
    cview->y = y;
    cview->w = width;
    cview->h = height;
    auto input_buffer = StdStringBuffer::make_handle();
    input_buffer->id = (int) std::hash<std::string>{}("CommandView");

    cview->input_buffer = std::move(input_buffer);
    util::println("Placing command view at {}, {} with dimensions {}, {}", x, y, width, height);
    auto v = View::create_managed(cview->input_buffer.get(), name, width, height, x, y + 4, ViewType::Command);
    cview->command_view = std::move(v);
    return cview;
}

CommandView *CommandView::create_not_managed(const std::string &name, int width, int height, int x, int y) {
    auto cview = new CommandView{};
    cview->name = name;
    cview->x = x;
    cview->y = y;
    cview->w = width;
    cview->h = height;
    auto input_buffer = StdStringBuffer::make_handle();
    input_buffer->id = (int) std::hash<std::string>{}("CommandView");
    cview->input_buffer = std::move(input_buffer);
    util::println("Placing command view at {}, {} with dimensions {}, {}", x, y, width, height);
    auto v = View::create_managed(cview->input_buffer.get(), name, width, height, x, y + 4, ViewType::Command);
    cview->command_view = std::move(v);
    return cview;
}

//! --------------------------------------------------------

void View::set_projection(glm::mat4 view_projection) {
    this->cursor->set_projection(view_projection);
    this->projection = view_projection;
}
void View::set_dimensions(int w, int h) {
    this->width = w;
    this->height = h;
}
void View::anchor_at(int x, int y) {
    this->x = x;
    this->y = y;
}
SimpleFont *View::get_font() { return font; }
TextData *View::get_text_buffer() const { return data; }

void CommandView::set_prefix(const std::string &prefix) {
    this->infoPrefix = prefix;
    this->infoPrefix.append(": ");
}

/// ----------------- VIEW DRAW METHODS -----------------

void View::draw(bool isActive) {
    glEnable(GL_SCISSOR_TEST);
    // GL anchors x, y in the bottom left, with our orthographic view, we anchor from top left, thus we have to take y-h, instead of just taking y
    glScissor(x, y - height, this->width, this->height);

    if(isActive) {
        glClearColor(0.25f, 0.35f, 0.35f, 1.0f);
    } else {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    vao->bind_all();
    shader->use();
    font->t->bind();
    // projection = glm::ortho(0.0f, static_cast<float>(ww), 0.0f, static_cast<float>(wh));
    shader->set_projection(projection);
    cursor->set_projection(projection);
    auto vao_ = vao.get();
    auto text_size = data->size();
    if (text_size > this->vertexCapacity) { this->vao->reserve_gpu_size(text_size * 2); }

    if (data->is_pristine()) {
        vao->draw();
        this->cursor->draw();
    } else {
        font->emplace_source_text_gpu_data(vao_, this, this->x + View::TEXT_LENGTH_FROM_EDGE,
                                           this->y - font->get_row_advance());
        vao->flush_and_draw();
        this->cursor->forced_draw();
    }
    glDisable(GL_SCISSOR_TEST);
}

void View::forced_draw(bool isActive) {
    // FN_MICRO_BENCH();
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y - height, this->width, this->height);
    if(isActive) {
        glClearColor(0.25f, 0.35f, 0.35f, 1.0f);
    } else {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    vao->bind_all();
    shader->use();
    font->t->bind();
    shader->set_projection(projection);
    cursor->set_projection(projection);
    auto data_view = data->to_string_view();
    auto text_size = data->size();
    if (text_size > this->vertexCapacity) { this->vao->reserve_gpu_size(text_size * 2); }
    font->emplace_source_text_gpu_data(vao.get(), this, this->x + View::TEXT_LENGTH_FROM_EDGE,
                                       this->y - font->get_row_advance());
    vao->flush_and_draw();
    this->cursor->forced_draw();
    glDisable(GL_SCISSOR_TEST);
}

void View::forced_draw_with_prefix_colorized(const std::string &prefix,
                                             std::optional<std::vector<ColorizeTextRange>> cInfo, bool isActive) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, 0, this->width, this->height);

    if(isActive) {
        glClearColor(0.25f, 0.35f, 0.35f, 1.0f);
    } else {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    auto &ci = CommandInterpreter::get_instance();
    std::string textToRender = prefix;
    std::string cmd_rep = "";
    if (ci.get_currently_edited_cmd()) {
        cmd_rep = ci.get_currently_edited_cmd()->as_auto_completed();
    } else {
        cmd_rep = this->data->to_std_string();
    }
    vao->bind_all();
    shader->use();
    shader->set_projection(projection);
    font->t->bind();

    textToRender.append(cmd_rep);

    if (cInfo) {
        auto defaultColor = glm::fvec3{0.74f, 0.425f, 0.46f};
        auto text_len = textToRender.size();
        auto &vec = cInfo.value();
        std::vector<ColorizeTextRange> fully_formatted{};
        auto index = 0u;
        for (auto range : vec) {
            if (index < range.begin) {
                auto a = ColorizeTextRange{.begin = index, .length = (range.begin) - index, .color = defaultColor};
                fully_formatted.push_back(a);
                fully_formatted.push_back(range);
            } else {
                fully_formatted.push_back(range);
            }
            index += range.begin + range.length;
        }
        if (index != text_len) {
            fully_formatted.push_back(
                    ColorizeTextRange{.begin = index, .length = (text_len) -index, .color = defaultColor});
        }

        font->emplace_colorized_text_gpu_data(vao.get(), textToRender, this->x + View::TEXT_LENGTH_FROM_EDGE,
                                              this->y - font->get_row_advance(), fully_formatted);
        vao->flush_and_draw();
    } else {
        font->emplace_colorized_text_gpu_data(vao.get(), textToRender, this->x + View::TEXT_LENGTH_FROM_EDGE,
                                              this->y - font->get_row_advance(), std::move(cInfo));
        // font->emplace_source_text_gpu_data(vao.get(), textToRender, this->x + View::TEXT_LENGTH_FROM_EDGE,this->y - font->get_row_advance());
        vao->flush_and_draw();
    }
    glDisable(GL_SCISSOR_TEST);
}
ViewCursor *View::get_cursor() { return cursor.get(); }
void View::set_name(const std::string &n) { name = n; }
void View::draw_statusbar() {
    vao->bind_all();
    shader->use();
    font->t->bind();
    shader->set_projection(projection);
    auto textToRender = this->data->to_string_view();
    font->emplace_colorized_text_gpu_data(vao.get(), textToRender, this->x + View::TEXT_LENGTH_FROM_EDGE,
                                          this->y - font->get_row_advance(), {});
    // font->emplace_source_text_gpu_data(vao.get(), textToRender, this->x + View::TEXT_LENGTH_FROM_EDGE,this->y - font->get_row_advance());
    vao->flush_and_draw();
}
void View::draw_command_view(const std::string &prefix, std::optional<std::vector<ColorizeTextRange>> colorInfo) {
    auto &ci = CommandInterpreter::get_instance();
    std::string textToRender = prefix;
    std::string cmd_rep = "";
    if (ci.get_currently_edited_cmd()) {
        cmd_rep = ci.get_currently_edited_cmd()->as_auto_completed();
    } else {
        cmd_rep = this->data->to_std_string();
    }
    vao->bind_all();
    shader->use();
    shader->set_projection(projection);
    font->t->bind();

    textToRender.append(cmd_rep);

    if (colorInfo) {
        auto defaultColor = glm::fvec3{0.74f, 0.425f, 0.46f};
        auto text_len = textToRender.size();
        auto &vec = colorInfo.value();
        std::vector<ColorizeTextRange> fully_formatted{};
        auto index = 0u;
        for (auto range : vec) {
            if (index < range.begin) {
                auto a = ColorizeTextRange{.begin = index, .length = (range.begin) - index, .color = defaultColor};
                fully_formatted.push_back(a);
                fully_formatted.push_back(range);
            } else {
                fully_formatted.push_back(range);
            }
            index += range.begin + range.length;
        }
        if (index != text_len) {
            fully_formatted.push_back(
                    ColorizeTextRange{.begin = index, .length = (text_len) -index, .color = defaultColor});
        }

        font->emplace_colorized_text_gpu_data(vao.get(), textToRender, this->x + View::TEXT_LENGTH_FROM_EDGE,
                                              this->y - font->get_row_advance(), fully_formatted);
        vao->flush_and_draw();
    } else {
        font->emplace_colorized_text_gpu_data(vao.get(), textToRender, this->x + View::TEXT_LENGTH_FROM_EDGE,
                                              this->y - font->get_row_advance(), std::move(colorInfo));
        // font->emplace_source_text_gpu_data(vao.get(), textToRender, this->x + View::TEXT_LENGTH_FROM_EDGE,this->y - font->get_row_advance());
        vao->flush_and_draw();
    }
}
void View::scroll(Scroll direction, int linesToScroll) {
    auto [win_width, win_height] = App::get_window_dimension();
    switch (direction) {
        case Scroll::Up: {
            auto scroll_pos = scrolled + (linesToScroll * font->get_row_advance());
            scrolled = std::min(scroll_pos, 0);
            auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                static_cast<float>(win_height + scrolled));
            set_projection(p);
        } break;
        case Scroll::Down: {
            scrolled -= (linesToScroll * font->get_row_advance());
            auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                static_cast<float>(win_height + scrolled));
            set_projection(p);
        } break;
    }
}

void View::set_fill(float w, float h, int parent_w, int parent_h) {
    assert(w <= 1.0 && h <= 1.0f);
    width_fill = w;
    height_fill = h;
    width = int(float(parent_w) * w);
    width = int(float(parent_w) * w);
    height = int(float(parent_h) * h);
}

View *View::create(TextData *data, const std::string &name, int w, int h, int x, int y, ViewType type) {
    auto reserveMemory =
            gpu_mem_required_for_quads<TextVertex>(1024);// reserve GPU memory for at least 1024 characters.
    if (!data->empty()) { reserveMemory = gpu_mem_required_for_quads<TextVertex>(data->size()); }
    auto vao = VAO::make(GL_ARRAY_BUFFER, reserveMemory);
    auto v = new View{};
    v->td_id = data->id;
    v->type = type;
    v->width = w;
    v->height = h;
    v->x = x;
    v->y = y;
    v->font = FontLibrary::get_default_font();
    v->shader = ShaderLibrary::get_text_shader();
    v->vao = std::move(vao);
    v->data = data;
    v->vertexCapacity = reserveMemory / sizeof(TextVertex);
    v->cursor = ViewCursor::create_from(*v);
    v->name = name;
    return v;
}

void CommandView::draw() {
    glEnable(GL_SCISSOR_TEST);

    // glViewport(0, this->y, this->w, this->h);
    glScissor(x, 0, this->w, this->h);
    glClearColor(.3f, .3f, .3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (this->active) {
        auto &ci = CommandInterpreter::get_instance();
        if (ci.has_command_waiting()) {
            auto s = ci.get_currently_edited_cmd()->actual_input();
            this->input_buffer->clear();
            this->input_buffer->insert_str(s);

            // command: open ./ma => hit tab
            // command gets auto completed to:
            // command: open ./ma|in.cpp|
            //                      ^- this section gets colored gray
            auto total_len = infoPrefix.size() + ci.get_currently_edited_cmd()->as_auto_completed().size();
            auto what_should_be_colorized_len = total_len - (infoPrefix.size() + s.size());
            ColorizeTextRange color{.begin = infoPrefix.size() + s.size(),
                                    .length = what_should_be_colorized_len,
                                    .color = glm::vec3{0.5f, 0.5f, 0.5f}};
            this->command_view->draw_command_view(this->infoPrefix, {{color}});
        } else {
            this->command_view->draw_command_view(this->infoPrefix, {});
        }
    } else if (this->show_last_message) {
        //TODO: render something else. We are not inputing a commmand we are editing text, when this branch is true
        this->active = false;
        this->command_view->get_text_buffer()->clear();
        this->command_view->get_text_buffer()->insert_str(this->last_message);
        this->command_view->draw();
    } else {
    }

    glDisable(GL_SCISSOR_TEST);
}

//! -----------------------------------------------------
std::unique_ptr<StatusBar> StatusBar::create_managed(int width, int height, int x, int y) {
    auto status_bar_text = DataManager::get_instance().create_managed_buffer(BufferType::CodeInput);
    auto ui_view = View::create_managed(status_bar_text, "status_bar", width, height, x, y);
    TextData::BufferCursor *cursor_info = &status_bar_text->cursor;
    return std::make_unique<StatusBar>(StatusBar{.ui_view = std::move(ui_view), .active_buffer_cursor = cursor_info});
}
void StatusBar::set_buffer_cursor(TextData::BufferCursor *cursor) { active_buffer_cursor = cursor; }
void StatusBar::draw(View *view) {
    auto col = active_buffer_cursor->col_pos;
    auto ln = active_buffer_cursor->col_pos;
    glEnable(GL_SCISSOR_TEST);
    auto dims = App::get_window_dimension();
    glScissor(ui_view->x, dims.h - (ui_view->height + 4), ui_view->width, ui_view->height + 4);
    glClearColor(bg_color.x, bg_color.y, bg_color.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto buf = view->get_text_buffer();
    auto fName = buf->meta_data.buf_name;

    if (fName.empty()) { fName = "*unnamed buffer*"; }

    auto output = fmt::format("{} - [{}, {}]", fName, active_buffer_cursor->line, active_buffer_cursor->col_pos);
    ui_view->get_text_buffer()->clear();
    ui_view->get_text_buffer()->insert_str(output);
    ui_view->draw_statusbar();
    glDisable(GL_SCISSOR_TEST);
}
void StatusBar::print_debug_info() const {
    auto output = fmt::format("[{}, {}]", active_buffer_cursor->line, active_buffer_cursor->col_pos);
    util::println("Text to display: '{}'", output);
}
StatusBar *StatusBar::create(int width, int height, int x, int y) {
    auto status_bar_text = DataManager::get_instance().create_managed_buffer(BufferType::CodeInput);
    auto ui_view = View::create_managed(status_bar_text, "status_bar", width, height, x, y);
    TextData::BufferCursor *cursor_info = &status_bar_text->cursor;
    auto sb = new StatusBar{};
    sb->ui_view = std::move(ui_view);
    sb->active_buffer_cursor = cursor_info;
    return sb;
}

void StatusBar::update_layout(DimInfo dimInfo) {}

/// The optional<TD*> is *ONLY* to express the purpose that this value can be nil
/// just having a pointer here, could *very* well lead me to believe at some point that null can't be passed here
/// when it can.

EditorWindow* EditorWindow::create(std::optional<TextData *> textData, glm::mat4 projection, int layout_id, DimInfo dimInfo) {
    auto&[x, y, width, height] = dimInfo;

    auto sb_height = FontLibrary::get_default_font()->get_row_advance() + 2;
    // we have to make room for status bar & command view spanning across entire bottom, both of which are equal in height
    auto text_editor_height = height - (sb_height * 2);
    auto ew = new EditorWindow{};
    ew->ui_layout_id = layout_id;
    ew->dimInfo = dimInfo;
    if (textData) {
        auto buf = DataManager::get_instance().create_managed_buffer(CodeInput);
        ew->status_bar = StatusBar::create(width, sb_height, x, height);
        ew->view = View::create(*textData, "unnamed buffer", width, text_editor_height, x, height - sb_height);
    } else {
        auto buf = DataManager::get_instance().create_managed_buffer(CodeInput);
        ew->status_bar = StatusBar::create(width, sb_height, x, height);
        ew->view = View::create(buf, "unnamed buffer", width, text_editor_height, x, height - sb_height);
    }

    ew->view->set_projection(projection);
    ew->status_bar->ui_view->set_projection(projection);
    ew->status_bar->set_buffer_cursor(&ew->view->get_text_buffer()->cursor);
    return ew;
}

EditorWindow::~EditorWindow() {
    delete view;
    delete status_bar;
}

void EditorWindow::draw(bool force_redraw) {
    if (force_redraw) {
        view->forced_draw(this->active);
    } else {
        view->draw(this->active);
    }
    this->status_bar->draw(view);
}

TextData *EditorWindow::get_text_buffer() { return view->get_text_buffer(); }

void EditorWindow::update_layout(DimInfo dimInfo) {
    auto&[x, y, width, height] = dimInfo;
    auto sb_height = FontLibrary::get_default_font()->get_row_advance() + 2;
    // we have to make room for status bar & command view spanning across entire bottom, both of which are equal in height
    auto text_editor_height = height - (sb_height * 2);
    auto text_editor_y_pos = height - sb_height;
    this->view->x = x;
    this->view->y = text_editor_y_pos;
    this->view->width = width;
    this->view->height = text_editor_height;
    this->status_bar->ui_view->width = width;
    this->status_bar->ui_view->height = sb_height;
    this->status_bar->ui_view->x = x;
    this->status_bar->ui_view->y = height;
}
void EditorWindow::set_projection(glm::mat4 projection) {
    this->view->set_projection(projection);
    this->status_bar->ui_view->set_projection(projection);
}
