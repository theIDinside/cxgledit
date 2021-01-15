//
// Created by 46769 on 2020-12-20.
//

// App headers
#include "font.hpp"
#include <ui/syntax_highlighting.hpp>
#include <ui/view.hpp>
#include <ui/core/layout.hpp>

// Sys headers
#include <algorithm>
#include <vector>

// 3rd party headers
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

using u64 = std::size_t;

SyntaxColor red{.r = 1.0f};
SyntaxColor green{.g = 1.0f};
SyntaxColor blue{.b = 1.0f};
SyntaxColor sc1{.r = 1.0f, .g = 0.5f, .b = 0.0f};
SyntaxColor sc2{.r = 1.0f, .g = 0.5f, .b = 0.3f};
SyntaxColor sc3{.r = 0.3f, .g = 0.8f, .b = 0.8f};
SyntaxColor sc4{.r = 0.2f, .g = 0.9f, .b = 0.1f};
SyntaxColor sc5{.r = 0.7f, .g = 1.0f, .b = 1.0f};

SyntaxColor SimpleFont::colors[8]{red, green, blue, sc1, sc2, sc3, sc4, sc5};

std::unique_ptr<SimpleFont> SimpleFont::setup_font(const std::string &path, int pixel_size, CharacterRange charRange) {
    FT_Library ft;
    FT_Face face;
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, path.c_str(), 0, &face);
    FT_Set_Pixel_Sizes(face, 0, pixel_size);
    // FT_Set_Char_Size(face, 0, 16 << 6, 96, 96);

    // quick and dirty max texture size estimate

    auto total_possible_glyph_count = charRange.to - charRange.from;

    int max_dim = (1 + (face->size->metrics.height >> 6)) * ceilf(sqrtf(NUM_GLYPHS));
    int tex_width = 1;
    while (tex_width < max_dim) tex_width <<= 1;
    int tex_height = tex_width;

    // render glyphs to atlas

    auto *pixels = (unsigned char *) calloc(tex_width * tex_height, 1);
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
                .advance = face->glyph->advance.x >> 6,
                .size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                .bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        };

        max_bearing_size_diff = std::max(std::abs(glyphInfo.size.y - glyphInfo.bearing.y), max_bearing_size_diff);
        glyph_cache.push_back(glyphInfo);
        pen_x += bmp->width + 1;
    }
    auto max_adv_y = max_glyph_height + 5;
    row_advance = max_adv_y;
    auto texture = Texture::make_from_data(pixels, tex_width, tex_height, 1);
    FT_Done_FreeType(ft);

    char *png_data = (char *) calloc(tex_width * tex_height * 4, 1);
    for (int i = 0; i < (tex_width * tex_height); ++i) {
        png_data[i * 4 + 0] |= pixels[i];
        png_data[i * 4 + 1] |= pixels[i];
        png_data[i * 4 + 2] |= pixels[i];
        png_data[i * 4 + 3] = 0xff;
    }

    auto png_filename = fmt::format("{}_{}_output.png", path, pixel_size);

    stbi_write_png(png_filename.c_str(), tex_width, tex_height, 4, png_data, tex_width * 4);
    free(png_data);
    free(pixels);

    auto font = std::make_unique<SimpleFont>(SimpleFont{pixel_size, std::move(texture), std::move(glyph_cache)});
    font->row_height = row_advance;
    font->max_glyph_width = max_glyph_width;
    font->max_glyph_height = max_glyph_height;
    font->size_bearing_difference_max = max_bearing_size_diff;

    return font;
}

SimpleFont::SimpleFont(int pixelSize, std::unique_ptr<Texture> &&texture, std::vector<glyph_info> &&glyphs)
    : pixel_size(pixelSize), t(std::move(texture)), glyph_cache(std::move(glyphs)) {}

static int test_count = 0;

int SimpleFont::get_row_advance() const { return row_height; }

static bool exampleHowToLexVar = false;

