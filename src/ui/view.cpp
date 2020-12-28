//
// Created by 46769 on 2020-12-22.
//

#include "view.hpp"
#include <utility>
#include <vector>

#include <core/commands/command_interpreter.hpp>
#include <ui/managers/font_library.hpp>
#include <ui/managers/shader_library.hpp>

/// ----------------- VIEW FACTORY FUNCTIONS -----------------

std::unique_ptr<View> View::create(TextData *data, const std::string &name, int w, int h, int x, int y, ViewType type) {
    auto reserveMemory =
            gpu_mem_required_for_quads<TextVertex>(1024);// reserve GPU memory for at least 1024 characters.
    if (!data->empty()) { reserveMemory = gpu_mem_required_for_quads<TextVertex>(data->size()); }
    auto vao = VAO::make(GL_ARRAY_BUFFER, reserveMemory);
    auto font = FontLibrary::get_default_font();
    auto shader = ShaderLibrary::get_text_shader();
    auto v = std::make_unique<View>();
    v->type = type;
    v->width = w;
    v->height = h;
    v->x = x;
    v->y = y;
    v->font = font;
    v->shader = shader;
    v->vao = std::move(vao);
    v->data = data;
    v->vertexCapacity = reserveMemory / sizeof(TextVertex);
    v->cursor = ViewCursor::create_from(v);
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
    input_buffer->id = 2;
    cview->input_buffer = std::move(input_buffer);
    util::println("Placing command view at {}, {} with dimensions {}, {}", x, y, width, height);
    auto v = View::create(cview->input_buffer.get(), name, width, height, x, y + 4, ViewType::Command);
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

void View::set_scroll(int scrollPos) { this->scroll = scrollPos; }

void CommandView::set_prefix(const std::string &prefix) {
    this->infoPrefix = prefix;
    this->infoPrefix.append(": ");
}

/// ----------------- VIEW DRAW METHODS -----------------

void View::draw() {
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
}

void View::forced_draw() {
    // FN_MICRO_BENCH();
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
}

void View::forced_draw_with_prefix_colorized(const std::string &prefix,
                                             std::optional<std::vector<ColorizeTextRange>> cInfo) {
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
    font->t->bind();

    textToRender.append(cmd_rep);

    if (cInfo) {
        auto defaultColor = glm::fvec3{0.2f, 0.325f, 0.75f};
        auto text_len = textToRender.size();
        auto &vec = cInfo.value();
        std::vector<ColorizeTextRange> fully_formatted{};
        auto index = 0u;
        for (auto range : vec) {
            if (index < range.begin) {
                auto a = ColorizeTextRange{.begin = index, .length = (range.begin) - index, .color = defaultColor};
                util::println("Adding format for {} -> {} before {} -> {}", a.begin, a.begin+a.length, range.begin, range.begin+range.length);
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

        for(const auto& r : fully_formatted) {
            util::println("{} -> {} out of {}", r.begin, r.begin+r.length, text_len);
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
}
ViewCursor *View::get_cursor() { return cursor.get(); }

void CommandView::draw() {
    glEnable(GL_SCISSOR_TEST);

    // glViewport(0, this->y, this->w, this->h);
    glScissor(0, 0, this->w, this->h);
    glClearColor(.0f, .0f, .0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if (this->active) {
        auto &ci = CommandInterpreter::get_instance();
        if (ci.has_command_waiting()) {
            auto s = ci.get_currently_edited_cmd()->actual_input();
            this->input_buffer->clear();
            this->input_buffer->insert(s);

            // command: open ./ma => hit tab
            // command gets auto completed to:
            // command: open ./ma|in.cpp|
            //                      ^- this section gets colored gray
            auto total_len = infoPrefix.size() + ci.get_currently_edited_cmd()->as_auto_completed().size();
            auto what_should_be_colorized_len = total_len - (infoPrefix.size() + s.size());
            ColorizeTextRange color{.begin = infoPrefix.size() + s.size(),
                                    .length = what_should_be_colorized_len,
                                    .color = glm::vec3{0.5f, 0.5f, 0.5f}};
            this->command_view->forced_draw_with_prefix_colorized(this->infoPrefix, {{color}});
        } else {
            this->command_view->forced_draw_with_prefix_colorized(this->infoPrefix, {});
        }
    } else {
        //TODO: render something else. We are not inputing a commmand we are editing text, when this branch is true
    }

    glDisable(GL_SCISSOR_TEST);
}

//! -----------------------------------------------------
