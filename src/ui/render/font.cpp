//
// Created by 46769 on 2020-12-20.
//

// App headers
#include "font.hpp"
#include <ui/syntax_highlighting.hpp>
#include <ui/views/view.hpp>
#include <ui/core/layout.hpp>
#include <core/buffer/std_string_buffer.hpp>

// Sys headers
#include <algorithm>
#include <numeric>
#include <vector>
#include <ranges>

// 3rd party headers
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

using u64 = std::size_t;

/// Clean interface wrapped over the C-library. this illustrates intent much better for a moron like me.
/// Also, it's *only* used here, since it's using the free type library.
template <typename T, Integral I>
constexpr auto alloc_objects_of(I objects_count) {
    return (T*)std::calloc(objects_count, sizeof(T));
}

std::unique_ptr<SimpleFont> SimpleFont::setup_font(const std::string &path, int pixel_size, CharacterRange charRange) {
    FT_Library ft;
    FT_Face face;
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, path.c_str(), 0, &face);
    FT_Set_Pixel_Sizes(face, pixel_size, pixel_size);
    // FT_Set_Char_Size(face, 0, 16 << 6, 96, 96);

    // quick and dirty max texture size estimate

    const int max_dim = (1 + (face->size->metrics.height >> 6)) * ceilf(sqrtf(NUM_GLYPHS));
    int tex_width = 1;
    while (tex_width < max_dim) tex_width <<= 1;
    const int tex_height = tex_width;

    // render glyphs to atlas
    auto pixels = alloc_objects_of<unsigned char>(tex_width * tex_height);
    int pen_x = 0, pen_y = 0;
    auto max_glyph_height = 0u;
    auto max_glyph_width = 0u;
    std::vector<glyph_info> glyph_cache;
    auto max_bearing_size_diff = 0;
    for (int i = 0; i < charRange.to; ++i) {

        FT_Load_Char(face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
        FT_Bitmap *bmp = &face->glyph->bitmap;
        max_glyph_height = std::max(face->glyph->bitmap.rows, max_glyph_height);
        max_glyph_width = std::max(face->glyph->bitmap.width, max_glyph_width);

        if (pen_x + bmp->width >= tex_width) {
            pen_x = 0;
            pen_y += ((face->size->metrics.height >> 6) + 1);
        }

        for (int row = 0; row < bmp->rows; ++row) {
            for (int col = 0; col < bmp->width; ++col) {
                int x = pen_x + col;
                int y = pen_y + row;
                pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
            }

        }

        // this is stuff you'd need when rendering individual glyphs out of the
        // atlas

        glyph_info glyphInfo{
                .x0 = pen_x,
                .y0 = pen_y,
                .x1 = static_cast<int>(pen_x + bmp->width),
                .y1 = static_cast<int>(pen_y + bmp->rows),
                .x_off = face->glyph->bitmap_left,
                .y_off = face->glyph->bitmap_top,
                .advance = static_cast<int>(face->glyph->advance.x >> 6),
                .size = Vec2i{static_cast<int>(face->glyph->bitmap.width), static_cast<int>(face->glyph->bitmap.rows)},
                .bearing = Vec2i{face->glyph->bitmap_left, face->glyph->bitmap_top},
        };

        max_bearing_size_diff = std::max(std::abs(glyphInfo.size.y - glyphInfo.bearing.y), max_bearing_size_diff);
        glyph_cache.emplace_back(glyphInfo);
        pen_x += bmp->width + 1;
    }
    auto max_adv_y = max_glyph_height + 5;
    row_advance = max_adv_y;
    auto texture = GlyphTexture::make_from_data(pixels, tex_width, tex_height, 1);
    FT_Done_FreeType(ft);

    auto png_data = alloc_objects_of<char>(tex_width * tex_height * 4);
    for (int i = 0; i < (tex_width * tex_height); ++i) {
        png_data[i * 4 + 0] |= pixels[i];
        png_data[i * 4 + 1] |= pixels[i];
        png_data[i * 4 + 2] |= pixels[i];
        png_data[i * 4 + 3] = 0xff;
    }

    auto output_path = fs::current_path() / fmt::format("{}_{}_output.png", path, pixel_size);

    auto png_filename = fmt::format("{}", output_path.string());

    util::println("Writing file to {}", png_filename);

    stbi_write_png(png_filename.c_str(), tex_width, tex_height, 4, png_data, tex_width * 4);
    free(png_data);
    free(pixels);

    auto font = std::make_unique<SimpleFont>(SimpleFont{pixel_size, std::move(texture), std::move(glyph_cache)});
    font->row_height = row_advance;
    font->max_glyph_width = static_cast<int>(max_glyph_width);
    font->max_glyph_height = static_cast<int>(max_glyph_height);
    font->size_bearing_difference_max = max_bearing_size_diff;

    return font;
}

