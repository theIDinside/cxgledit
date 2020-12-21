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

        // this is stuff you'd need when rendering individual glyphs out of the atlas

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
/*
        info[i].x0 = pen_x;
        info[i].y0 = pen_y;
        info[i].x1 = pen_x + bmp->width;
        info[i].y1 = pen_y + bmp->rows;

        info[i].x_off = face->glyph->bitmap_left;
        info[i].y_off = face->glyph->bitmap_top;
        info[i].advance = face->glyph->advance.x >> 6;
        info[i].bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        info[i].size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
*/


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
    stbi_write_png("font_output.png", tex_width, tex_height, 4, png_data, tex_width * 4);

    free(png_data);
    free(pixels);

    return std::make_unique<SimpleFont>(SimpleFont{pixel_size, std::move(texture), std::move(glyph_cache)});
}

SimpleFont::SimpleFont(int pixelSize, std::unique_ptr<Texture> &&texture, std::vector<glyph_info>&& glyphs) : pixel_size(pixelSize), t(std::move(texture)), glyph_cache(std::move(glyphs)) {}

static int test_count = 0;

TextVertices SimpleFont::make_gpu_data(const std::string &text, int xPos, int yPos) {
    auto string_data = TextVertices::init_from_string(text);


    auto start_x = xPos;
    auto start_y = yPos;
    auto x = start_x;
    auto y = start_y;
    /*
    float xpos = x + ch.bearing.x;
    float ypos = y - (ch.size.y - ch.bearing.y);

    // ch.bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top)
    // ch.size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows)
    //
    // vilket ger (x_off, y_off) = ch.bearing
    // och (_, y1) = ch.size
    h = y1 - y_off
    float w = ch.size.x * scale;
    float h = ch.size.y * scale;
    // update VBO for each character
    float vertices[6][4] = {{xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
                            {xpos + w, ypos, 1.0f, 1.0f},

                            {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
                            {xpos + w, ypos + h, 1.0f, 0.0f}};

    float ypos = y - (ch.size.y - ch.bearing.y);
    float ypos = y - (y_off - y1)
*/
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


    // assert(vertex_drawn - non_drawables*6 == string_data.vertex_count);
    return string_data;
}
