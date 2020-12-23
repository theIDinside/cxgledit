//
// Created by 46769 on 2020-12-20.
//

#include "font.hpp"

#include "texture.hpp"
#include "vertex_buffer.hpp"
#include <algorithm>
#include <fmt/core.h>
#include <vector>

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
    std::vector<glyph_info> glyph_cache;

    // for (int i = 0; i < NUM_GLYPHS; ++i) {
    for (int i = 0; i < charRange.to; ++i) {

        FT_Load_Char(face, i, FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
        FT_Bitmap *bmp = &face->glyph->bitmap;
        max_glyph_height = std::max(face->glyph->bitmap.rows, max_glyph_height);

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
    auto max_adv_y = max_glyph_height + 2;
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
    return font;
}

SimpleFont::SimpleFont(int pixelSize, std::unique_ptr<Texture> &&texture, std::vector<glyph_info> &&glyphs)
    : pixel_size(pixelSize), t(std::move(texture)), glyph_cache(std::move(glyphs)) {}

static int test_count = 0;

TextVertices SimpleFont::make_gpu_data(const std::string &text, int xPos, int yPos) {
    auto string_data = TextVertices::init_from_string(text);
    auto start_x = xPos;
    auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;
    auto vert_pointer_pos = string_data.data;
    for (const auto &c : text) {
        // auto &glyph = this->data[c];
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

        vert_pointer_pos[0] = TextVertex{xpos, ypos + h, x0, y0};
        vert_pointer_pos[1] = TextVertex{xpos, ypos, x0, y1};
        vert_pointer_pos[2] = TextVertex{xpos + w, ypos, x1, y1};

        vert_pointer_pos[3] = TextVertex{xpos, ypos + h, x0, y0};
        vert_pointer_pos[4] = TextVertex{xpos + w, ypos, x1, y1};
        vert_pointer_pos[5] = TextVertex{xpos + w, ypos + h, x1, y0};

        vert_pointer_pos += 6;
        x += glyph.advance;
    }
    return string_data;
}
int SimpleFont::get_row_advance() const { return row_advance; }
void SimpleFont::emplace_gpu_data(VAO *vao, const std::string &text, int xPos, int yPos) {
    vao->vbo->data.clear();
    vao->vbo->data.reserve(text.size() * 6);
    auto &store = vao->vbo->data;
    auto start_x = xPos;
    auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;
    auto r = 1.0f;
    auto g = 0.0f;
    auto b = 0.2f;

    std::vector<std::size_t>
            del; // delimiters are separated by whitespaces, (), {} or []

    for (const auto &c : text) {
        // auto &glyph = this->data[c];
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
    }
}

void SimpleFont::emplace_gpu_data(VAO *vao, std::string_view text, int xPos, int yPos) {
    vao->vbo->data.clear();
    vao->vbo->data.reserve(text.size() * 6);
    auto &store = vao->vbo->data;
    auto start_x = xPos;
    auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;
    auto r = 0.2f;
    auto g = 0.325f;
    auto b = 0.75f;

    auto words = text_elements(text);
    std::vector<Keyword> keywords_ranges;

    for (const auto &[begin, end] : words) {
        auto rng = text.substr(begin, end - begin);
        for (const auto &[word, color] : keywords) {
            if (word == rng) {
                keywords_ranges.emplace_back(Keyword{begin, end, color});
            }
        }
    }

    // iterate through all characters
    auto item_it = keywords_ranges.begin();
    auto pos = 0;
    bool syntax_set = false;

    auto data_index = 0;
    for (auto c = text.begin(); c != text.end(); c++, pos++) {
        if(item_it != keywords_ranges.end()) {
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
            if (pos >= begin && pos < end) { // handled syntax color
                r = col.x;
                g = col.y;
                b = col.z;
            } else {    // default text color
                r = 1;
                g = 1;
                b = 1;
            }
        }
        auto &glyph = this->glyph_cache[*c];
        if (*c == '\n') {
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
std::vector<Word> text_elements(std::string_view text) {
    std::vector<Word> items;
    auto ptr = text.begin();
    auto end = text.end();
    std::vector<std::size_t> del;

    for (auto i = 0; ptr != end; i++, ptr++) {
        if (!std::isalpha(*ptr) && *ptr != '_' && *ptr != '#') {
            del.push_back(i);
        }
    }
    auto begin = 0;
    for (auto pos : del) {
        items.emplace_back(begin, pos);
        begin = pos + 1;
    }
    return items;
}