SimpleFont::SimpleFont(int pixelSize, std::unique_ptr<GlyphTexture> &&texture, std::vector<glyph_info> &&glyphs)
    : t(std::move(texture)), glyph_cache(std::move(glyphs)), pixel_size(pixelSize) {}

int SimpleFont::get_pixel_row_advance() const { return row_height; }

void SimpleFont::create_vertex_data_in(TextVertexArrayObject *vao, ui::View *view, int xPos, int yPos) {

    auto text = view->get_text_buffer()->to_string_view();
    auto view_cursor = view->get_cursor();

    // TODO(use cy2 for when we select multiple lines): right now only one line can be selected, which is why cy2 is not used
    GLfloat cx1, cx2, cy1, cy2;
    auto bufPtr = view->get_text_buffer();

    auto [cursor_a, cursor_b] = bufPtr->get_cursor_rect();

    int data_index_pos = cursor_a.pos;
    int data_index_pos_end = cursor_b.pos;

    vao->vbo->data.clear();
    vao->vbo->data.reserve(gpu_mem_required_for_quads<TextVertex>(text.size()));
    auto &store = vao->vbo->data;
    const auto start_x = xPos;
    const auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;

    auto r = 1.0f;
    auto g = 1.0f;
    auto b = 1.0f;

    // auto words = text_elements(text);
    std::vector<ColorFormatInfo> keywords_ranges;
    auto tokens = tokenize(text);


    // FIXME: write a decent (this is trash) tokenizer/lexer that scans the source code.
    for (const auto &[begin, end, type] : tokens) {
        switch (type) {
            case TokenType::Qualifier:
            case TokenType::Keyword:
            case TokenType::Namespace:
            case TokenType::ParameterType:
                keywords_ranges.emplace_back(ColorFormatInfo{begin, end, {0.820f, 0.500f, 0.000f}});
                break;
            // case TokenType::Variable:
            // case TokenType::Parameter:
            // case TokenType::Function:
            case TokenType::StringLiteral:
                keywords_ranges.emplace_back(ColorFormatInfo{begin, end, DARKER_GREEN});
                break;
            case TokenType::NumberLiteral:
                keywords_ranges.emplace_back(ColorFormatInfo{begin, end, BLUE});
                break;
            case TokenType::Comment:
                keywords_ranges.emplace_back(ColorFormatInfo{begin, end, LIGHT_GRAY});
                break;
            case TokenType::Macro:
                keywords_ranges.emplace_back(ColorFormatInfo{begin, end, YELLOW});
                break;
            case TokenType::Include:
                keywords_ranges.emplace_back(ColorFormatInfo{begin, end, DARKER_GREEN});
                break;
            default:
                break;
        }
    }

    // iterate through all characters
    auto item_it = keywords_ranges.begin();
    auto pos = 0;
    bool have_text = !text.empty();
    auto xpos = float(x);
    auto ypos = float(y);
    if (not have_text) {
        view_cursor->update_cursor_data(static_cast<GLfloat>(x), static_cast<GLfloat>(y - 6));
    } else {
        // TODO(optimization): change so that instead of doing IF-THEN_ELSE inside this for loop for every character
        //  make it so, that it checks IF we are inside range, then draw the data up until last character, then iterate 1 step
        //  and check again
        for (auto c = text.begin(); c != text.end(); c++, pos++, data_index_pos--, data_index_pos_end--) {
            if (item_it != keywords_ranges.end()) {
                auto &kw = *item_it;
                auto [begin, end, col] = *item_it;
                if (pos > end) {
                    item_it++;
                    if (item_it != keywords_ranges.end()) {
                        begin = item_it->begin;
                        end = item_it->end;
                        col = item_it->color;
                    }
                }
                if (pos >= begin && pos < end) {// handled syntax color
                    r = col.x;
                    g = col.y;
                    b = col.z;
                } else {// default text color
                    r = 1;
                    g = 1;
                    b = 1;
                }
                if (pos >= end && item_it != keywords_ranges.end()) item_it++;
            }
            auto &glyph = this->glyph_cache[*c];
            if (*c == '\n') {
                if (data_index_pos == 0) {
                    if (bufPtr->mark_set) {
                        cx1 = x;
                        cy1 = y - 6;
                    } else {
                        view_cursor->update_cursor_data(x, y - 6);
                    }
                }
                if (data_index_pos_end == 0) { cx2 = x; }
                x = start_x;
                y -= row_height;
                continue;
            }
            xpos = float(x) + static_cast<float>(glyph.bearing.x);
            ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
            const auto x0 = float(glyph.x0) / float(t->width);
            const auto x1 = float(glyph.x1) / float(t->width);
            const auto y0 = float(glyph.y0) / float(t->height);
            const auto y1 = float(glyph.y1) / float(t->height);

            const auto w = glyph_width(glyph);
            const auto h = glyph_height(glyph);
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos, x0, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos + h, x1, y0, r, g, b});
            if (data_index_pos == 0) {
                if (bufPtr->mark_set) {
                    cx1 = x;
                    cy1 = y - 6;
                } else {
                    view_cursor->update_cursor_data(xpos, y - 6);
                }
            }
            if (data_index_pos_end == 0) { cx2 = x; }
            x += glyph.advance;
        }
    }

    if (bufPtr->mark_set) {
        if (cursor_b.pos == text.size()) cx2 = x;
        // TODO: implement multi-line selection. selecting multiple lines on the backend is super-easy as the data
        //  structure is simply a 1-dimensional stream of characters, displaying it properly isn't as easy
        //  and there are multiple ways to represent this. We can push "quads" to a vector, one per each line
        //  or we can do like in some editors and not have the "selection" visualization at all, but just leave kind of like
        //  an empty [] half-transparent marker where the selection begins (kind of like how 4coder does it)
        view_cursor->set_line_rect(cx1, cx2, cy1);
    } else if (view->get_text_buffer()->get_cursor_pos() == view->get_text_buffer()->size()) {
        xpos = float(x);
        ypos = float(y);
        view_cursor->update_cursor_data(xpos, y - 6);
    }
}

