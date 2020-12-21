//
// Created by 46769 on 2020-12-20.
//

#pragma once

#include <glm/glm.hpp>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "texture.hpp"
#include "vertex_buffer.hpp"

struct CharacterRange {
    unsigned int from, to;
};

#define NUM_GLYPHS 128

struct glyph_info {
    int x0, y0, x1, y1;	// coords of glyph in the texture atlas
    int x_off, y_off;   // left & top bearing when rendering
    int advance;        // x advance when rendering
    glm::ivec2 size, bearing;
};

static int row_advance = 0;

static glyph_info info[NUM_GLYPHS];

class SimpleFont {
public:
    // Static member vars
    static const int CHARACTERS_TEXTURE_SIZE; //!< Size of texture atlas (in pixels) that stores characters
    // Static member functions
    [[maybe_unused]] static std::unique_ptr<SimpleFont> setup_font(const std::string& path, int pixel_size, CharacterRange charRange = CharacterRange{.from = 32, .to = 255});
    SimpleFont(int pixelSize, std::unique_ptr<Texture>&& texture, std::vector<glyph_info>&& glyphs);

    TextVertices make_gpu_data(const std::string& text, int xpos, int ypos);
    std::unique_ptr<Texture> t{nullptr};

    std::vector<glyph_info> glyph_cache;

private:
    glyph_info* data = info;
    int pixel_size{};
};