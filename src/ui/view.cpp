//
// Created by 46769 on 2020-12-22.
//

#include <glm/gtc/matrix_transform.hpp>

#include "view.hpp"
#include <utility>
#include <vector>

#include <app.hpp>

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
        lines_displayable = std::ceil(h / font->get_row_advance());
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

        if (isActive) {
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

        if (text_size * 6 > this->vertexCapacity) {
            this->vao->reserve_gpu_size(text_size * 2 + 2);
            this->vertexCapacity = (text_size * 6 * 2 + 2);
        }

        if (data->is_pristine()) {
            vao->draw();
            cursor->draw();
        } else {
            font->emplace_source_text_gpu_data(vao_, this, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                               this->y - font->get_row_advance());
            vao->flush_and_draw();
            cursor->forced_draw();
        }
        glDisable(GL_SCISSOR_TEST);
    }

    void View::forced_draw(bool isActive) {
        // FN_MICRO_BENCH();
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, y - height, this->width, this->height);
        if (isActive) {
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
        auto text_size = data->size();
        if (text_size * 6 > this->vertexCapacity) {
            this->vao->reserve_gpu_size(text_size * 2 + 2);
            this->vertexCapacity = (text_size * 6 * 2 + 2);
        }
        font->emplace_source_text_gpu_data(vao.get(), this, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                           this->y - font->get_row_advance());
        vao->flush_and_draw();
        this->cursor->forced_draw();
        glDisable(GL_SCISSOR_TEST);
    }

    void View::forced_draw_with_prefix_colorized(const std::string &prefix,
                                                 std::optional<std::vector<ColorizeTextRange>> cInfo, bool isActive) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(x, 0, this->width, this->height);

        if (isActive) {
            glClearColor(0.25f, 0.35f, 0.35f, 1.0f);
        } else {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT);
        auto &ci = CommandInterpreter::get_instance();
        std::string textToRender = prefix;
        std::string cmd_rep = "";
        if (ci.has_command_waiting()) {
            cmd_rep = ci.current_input();
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

            font->emplace_colorized_text_gpu_data(vao.get(), textToRender, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                                  this->y - font->get_row_advance(), fully_formatted);
            vao->flush_and_draw();
        } else {
            font->emplace_colorized_text_gpu_data(vao.get(), textToRender, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                                  this->y - font->get_row_advance(), std::move(cInfo));
            // font->emplace_source_text_gpu_data(vao.get(), textToRender, this->x + View::TEXT_LENGTH_FROM_EDGE,this->y - font->get_row_advance());
            vao->flush_and_draw();
        }
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

            font->emplace_colorized_text_gpu_data(vao.get(), textToRender, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                                  this->y - font->get_row_advance(), fully_formatted);
            vao->flush_and_draw();
        } else {
            font->emplace_colorized_text_gpu_data(vao.get(), textToRender, AS(this->x + View::TEXT_LENGTH_FROM_EDGE, int),
                                                  this->y - font->get_row_advance(), std::move(colorInfo));
            vao->flush_and_draw();
        }
    }
    void View::scroll(Scroll direction, int linesToScroll) {
        /// TODO: add data so that the view cursor knows the entire range of lines being displayed
        ///  this is so that if we want to jump on a line that currently is displayed in view, we don't scroll down the view to it
        //// we can just scroll the buffer cursor position instead.
        auto [win_width, win_height] = ::App::get_window_dimension();
        util::println("View cursor line prior to scroll: {}", cursor->line);
        switch (direction) {
            case Scroll::Up: {
                /// TODO: Bounds check so we prohibit the user from scrolling up where no lines can be. Which would be bad UX
                auto scroll_pos = scrolled + (linesToScroll * font->get_row_advance());
                scrolled = std::min(scroll_pos, 0);
                auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                    static_cast<float>(win_height + scrolled));
                set_projection(p);
                cursor->line -= linesToScroll;
                cursor->line = std::max(cursor->line, 0);
            } break;
            case Scroll::Down: {
                /// TODO: Bounds check so we prohibit the user from scrolling down where no lines are. Which would be bad UX
                scrolled -= (linesToScroll * font->get_row_advance());
                auto p = glm::ortho(0.0f, static_cast<float>(win_width), static_cast<float>(scrolled),
                                    static_cast<float>(win_height + scrolled));
                set_projection(p);
                cursor->line += linesToScroll;
                cursor->line = std::min(cursor->line, int(get_text_buffer()->meta_data.line_begins.size() - 3));
            } break;
        }
        if(data->has_metadata()) {
            auto pos = data->meta_data.line_begins[cursor->line];
            data->step_cursor_to(pos);
        }
        util::println("View cursor line: {}", cursor->line);
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
        auto reserveMemory_Quads =
                gpu_mem_required_for_quads<TextVertex>(1024);// reserve GPU memory for at least 1024 characters.
        if (!data->empty()) { reserveMemory_Quads = gpu_mem_required_for_quads<TextVertex>(data->size() * 2);
        }
        auto vao = VAO::make(GL_ARRAY_BUFFER, reserveMemory_Quads);
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
        v->vertexCapacity = reserveMemory_Quads / sizeof(TextVertex);
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
                if (ci.cmd_is_interactive()) {
                    auto s = ci.current_input();
                    auto suggestion = ci.command_auto_completed();
                    // command: open ./ma => hit tab
                    // command gets auto completed to:
                    // command: open ./ma|in.cpp|
                    //                      ^- this section gets colored gray
                    auto total_len = infoPrefix.size() + suggestion.size();
                    auto what_should_be_colorized_len = total_len - (infoPrefix.size() + s.size());
                    ColorizeTextRange color{.begin = infoPrefix.size() + s.size(),
                                            .length = what_should_be_colorized_len,
                                            .color = glm::vec3{0.5f, 0.5f, 0.5f}};
                    this->command_view->draw_command_view(this->infoPrefix, {{color}});
                } else {
                    auto s = ci.current_input();
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
            util::println("Showing last err message: {}", last_message);
            this->active = false;
            this->command_view->get_text_buffer()->clear();
            this->command_view->get_text_buffer()->insert_str(last_message);
            draw_error_message();
        } else {
        }

        glDisable(GL_SCISSOR_TEST);
    }

    void CommandView::draw_error_message() {
        assert(not last_message.empty());
        command_view->draw_command_view("error: ", {});
    }

}// namespace ui