void SimpleFont::emplace_colorized_text_gpu_data(TextVertexArrayObject *vao, std::string_view text, int xPos, int yPos,
                                                 std::optional<std::vector<ColorizeTextRange>> colorData) {

    // FN_MICRO_BENCH();

    vao->vbo->data.clear();
    vao->vbo->data.reserve(gpu_mem_required_for_quads<TextVertex>(text.size()));
    auto &store = vao->vbo->data;
    const auto start_x = xPos;
    const auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;
    auto defaultColor = Vec3f{0.84f, 0.725f, 0.66f};
    auto r = defaultColor.x;
    auto g = defaultColor.y;
    auto b = defaultColor.z;

    if (colorData) {
        auto cd = colorData.value();
        auto colorInfo = cd.front();

        for (auto &cInfo : cd) {
            r = cInfo.color.x;
            g = cInfo.color.y;
            b = cInfo.color.z;
            auto end = std::min(cInfo.begin + cInfo.length, text.size());
            for (auto idx = cInfo.begin; idx < end; idx++) {
                const auto &glyph = this->glyph_cache[text[idx]];
                if (text[idx] == '\n') {
                    x = start_x;
                    y -= row_height;
                    continue;
                }
                const auto xpos = float(x) + glyph.bearing.x;
                const auto ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
                const auto x0 = float(glyph.x0) / float(t->width);
                const auto x1 = float(glyph.x1) / float(t->width);
                const auto y0 = float(glyph.y0) / float(t->height);
                const auto y1 = float(glyph.y1) / float(t->height);
                const auto w = glyph_width(glyph);
                const auto h = glyph_height(glyph);
                store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
                store.emplace_back(TextVertex{xpos, ypos, x0, y1, r, g, b});
                store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
                store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
                store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
                store.emplace_back(TextVertex{xpos + w, ypos + h, x1, y0, r, g, b});
                x += glyph.advance;
            }
        }
    } else {
        auto data_index = 0;
        for (char c : text) {
            auto &glyph = this->glyph_cache[c];
            if (c == '\n') {
                x = start_x;
                y -= row_height;
                continue;
            }
            const auto xpos = float(x) + static_cast<float>(glyph.bearing.x);
            const auto ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
            const auto x0 = float(glyph.x0) / float(t->width);
            const auto x1 = float(glyph.x1) / float(t->width);
            const auto y0 = float(glyph.y0) / float(t->height);
            const auto y1 = float(glyph.y1) / float(t->height);
            const auto w = glyph_width(glyph);
            const auto h = glyph_height(glyph);
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos, x0, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos + h, x1, y0, r, g, b});
            x += glyph.advance;
            data_index++;
        }
    }
}

