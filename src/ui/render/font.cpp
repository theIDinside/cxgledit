//
// Created by 46769 on 2020-12-20.
//

// App headers
#include "font.hpp"
#include <core/core.hpp>
#include <ui/view.hpp>

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

        glyph_cache.push_back(glyphInfo);
        pen_x += bmp->width + 1;
    }
    fmt::print("Max glyph height was: {}\n", max_glyph_height);
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

    auto png_filename = fmt::format("{}_output.png", path);

    stbi_write_png(png_filename.c_str(), tex_width, tex_height, 4, png_data, tex_width * 4);

    free(png_data);
    free(pixels);

    auto font = std::make_unique<SimpleFont>(SimpleFont{pixel_size, std::move(texture), std::move(glyph_cache)});
    font->row_height = row_advance;
    font->max_glyph_width = max_glyph_width;
    font->max_glyph_height = max_glyph_height;

    return font;
}

SimpleFont::SimpleFont(int pixelSize, std::unique_ptr<Texture> &&texture, std::vector<glyph_info> &&glyphs)
    : pixel_size(pixelSize), t(std::move(texture)), glyph_cache(std::move(glyphs)) {}

static int test_count = 0;

int SimpleFont::get_row_advance() const { return row_advance; }

static bool exampleHowToLexVar = false;


void SimpleFont::emplace_source_text_gpu_data(VAO *vao, ui::View *view, int xPos, int yPos) {
    FN_MICRO_BENCH();
    auto text = view->get_text_buffer()->to_string_view();
    auto view_cursor = view->get_cursor();

    GLfloat cx1, cx2, cy1, cy2;
    auto bufPtr = view->get_text_buffer();

    auto [idx_curs_b, idx_curs_e] = bufPtr->get_mark_index_range();
    auto [cursor_a, cursor_b] = bufPtr->get_cursor_rect();

    int data_index_pos = cursor_a.pos;
    int data_index_pos_end = cursor_b.pos;

    vao->vbo->data.clear();
    vao->vbo->data.reserve(text.size() * sizeof(TextVertex));
    auto &store = vao->vbo->data;
    auto start_x = xPos;
    auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;
    auto r = 0.2f;
    auto g = 0.325f;
    auto b = 0.75f;

    auto words = text_elements(text);
    std::vector<ColorFormatInfo> keywords_ranges;

    // Some external condition, being set by another context or thread, as a hook, letting us know that, we got new visual data
    // or meta data concerning this text buffer, perhaps a lexer is finished with it's job and has produced data so we can display it
    if(exampleHowToLexVar) {

    }

    for (const auto &[begin, end] : words) {
        auto rng = text.substr(begin, end - begin);
        if (rng.begin() != rng.end() && *rng.begin() == '"') {
            keywords_ranges.emplace_back(ColorFormatInfo{begin, end, DARKER_GREEN});
        } else {
            for (const auto &[word, color] : keywords) {
                if (word == rng) { keywords_ranges.emplace_back(ColorFormatInfo{begin, end, color}); }
            }
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
            }
            auto &glyph = this->glyph_cache[*c];
            if (*c == '\n') {
                if (data_index_pos == 0) {
                    if(bufPtr->mark_set) {
                        cx1 = x;
                        cy1 = y - 6;
                    } else {
                        view_cursor->update_cursor_data(x, y - 6);
                    }
                }
                if(data_index_pos_end == 0) {
                    cx2 = x;
                }
                x = start_x;
                y -= row_advance;
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
                if(bufPtr->mark_set) {
                    cx1 = x;
                    cy1 = y - 6;
                } else {
                    view_cursor->update_cursor_data(xpos, y - 6);
                }
            }
            if(data_index_pos_end == 0) {
                cx2 = x;
            }
            x += glyph.advance;
        }
    }

    if(bufPtr->mark_set) {
        util::println("Mark set between {} - {}. Screen coords: x1, x2: {} -> {}", cursor_a.pos, cursor_b.pos, cx1, cx2);
        view_cursor->set_line_rect(cx1, cx2, cy1);
    }

    if (view->get_text_buffer()->get_cursor_pos() == view->get_text_buffer()->size()) {
        xpos = float(x);
        ypos = float(y);
        view_cursor->update_cursor_data(xpos, y - 6);
    }
}