void SimpleFont::create_vertex_data_in(VAO *vao, ui::View *view, int xPos, int yPos) {
    // FN_MICRO_BENCH();
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
    auto start_x = xPos;
    auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;

    auto r = 1.0f;
    auto g = 1.0f;
    auto b = 1.0f;

    // auto words = text_elements(text);
    auto tokens = tokenize(text);

    std::vector<ColorFormatInfo> keywords_ranges;

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
        view_cursor->update_cursor_data(x, y - 6);
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
            xpos = float(x) + glyph.bearing.x;
            ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
            auto x0 = float(glyph.x0) / float(t->width);
            auto x1 = float(glyph.x1) / float(t->width);
            auto y0 = float(glyph.y0) / float(t->height);
            auto y1 = float(glyph.y1) / float(t->height);
            auto w = float(glyph.x1 - glyph.x0);
            auto h = float(glyph.y1 - glyph.y0);
            store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
            store.emplace_back(xpos, ypos, x0, y1, r, g, b);
            store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
            store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
            store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
            store.emplace_back(xpos + w, ypos + h, x1, y0, r, g, b);
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

void SimpleFont::emplace_colorized_text_gpu_data(VAO *vao, std::string_view text, int xPos, int yPos,
                                                 std::optional<std::vector<ColorizeTextRange>> colorData) {

    // FN_MICRO_BENCH();

    vao->vbo->data.clear();
    vao->vbo->data.reserve(gpu_mem_required_for_quads<TextVertex>(text.size()));
    auto &store = vao->vbo->data;
    auto start_x = xPos;
    auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;
    auto defaultColor = glm::fvec3{0.84f, 0.725f, 0.66f};
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
                auto &glyph = this->glyph_cache[text[idx]];
                if (text[idx] == '\n') {
                    x = start_x;
                    y -= row_height;
                    continue;
                }
                auto xpos = float(x) + glyph.bearing.x;
                auto ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
                auto x0 = float(glyph.x0) / float(t->width);
                auto x1 = float(glyph.x1) / float(t->width);
                auto y0 = float(glyph.y0) / float(t->height);
                auto y1 = float(glyph.y1) / float(t->height);
                auto w = float(glyph.x1 - glyph.x0);
                auto h = float(glyph.y1 - glyph.y0);
                store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
                store.emplace_back(xpos, ypos, x0, y1, r, g, b);
                store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
                store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
                store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
                store.emplace_back(xpos + w, ypos + h, x1, y0, r, g, b);
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
            auto xpos = float(x) + glyph.bearing.x;
            auto ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
            auto x0 = float(glyph.x0) / float(t->width);
            auto x1 = float(glyph.x1) / float(t->width);
            auto y0 = float(glyph.y0) / float(t->height);
            auto y1 = float(glyph.y1) / float(t->height);
            auto w = float(glyph.x1 - glyph.x0);
            auto h = float(glyph.y1 - glyph.y0);
            store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
            store.emplace_back(xpos, ypos, x0, y1, r, g, b);
            store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
            store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
            store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
            store.emplace_back(xpos + w, ypos + h, x1, y0, r, g, b);
            x += glyph.advance;
            data_index++;
        }
    }
}

void SimpleFont::add_colorized_text_gpu_data(VAO *vao, std::vector<TextDrawable> textDrawables) {

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

            auto x0 = float(glyph.x0) / float(t->width);
            auto x1 = float(glyph.x1) / float(t->width);
            auto y0 = float(glyph.y0) / float(t->height);
            auto y1 = float(glyph.y1) / float(t->height);
            auto w = float(glyph.x1 - glyph.x0);
            auto h = float(glyph.y1 - glyph.y0);
            store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
            store.emplace_back(xpos, ypos, x0, y1, r, g, b);
            store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
            store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
            store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
            store.emplace_back(xpos + w, ypos + h, x1, y0, r, g, b);
            x += glyph.advance;
            data_index++;
        }
    }
}

void SimpleFont::create_vertex_data_for(ui::View* view, const ui::core::ScreenPos startingTopLeftPos) {
    // FN_MICRO_BENCH();
    auto text = view->get_text_buffer()->to_string_view();
    auto view_cursor = view->get_cursor();

    // TODO(use cy2 for when we select multiple lines): right now only one line can be selected, which is why cy2 is not used
    GLfloat cx1, cx2, cy1, cy2;
    auto bufPtr = view->get_text_buffer();

    auto [cursor_a, cursor_b] = bufPtr->get_cursor_rect();
    int data_index_pos = cursor_a.pos;
    int data_index_pos_end = cursor_b.pos;

    view->vao->vbo->data.clear();
    view->vao->vbo->data.reserve(gpu_mem_required_for_quads<TextVertex>(text.size()));
    auto &store = view->vao->vbo->data;
    auto[start_x, start_y] = startingTopLeftPos;
    auto x = start_x;
    auto y = start_y;

    auto r = 1.0f;
    auto g = 1.0f;
    auto b = 1.0f;

    // auto words = text_elements(text);
    auto tokens = tokenize(text);

    std::vector<ColorFormatInfo> keywords_ranges;

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
        view_cursor->update_cursor_data(x, y - 6);
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
            xpos = float(x) + glyph.bearing.x;
            ypos = float(y) - static_cast<float>(glyph.size.y - glyph.bearing.y);
            auto x0 = float(glyph.x0) / float(t->width);
            auto x1 = float(glyph.x1) / float(t->width);
            auto y0 = float(glyph.y0) / float(t->height);
            auto y1 = float(glyph.y1) / float(t->height);
            auto w = float(glyph.x1 - glyph.x0);
            auto h = float(glyph.y1 - glyph.y0);
            store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
            store.emplace_back(xpos, ypos, x0, y1, r, g, b);
            store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
            store.emplace_back(xpos, ypos + h, x0, y0, r, g, b);
            store.emplace_back(xpos + w, ypos, x1, y1, r, g, b);
            store.emplace_back(xpos + w, ypos + h, x1, y0, r, g, b);
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

