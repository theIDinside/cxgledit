//
// Created by 46769 on 2020-12-22.
//

#include <glm/gtc/matrix_transform.hpp>

#include "view.hpp"
#include <utility>
#include <vector>

// Managers
#include <core/data_manager.hpp>
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>
#include <core/commands/command_interpreter.hpp>

#undef min
#undef max

/// ----------------- VIEW FACTORY FUNCTIONS -----------------

namespace ui {

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
    v->lines_displayable = int_floor(float(h) / float(v->font->get_row_advance())) - 7;
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
    lines_displayable = int_floor(float(h) / float(font->get_row_advance())) - 7;
    util::println("lines displayable updated to: {}", lines_displayable);
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
    using Pos = ui::core::ScreenPos;
    glEnable(GL_SCISSOR_TEST);
    // GL anchors x, y in the bottom left, with our orthographic view, we anchor from top left, thus we have to take y-h, instead of just taking y
    glScissor(x, y - height, this->width, this->height);

    // TODO(optimization?): lift out display settings into a static structure reachable everywhere, to query info about size, settings, colors etc
    auto[r,g,b] = bg_color;
    if (isActive) {
        glClearColor(r + 0.05, g + 0.05, b + 0.05, 1.0f);
    } else {
        glClearColor(r, g, b, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    vao->bind_all();
    shader->use();
    font->t->bind();
    shader->set_projection(projection);
    cursor->set_projection(projection);
    auto text_size = data->size();

    if (text_size * 6 > this->vertexCapacity) {
        this->vao->reserve_gpu_size(text_size * 2 + 2);
        this->vertexCapacity = (text_size * 6 * 2 + 2);
    }

    if (data->is_pristine()) {
        vao->draw();
        cursor->draw();
    } else {
        auto xpos = AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int);
        auto ypos = AS(this->y - font->get_row_advance(), int);
        font->create_vertex_data_for(this, Pos{xpos, ypos});
        vao->flush_and_draw();
        cursor->forced_draw();
    }
    glDisable(GL_SCISSOR_TEST);
}

void View::forced_draw(bool isActive) {
    // FN_MICRO_BENCH();
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y - height, this->width, this->height);
    auto[r,g,b] = bg_color;
    if (isActive) {
        glClearColor(r + 0.05, g + 0.05, b + 0.05, 1.0f);
    } else {
        glClearColor(r, g, b, 1.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT);
    vao->bind_all();
    shader->use();
    font->t->bind();
    shader->set_projection(projection);
    cursor->set_projection(projection);
    auto text_size = data->size();
    if (text_size * 6 > this->vertexCapacity) {
        this->vao->reserve_gpu_size(text_size * 2 + 2);
        this->vertexCapacity = (text_size * 6 * 2 + 2);
    }
    font->create_vertex_data_in(vao.get(), this, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                       this->y - font->get_row_advance());
    vao->flush_and_draw();
    this->cursor->forced_draw();
    glDisable(GL_SCISSOR_TEST);
}

ViewCursor *View::get_cursor() { return cursor.get(); }

void View::draw_statusbar() {
    vao->bind_all();
    shader->use();
    font->t->bind();
    shader->set_projection(projection);
    auto textToRender = this->data->to_string_view();
    font->emplace_colorized_text_gpu_data(vao.get(), textToRender, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                          this->y - font->get_row_advance(), {});
    vao->flush_and_draw();
}
void View::draw_command_view(const std::string &prefix, std::optional<std::vector<ColorizeTextRange>> colorInfo) {
    auto &ci = CommandInterpreter::get_instance();
    std::string textToRender = prefix;
    std::string cmd_rep = ci.command_auto_completed();
    vao->bind_all();
    shader->use();
    shader->set_projection(projection);
    font->t->bind();
    auto defaultColor = glm::fvec3{1.0f, 1.0f, 1.0f};
    textToRender.append(cmd_rep);
    if (colorInfo) {
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

        font->emplace_colorized_text_gpu_data(vao.get(), textToRender, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                              this->y - font->get_row_advance(), fully_formatted);
        vao->flush_and_draw();
    } else {
        font->emplace_colorized_text_gpu_data(vao.get(), textToRender, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                              this->y - font->get_row_advance(), std::move(colorInfo));
        vao->flush_and_draw();
    }
}

void View::goto_buffer_position() {
    auto view_begin = cursor->line;
    auto view_end = view_begin + lines_displayable;
    if(not is_within(data->cursor.line, view_begin, view_end)) {
        scroll_to(data->cursor.line);
    }
}


// TODO: make it so, if view is outside of where the buffer cursor is, when navigational keys are pressed, zoom right into where
//  it is.
void View::scroll_to(int line) {
    auto [win_width, win_height] = ::App::get_window_dimension();

    if(line < cursor->line) { // means we need to => scroll up
        auto linesToScroll = (cursor->line + lines_displayable) - line;
        auto scroll_pos = scrolled + (linesToScroll * font->get_row_advance());
        scrolled = std::min(scroll_pos, 0);
        auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                            static_cast<float>(win_height + scrolled));
        set_projection(p);
        cursor->line -= linesToScroll;
        cursor->line = std::max(cursor->line, 0);
    } else { // means we need to => scroll down
        auto lineAnchorOfLastPage = get_text_buffer()->lines_count() - lines_displayable;
        if(line > lineAnchorOfLastPage) {
            auto linesToScroll = lineAnchorOfLastPage - cursor->line;
            scrolled -= (linesToScroll * font->get_row_advance());
            auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                static_cast<float>(win_height + scrolled));
            set_projection(p);
            cursor->line += linesToScroll;
            cursor->line = std::min(cursor->line, int(get_text_buffer()->meta_data.line_begins.size() - lines_displayable));
        } else {
            auto linesToScroll = line - cursor->line;
            scrolled -= (linesToScroll * font->get_row_advance());
            auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                static_cast<float>(win_height + scrolled));
            set_projection(p);
            cursor->line += linesToScroll;
            cursor->line = std::min(cursor->line, int(get_text_buffer()->meta_data.line_begins.size() - lines_displayable));
        }
    }
}