void SimpleFont::add_colorized_text_gpu_data(TextVertexArrayObject *vao, std::vector<TextDrawable> textDrawables) {

    auto count_chars_in_drawables = std::accumulate(textDrawables.begin(), textDrawables.end(), 0, [](auto acc, auto el) {
        return acc + el.text.size();
    });

    vao->vbo->data.clear();
    vao->vbo->data.reserve(gpu_mem_required_for_quads<TextVertex>(count_chars_in_drawables));
    auto &store = vao->vbo->data;
    auto defaultColor = Vec3f{0.84f, 0.725f, 0.66f};

    for(const auto& drawable : textDrawables) {
        auto [r,g,b] = drawable.color.value_or(defaultColor);
        auto x = drawable.xpos;
        auto y = drawable.ypos;
        auto data_index = 0;
        for (char c : drawable.text) {
            auto &glyph = this->glyph_cache[c];
            if (c == '\n') {
                x = drawable.xpos;
                y -= row_height;
                continue;
            }
            auto xpos = float(x) + glyph.bearing.x;
            auto ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);

            const auto x0 = float(glyph.x0) / float(t->width);
            const auto x1 = float(glyph.x1) / float(t->width);
            const auto y0 = float(glyph.y0) / float(t->height);
            const auto y1 = float(glyph.y1) / float(t->height);
            const auto w = glyph_width(glyph);
            const auto h = glyph_height(glyph);
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos, x0, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos + h, x1, y0, r, g, b});
            x += glyph.advance;
            data_index++;
        }
    }
}