void SimpleFont::emplace_colorized_text_gpu_data(VAO *vao, TextData *text_buffer, int xPos, int yPos,
                                                 std::optional<std::vector<ColorizeTextRange>> colorData) {
    // FN_MICRO_BENCH();
    auto text = text_buffer->to_string_view();
    auto cursor_char_pos = text_buffer->get_cursor_pos();

    vao->vbo->data.clear();
    vao->vbo->data.reserve(text.size() * sizeof(TextVertex));
    auto &store = vao->vbo->data;
    auto start_x = xPos;
    auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;
    auto defaultColor = glm::fvec3{0.2f, 0.325f, 0.75f};
    auto r = defaultColor.x;
    auto g = defaultColor.y;
    auto b = defaultColor.z;

    if (colorData) {
        auto cd = colorData.value();
        auto colorInfo = cd.front();
        auto curr = 0;
        for (auto &cInfo : cd) {
            r = defaultColor.x;
            g = defaultColor.y;
            b = defaultColor.z;
            for (auto i = curr; i < colorInfo.begin; i++) {
                auto &glyph = this->glyph_cache[text[i]];
                if (text[i] == '\n') {
                    x = start_x;
                    y -= row_advance;
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
            auto ce = colorInfo.begin + colorInfo.length;
            r = colorInfo.color.x;
            g = colorInfo.color.y;
            b = colorInfo.color.z;
            for (auto i = colorInfo.begin; i < ce; i++) {
                auto &glyph = this->glyph_cache[text[i]];
                if (text[i] == '\n') {
                    x = start_x;
                    y -= row_advance;
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
            curr = ce + 1;
        }
        if (curr < text.size()) {
            r = defaultColor.x;
            g = defaultColor.y;
            b = defaultColor.z;
            for (auto i = curr; i < text.size(); i++) {
                auto &glyph = this->glyph_cache[text[i]];
                if (text[i] == '\n') {
                    x = start_x;
                    y -= row_advance;
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
                y -= row_advance;
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
void SimpleFont::emplace_colorized_text_gpu_data(VAO *vao, std::string_view text, int xPos, int yPos,
                                                 std::optional<std::vector<ColorizeTextRange>> colorData) {

    // FN_MICRO_BENCH();

    vao->vbo->data.clear();
    vao->vbo->data.reserve(text.size() * sizeof(TextVertex));
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

        for(auto& cInfo : cd) {
            r = cInfo.color.x;
            g = cInfo.color.y;
            b = cInfo.color.z;
            auto end = cInfo.begin + cInfo.length;
            for(auto idx = cInfo.begin; idx < end; idx++) {
                auto &glyph = this->glyph_cache[text[idx]];
                if (text[idx] == '\n') {
                    x = start_x;
                    y -= row_advance;
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
                y -= row_advance;
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

enum class LexType {
    Unknown = -1,
    // Literals
    String,
    Number,
    // Types
    Type,
    Primitive,
    // Identifiers
    Variable,
    LineComment,
    BlockComment,
    KeyWord,
    Macro,
    Include,
};

struct Lexeme {
    int begin, length;
    LexType type;
};

//
// inline constexpr auto length = [](auto begin, auto end) { return end - begin; };

std::vector<Lexeme> lex_text(std::string_view text) {
    auto current_type = LexType::Unknown;
    auto last_lexed = LexType::Unknown;
    std::vector<Lexeme> results{};

    // For some reason I keep mentally struggling with this absolutely simple thing, which means I have to double check
    // all the damn time, which is often, thus killing efficiency.
    constexpr auto length = [](auto begin, auto end) { return end - begin; };
    constexpr auto advance = [](auto &it, auto &idx) {
        ++it;
        ++idx;
    };
    const auto e = text.end();
    auto pos = 0;
    auto item_begin = 0;
    for (auto it = text.begin(); it != e; advance(it, pos)) {
        if (*it == '/') {
            if (current_type == LexType::LineComment) {// find newline and set Lexeme {
                for (; it != e && *it != '\n'; advance(it, pos)) {}
                last_lexed = LexType::LineComment;
                results.emplace_back(item_begin, length(item_begin, pos), last_lexed);
                current_type = LexType::Unknown;
            } else {
                item_begin = pos;
                current_type = LexType::LineComment;
            }
        } else if (*it == '"') {
            item_begin = pos;
            bool escaped = false;
            for (; it != e; advance(it, pos)) {
                if (*it == '"' && !escaped) break;
                else if (*it == '"' && escaped) {
                    escaped = false;
                } else if (*it == '\\') {
                    escaped = true;
                }
            }
            last_lexed = LexType::String;
            results.emplace_back(item_begin, length(item_begin, pos), last_lexed);
            current_type = LexType::Unknown;
        } else if (*it == '*') {
            if (current_type == LexType::LineComment) {
                current_type = LexType::BlockComment;
                for (; it != e; advance(it, pos)) {
                    if (*it == '*') {
                        auto next = it;
                        next++;
                        if (next != e && *next == '/') {
                            advance(it, pos);
                            break;
                        }
                    }
                }
                last_lexed = current_type;
                current_type = LexType::Unknown;
                results.emplace_back(item_begin, length(item_begin, pos), last_lexed);
            }
        } else if (*it == '#') {

        } else if (*it == '<' && last_lexed == LexType::Include) {
        }
    }
    return results;
}

std::vector<Words> text_elements(std::string_view text) {
    std::vector<Words> items;
    auto ptr = text.begin();
    auto end = text.end();
    std::vector<std::size_t> del;

    /*
     * if (*ptr == '"') {
            del.push_back(i);
            ptr++;
            i++;
            while (*ptr != '"' && ptr != end) {
                i++;
                ptr++;
            }
            del.push_back(i);
            ptr++;
            i++;
        } else
     */

    auto start = 0;
    for (auto i = 0; ptr != end; i++, ptr++) {
        if (!std::isalpha(*ptr) && *ptr != '_' && *ptr != '#') { del.push_back(i); }
    }
    auto begin = 0;
    for (auto it = del.begin(); it != del.end(); it++) {
        auto pos = *it;
        items.emplace_back(begin, pos);
        if (text[pos] == '"') {
            begin = pos;
        } else {
            begin = pos + 1;
        }
    }
    return items;
}