void View::scroll(Scroll direction, int linesToScroll) {
    /// TODO: add data so that the view cursor knows the entire range of lines being displayed
    ///  this is so that if we want to jump on a line that currently is displayed in view, we don't scroll down the view to it
    //// we can just scroll the buffer cursor position instead.
    auto [win_width, win_height] = ::App::get_window_dimension();
    switch (direction) {
        case Scroll::Up: {
            /// TODO: Bounds check so we prohibit the user from scrolling up where no lines can be. Which would be bad UX
            if(cursor->line - linesToScroll < 0) {
                cursor->line = 0;
                scrolled = 0;
                auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                    static_cast<float>(win_height + scrolled));
                set_projection(p);
            } else {
                auto scroll_pos = scrolled + (linesToScroll * font->get_row_advance());
                scrolled = std::min(scroll_pos, 0);
                auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                    static_cast<float>(win_height + scrolled));
                set_projection(p);
                cursor->line -= linesToScroll;
                cursor->line = std::max(cursor->line, 0);
            }
        } break;
        case Scroll::Down: {
            int last_view_top_line = std::max(AS(get_text_buffer()->meta_data.line_begins.size(), int) - lines_displayable, 0);
            if(cursor->line + linesToScroll >= last_view_top_line) {
                util::println("Lines to scroll: {}, Lines displayable: {} - Max view anchor position: {}. Buffer lines: {}", linesToScroll, lines_displayable, int(get_text_buffer()->meta_data.line_begins.size() - lines_displayable), get_text_buffer()->meta_data.line_begins.size());
                auto diff = (cursor->line + linesToScroll) - int(get_text_buffer()->meta_data.line_begins.size() - lines_displayable);
                scrolled -= (diff * font->get_row_advance());
                auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                    static_cast<float>(win_height + scrolled));
                set_projection(p);
                cursor->line += diff;
                cursor->line = std::min(cursor->line, int(get_text_buffer()->meta_data.line_begins.size() - lines_displayable));
            } else {
                /// TODO: Bounds check so we prohibit the user from scrolling down where no lines are. Which would be bad UX
                // scrolled is not lines scrolled, but the amount of pixels we've moved the projection north/south
                scrolled -= (linesToScroll * font->get_row_advance());
                auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                    static_cast<float>(win_height + scrolled));
                set_projection(p);
                cursor->line += linesToScroll;
                cursor->line = std::min(cursor->line, int(get_text_buffer()->meta_data.line_begins.size() - lines_displayable));
            }
        } break;
    }
}

View *View::create(TextData *data, const std::string &name, int w, int h, int x, int y, ViewType type) {
    auto reserveMemory_Quads =
            gpu_mem_required_for_quads<TextVertex>(1024);// reserve GPU memory for at least 1024 characters.
    if (!data->empty()) { reserveMemory_Quads = gpu_mem_required_for_quads<TextVertex>(data->size() * 2); }
    auto vao = VAO::make(GL_ARRAY_BUFFER, reserveMemory_Quads);
    auto v = new View{};
    v->td_id = data->id;
    v->type = type;
    v->width = w;
    v->height = h;
    v->x = x;
    v->y = y;
    v->font = FontLibrary::get_default_font();
    v->lines_displayable = int_floor(float(h) / float(v->font->get_row_advance())) - 7;
    v->shader = ShaderLibrary::get_text_shader();
    v->vao = std::move(vao);
    v->data = data;
    v->vertexCapacity = reserveMemory_Quads / sizeof(TextVertex);
    v->cursor = ViewCursor::create_from(v);
    v->name = name;
    return v;
}
void View::set_font(SimpleFont *new_font) {
    font = new_font;
    cursor->setup_dimensions(cursor->width, font->max_glyph_height + 4);
    forced_draw(true);
}
View::~View() {
    util::println("Destroying View {} and it's affiliated resources. TextBuffer id: {} - Name: {}", name, get_text_buffer()->id, get_text_buffer()->fileName());
    if (not DataManager::get_instance().is_managed(data->id)) {
        DataManager::get_instance().print_all_managed();
        delete data;
    }
}