void SimpleFont::create_vertex_data_no_highlighting(ui::View *view, ui::core::ScreenPos startingTopLeftPos) {
    // FN_MICRO_BENCH();
    auto buf = view->get_text_buffer();
    auto character_start = buf->meta_data.line_begins[view->cursor->views_top_line];
    auto char_end = 0;
    if(view->cursor->views_top_line + view->lines_displayable >= buf->meta_data.line_begins.size() - 1) {
        char_end = buf->size();
    } else {
        auto end_line = view->cursor->views_top_line + view->lines_displayable;
        char_end = buf->meta_data.line_begins[end_line+1];
    }
    auto total_characters = char_end - character_start;
    auto reserve = total_characters * 2;

    auto total_text = view->get_text_buffer()->to_string_view();
    auto view_cursor = view->get_cursor();
    // TODO(use cy2 for when we select multiple lines): right now only one line can be selected, which is why cy2 is not used
    GLfloat cx1, cx2, cy1, cy2;
    auto bufPtr = view->get_text_buffer();

    auto [cursor_a, cursor_b] = bufPtr->get_cursor_rect();
    int data_index_pos = cursor_a.pos - character_start;
    int data_index_pos_end = cursor_b.pos - character_start;

    view->vao->vbo->data.clear();
    view->vao->vbo->data.reserve(gpu_mem_required_for_quads<TextVertex>(reserve));
    auto &store = view->vao->vbo->data;
    const auto[start_x, start_y] = startingTopLeftPos;
    auto x = start_x;
    auto y = start_y;

    auto r = 1.0f;
    auto g = 1.0f;
    auto b = 1.0f;

    // auto words = text_elements(text);
    // auto tokens = tokenize(text);
    // keywords_ranges.reserve(tokens.size());

    std::string_view text{total_text.data() + character_start, (unsigned)total_characters};
    auto pos = character_start;
    bool have_text = !text.empty();
    auto xpos = float(x);
    auto ypos = float(y);
    if (not have_text) {
        view_cursor->update_cursor_data(x, y - 6);
    } else {
        // TODO(optimization): change so that instead of doing IF-THEN_ELSE inside this for loop for every character
        //  make it so, that it checks IF we are inside range, then draw the data up until last character, then iterate 1 step
        //  and check again
        for (auto c = text.begin(); c != text.end(); c++, pos++, data_index_pos--, data_index_pos_end--) {
            auto &glyph = this->glyph_cache[*c];
            if (*c == '\n') {
                if (data_index_pos == 0) {
                    if (bufPtr->mark_set) {
                        cx1 = x;
                        cy1 = y - 6;
                    } else {
                        view_cursor->update_cursor_data(x, y - 6);
                    }
                }
                if (data_index_pos_end == 0) { cx2 = x; }
                x = start_x;
                y -= row_height;
                continue;
            }
            xpos = float(x) + glyph.bearing.x;
            ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
            const auto x0 = float(glyph.x0) / float(t->width);
            const auto x1 = float(glyph.x1) / float(t->width);
            const auto y0 = float(glyph.y0) / float(t->height);
            const auto y1 = float(glyph.y1) / float(t->height);
            const auto w = glyph_width(glyph);
            const auto h = glyph_height(glyph);
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos, x0, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos + h, x1, y0, r, g, b});
            if (data_index_pos == 0) {
                if (bufPtr->mark_set) {
                    cx1 = x;
                    cy1 = y - 6;
                } else {
                    view_cursor->update_cursor_data(xpos, y - 6);
                }
            }
            if (data_index_pos_end == 0) { cx2 = x; }
            x += glyph.advance;
        }
    }

    if (bufPtr->mark_set) {
        if (cursor_b.pos == text.size()) cx2 = x;
        // TODO: implement multi-line selection. selecting multiple lines on the backend is super-easy as the data
        //  structure is simply a 1-dimensional stream of characters, displaying it properly isn't as easy
        //  and there are multiple ways to represent this. We can push "quads" to a vector, one per each line
        //  or we can do like in some editors and not have the "selection" visualization at all, but just leave kind of like
        //  an empty [] half-transparent marker where the selection begins (kind of like how 4coder does it)
        view_cursor->set_line_rect(cx1, cx2, cy1);
    } else if (view->get_text_buffer()->get_cursor_pos() == view->get_text_buffer()->size()) {
        xpos = float(x);
        ypos = float(y);
        view_cursor->update_cursor_data(xpos, y - 6);
    }
}