void View::draw_modal_view(int selected, std::vector<TextDrawable>& drawables) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y - height, this->width, this->height);
    auto[r,g,b] = bg_color;

    glClear(GL_COLOR_BUFFER_BIT);
    vao->bind_all();
    shader->use();
    font->t->bind();
    shader->set_projection(projection);
    cursor->set_projection(projection);
    auto text_size = data->size();
    if (text_size * 6 > this->vertexCapacity) {
        this->vao->reserve_gpu_size(text_size * 2 + 2);
        this->vertexCapacity = (text_size * 6 * 2 + 2);
    }
    font->add_colorized_text_gpu_data(vao.get(), drawables);
    vao->flush_and_draw();
    cursor->set_line_rect(drawables[selected].xpos, drawables[selected].xpos + width, drawables[selected].ypos - 4, font->row_height - 2);
    cursor->forced_draw();
    glDisable(GL_SCISSOR_TEST);
}


void CommandView::draw() {
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, 0, this->w, this->h);
    glClearColor(.3f, .3f, .3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (this->active) {
        auto &ci = CommandInterpreter::get_instance();
        if (ci.has_command_waiting()) {
            if (ci.cmd_is_interactive()) {
                auto s = ci.current_input().value_or(ci.command_auto_completed());
                auto suggestion = ci.command_auto_completed();
                // command: open ./ma => hit tab
                // command gets auto completed to:
                // command: open ./ma|in.cpp|
                //                      ^- this section gets colored gray
                auto total_len = infoPrefix.size() + suggestion.size();
                if (ci.command_can_autocomplete()) {
                    auto what_should_be_colorized_len = total_len - (infoPrefix.size() + s.size());
                    ColorizeTextRange color{.begin = infoPrefix.size() + s.size(),
                                            .length = what_should_be_colorized_len,
                                            .color = glm::vec3{0.5f, 0.5f, 0.5f}};
                    this->command_view->draw_command_view(this->infoPrefix, {{color}});
                } else {
                    auto what_should_be_colorized_len = total_len - (infoPrefix.size() + suggestion.size());
                    ColorizeTextRange color{.begin = infoPrefix.size() + suggestion.size(),
                                            .length = what_should_be_colorized_len,
                                            .color = glm::vec3{0.5f, 0.5f, 0.5f}};
                    this->command_view->draw_command_view(this->infoPrefix, {{color}});
                }
            } else {
                auto s = ci.current_input().value();
                // command: open ./ma => hit tab
                // command gets auto completed to:
                // command: open ./ma|in.cpp|
                //                      ^- this section gets colored gray
                auto total_len = infoPrefix.size() + s.size();
                auto what_should_be_colorized_len = total_len - (infoPrefix.size() + s.size());
                ColorizeTextRange color{.begin = infoPrefix.size() + s.size(),
                                        .length = what_should_be_colorized_len,
                                        .color = glm::vec3{0.5f, 0.5f, 0.5f}};
                this->command_view->draw_command_view(this->infoPrefix, {{color}});
            }
        } else {
        }

    } else if (this->show_last_message) {
        //TODO: render something else. We are not inputing a commmand we are editing text, when this branch is true
        this->active = false;
        this->command_view->get_text_buffer()->clear();
        this->command_view->get_text_buffer()->insert_str(last_message);
        draw_current();
    } else {
    }

    glDisable(GL_SCISSOR_TEST);
}

using namespace std::string_view_literals;
void CommandView::draw_error_message() {
    assert(not last_message.empty());
    infoPrefix = "error: ";
    ColorizeTextRange msg_color{.begin = infoPrefix.size(),
                                .length = last_message.size(),
                                .color = glm::vec3{0.9f, 0.9f, 0.9f}};
    auto color_cfg = to_option_vec(msg_color);
    command_view->draw_command_view("error: ", color_cfg);
}

void CommandView::draw_error_message(std::string &&msg) {
    assert(not msg.empty());
    infoPrefix = "error: ";
    ColorizeTextRange msg_color{.begin = infoPrefix.size(), .length = msg.size(), .color = glm::vec3{0.9f, 0.9f, 0.9f}};
    auto color_cfg = to_option_vec(msg_color);
    last_message = std::move(msg);
    show_last_message = true;
    command_view->draw_command_view("error: ", color_cfg);
}

void CommandView::draw_message(std::string &&msg) {
    assert(not msg.empty());
    ColorizeTextRange msg_color{.begin = 0, .length = msg.size(), .color = glm::vec3{0.9f, 0.9f, 0.9f}};
    auto color_cfg = to_option_vec(msg_color);
    last_message = std::move(msg);
    show_last_message = true;
    infoPrefix = "";
    command_view->draw_command_view("", color_cfg);
}
void CommandView::draw_current() {
    ColorizeTextRange msg_color{.begin = infoPrefix.size(),
                                .length = last_message.size(),
                                .color = glm::vec3{0.9f, 0.9f, 0.9f}};
    auto color_cfg = to_option_vec(msg_color);
    command_view->draw_command_view(infoPrefix, color_cfg);
}

}// namespace ui