void SimpleFont::create_vertex_data_for_syntax(ui::View* view, const ui::core::ScreenPos startingTopLeftPos) {
    // FN_MICRO_BENCH();
    auto buf = view->get_text_buffer();
    auto character_start = buf->meta_data.line_begins[view->cursor->views_top_line];


    // Calculate the character range, that currently is displayable on the screen.
    // We use the amount of lines the view can display, it's current "top line" and where that points to
    // in the buffer. we get this info from out of buf->meta_data, which consists of indices where line beginnings are.
    auto char_end = 0;
    if(view->cursor->views_top_line + view->lines_displayable >= buf->meta_data.line_begins.size() - 1) {
        char_end = buf->size();
    } else {
        auto end_line = view->cursor->views_top_line + view->lines_displayable;
        char_end = buf->meta_data.line_begins[end_line+1];
    }

    auto total_characters = char_end - character_start;
    auto reserve = total_characters * 2;

    auto total_text = view->get_text_buffer()->to_string_view();
    auto view_cursor = view->get_cursor();
    // TODO(use cy2 for when we select multiple lines): right now only one line can be selected, which is why cy2 is not used
    GLfloat cx1, cx2, cy1, cy2;
    auto bufPtr = view->get_text_buffer();

    auto [cursor_a, cursor_b] = bufPtr->get_cursor_rect();
    int data_index_pos = cursor_a.pos - character_start;
    int data_index_pos_end = cursor_b.pos - character_start;

    view->vao->vbo->data.clear();
    view->vao->vbo->data.reserve(gpu_mem_required_for_quads<TextVertex>(reserve));
    auto &store = view->vao->vbo->data;
    const auto[start_x, start_y] = startingTopLeftPos;
    auto x = start_x;
    auto y = start_y;

    auto r = 1.0f;
    auto g = 1.0f;
    auto b = 1.0f;

    // auto words = text_elements(text);
    // auto tokens = tokenize(text);
    // keywords_ranges.reserve(tokens.size());

    std::string_view text{total_text.data() + character_start, (unsigned)total_characters};
    auto formatted_tokens = color_format_tokenize_range(total_text.data() + character_start, total_characters, character_start);
    auto item_it = formatted_tokens.begin();
    auto pos = character_start;
    bool have_text = !text.empty();
    auto xpos = float(x);
    auto ypos = float(y);
    if (not have_text) {
        view_cursor->update_cursor_data(x, y - 6);
    } else {
        // TODO(optimization): change so that instead of doing IF-THEN_ELSE inside this for loop for every character
        //  make it so, that it checks IF we are inside range, then draw the data up until last character, then iterate 1 step
        //  and check again
        for (auto c = text.begin(); c != text.end(); c++, pos++, data_index_pos--, data_index_pos_end--) {
            if (item_it != formatted_tokens.end()) {
                auto &kw = *item_it;
                auto [begin, end, col] = *item_it;
                if (pos > end) {
                    item_it++;
                    if (item_it != formatted_tokens.end()) {
                        begin = item_it->begin;
                        end = item_it->end;
                        col = item_it->color;
                    }
                }
                if (pos >= begin && pos < end) {// handled syntax color
                    r = col.x;
                    g = col.y;
                    b = col.z;
                } else {// default text color
                    r = 1;
                    g = 1;
                    b = 1;
                }
                if (pos >= end && item_it != formatted_tokens.end()) item_it++;
            }
            auto &glyph = this->glyph_cache[*c];
            // If c == newline, we do some setup, and skip this loop iteration, since we don't draw those
            if (*c == '\n') {
                if (data_index_pos == 0) {
                    if (bufPtr->mark_set) {
                        cx1 = x;
                        cy1 = y - 6;
                    } else {
                        view_cursor->update_cursor_data(x, y - 6);
                    }
                }
                if (data_index_pos_end == 0) { cx2 = x; }
                x = start_x;
                y -= row_height;
                continue;
            }
            xpos = float(x) + glyph.bearing.x;
            ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
            const auto x0 = float(glyph.x0) / float(t->width);
            const auto x1 = float(glyph.x1) / float(t->width);
            const auto y0 = float(glyph.y0) / float(t->height);
            const auto y1 = float(glyph.y1) / float(t->height);
            const auto w = glyph_width(glyph);
            const auto h = glyph_height(glyph);
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos, x0, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
            store.emplace_back(TextVertex{xpos + w, ypos + h, x1, y0, r, g, b});
            if (data_index_pos == 0) {
                if (bufPtr->mark_set) {
                    cx1 = x;
                    cy1 = y - 6;
                } else {
                    view_cursor->update_cursor_data(xpos, y - 6);
                }
            }
            if (data_index_pos_end == 0) { cx2 = x; }
            x += glyph.advance;
        }
    }

    if (bufPtr->mark_set) {
        if (cursor_b.pos == text.size()) cx2 = x;
        // TODO: implement multi-line selection. selecting multiple lines on the backend is super-easy as the data
        //  structure is simply a 1-dimensional stream of characters, displaying it properly isn't as easy
        //  and there are multiple ways to represent this. We can push "quads" to a vector, one per each line
        //  or we can do like in some editors and not have the "selection" visualization at all, but just leave kind of like
        //  an empty [] half-transparent marker where the selection begins (kind of like how 4coder does it)
        view_cursor->set_line_rect(cx1, cx2, cy1);
    } else if (view->get_text_buffer()->get_cursor_pos() == view->get_text_buffer()->size()) {
        xpos = float(x);
        ypos = float(y);
        view_cursor->update_cursor_data(xpos, y - 6);
    }
}

void SimpleFont::create_vertex_data_for_only_visible(ui::View *view, ui::core::ScreenPos startingTopLeftPos) {
    auto buf = view->get_text_buffer();
    auto buf_curs = view->get_text_buffer()->get_cursor();
    auto top_line = std::max(view->cursor->views_top_line - 40, 0);
    auto total_lines = view->lines_displayable;
    auto bottom_line = top_line + total_lines + 40;
    assert(row_height == view->font->get_pixel_row_advance());

    if(view->get_text_buffer()->meta_data.line_begins.size() > total_lines) {
        for(auto i = std::max(top_line, 1); i <= bottom_line; i++) {
            if(((StdStringBuffer*)buf)->store[buf->meta_data.line_begins[i]-1] != '\n') {
                util::println("{}", ((StdStringBuffer*)buf)->store[buf->meta_data.line_begins[i]]);
            }
        }
        auto total_text = view->get_text_buffer()->to_string_view();
        auto view_cursor = view->get_cursor();
        // TODO(use cy2 for when we select multiple lines): right now only one line can be selected, which is why cy2 is not used
        GLfloat cx1, cx2, cy1, cy2;
        auto bufPtr = view->get_text_buffer();

        auto [cursor_a, cursor_b] = bufPtr->get_cursor_rect();
        int data_index_pos = cursor_a.pos;
        int data_index_pos_end = cursor_b.pos;

        auto char_range_offset = view->get_text_buffer()->meta_data.line_begins[top_line];
        auto char_range_end = view->get_text_buffer()->meta_data.line_begins[bottom_line+1];
        auto characters_total = char_range_end - char_range_offset;

        view->vao->vbo->data.clear();
        view->vao->vbo->data.reserve(gpu_mem_required_for_quads<TextVertex>(characters_total));

        auto &store = view->vao->vbo->data;
        const auto[start_x, start_y] = startingTopLeftPos;
        auto x = start_x;
        auto y = start_y;

        auto r = 1.0f;
        auto g = 1.0f;
        auto b = 1.0f;

        // auto words = text_elements(text);
        // auto tokens = tokenize(text);
        // keywords_ranges.reserve(tokens.size());

        std::string_view text{total_text.data() + char_range_offset, (unsigned)characters_total};
        auto formatted_tokens = color_format_tokenize_range(total_text.data() + char_range_offset, characters_total, char_range_offset);

        auto item_it = formatted_tokens.begin();
        auto pos = char_range_offset;
        bool have_text = !total_text.empty();
        auto xpos = float(x);
        auto ypos = float(y) - float(top_line * view->font->get_pixel_row_advance());
        if (not have_text) {
            view_cursor->update_cursor_data(x, y - 6);
        } else {
            // TODO(optimization): change so that instead of doing IF-THEN_ELSE inside this for loop for every character
            //  make it so, that it checks IF we are inside range, then draw the data up until last character, then iterate 1 step
            //  and check again
            for (auto c = text.begin(); c != text.end(); c++, pos++, data_index_pos--, data_index_pos_end--) {
                if (item_it != formatted_tokens.end()) {
                    auto &kw = *item_it;
                    auto [begin, end, col] = *item_it;
                    if (pos > end) {
                        item_it++;
                        if (item_it != formatted_tokens.end()) {
                            begin = item_it->begin;
                            end = item_it->end;
                            col = item_it->color;
                        }
                    }
                    if (pos >= begin && pos < end) {// handled syntax color
                        r = col.x;
                        g = col.y;
                        b = col.z;
                    } else {// default text color
                        r = 1;
                        g = 1;
                        b = 1;
                    }
                    if (pos >= end && item_it != formatted_tokens.end()) item_it++;
                }
                auto &glyph = this->glyph_cache[*c];
                if (*c == '\n') {
                    if (data_index_pos == 0) {
                        if (bufPtr->mark_set) {
                            cx1 = x;
                            cy1 = y - 6;
                        } else {
                            view_cursor->update_cursor_data(x, y - 6);
                        }
                    }
                    if (data_index_pos_end == 0) { cx2 = x; }
                    x = start_x;
                    y -= view->font->get_pixel_row_advance();
                    continue;
                }
                xpos = float(x) + glyph.bearing.x;
                ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
                const auto x0 = float(glyph.x0) / float(t->width);
                const auto x1 = float(glyph.x1) / float(t->width);
                const auto y0 = float(glyph.y0) / float(t->height);
                const auto y1 = float(glyph.y1) / float(t->height);
                const auto w = glyph_width(glyph);
                const auto h = glyph_height(glyph);
                store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
                store.emplace_back(TextVertex{xpos, ypos, x0, y1, r, g, b});
                store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
                store.emplace_back(TextVertex{xpos, ypos + h, x0, y0, r, g, b});
                store.emplace_back(TextVertex{xpos + w, ypos, x1, y1, r, g, b});
                store.emplace_back(TextVertex{xpos + w, ypos + h, x1, y0, r, g, b});
                if (data_index_pos == 0) {
                    if (bufPtr->mark_set) {
                        cx1 = x;
                        cy1 = y - 6;
                    } else {
                        view_cursor->update_cursor_data(xpos, y - 6);
                    }
                }
                if (data_index_pos_end == 0) { cx2 = x; }
                x += glyph.advance;
            }
        }

        if (bufPtr->mark_set) {
            if (cursor_b.pos == text.size()) cx2 = x;
            // TODO: implement multi-line selection. selecting multiple lines on the backend is super-easy as the data
            //  structure is simply a 1-dimensional stream of characters, displaying it properly isn't as easy
            //  and there are multiple ways to represent this. We can push "quads" to a vector, one per each line
            //  or we can do like in some editors and not have the "selection" visualization at all, but just leave kind of like
            //  an empty [] half-transparent marker where the selection begins (kind of like how 4coder does it)
            view_cursor->set_line_rect(cx1, cx2, cy1);
        } else if (view->get_text_buffer()->get_cursor_pos() == view->get_text_buffer()->size()) {
            xpos = float(x);
            ypos = float(y);
            view_cursor->update_cursor_data(xpos, y - 6);
        }
    } else {
        create_vertex_data_for_syntax(view, startingTopLeftPos);
    }
}


int SimpleFont::calculate_text_width(std::string_view str) {
    auto width_in_pixels = 0;
    auto acc = 0;

    for(const auto& c : str) {
        auto &glyph = glyph_cache[c];
        acc += glyph.advance;
        if(c == '\n') {
            width_in_pixels = std::max(acc, width_in_pixels);
            acc = 0;
        }
    }
    return std::max(width_in_pixels, acc);
}
int SimpleFont::get_pixel_size() const {
    return pixel_size;
}

int SimpleFont::get_next_line_y_position_of(int y_pos) const { return y_pos + get_pixel_row_advance(); }

static inline float glyph_width(const glyph_info &g) {
    return static_cast<float>(g.x1) - static_cast<float>(g.x0);
}

static inline float glyph_height(const glyph_info &g) {
    return static_cast<float>(g.y1) - static_cast<float>(g.y0);
}
