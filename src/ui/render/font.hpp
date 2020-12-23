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

struct SyntaxColor {
    GLfloat r{0.0f}, g{0.0f}, b{0.0f};
};

template <typename T, typename U> constexpr auto mp(T t, U u) {
    return std::pair<T, U>{t, u};
}

constexpr glm::vec3 vred{1.0f, 0.0f, 0.0f};
constexpr glm::vec3 vgreen{0.f, 1.0f, 0.0f};
constexpr glm::vec3 vblue{0.0f, 0.0f, 1.0f,};
constexpr glm::vec3 vsc1{0.820f,0.500f,0.000f};
constexpr glm::vec3 vsc2{1.000f,0.300f,0.600f};
constexpr glm::vec3 vsc3{0.300f,0.231f,0.800f};
constexpr glm::vec3 vsc4{0.031f,0.921f,0.140f};
constexpr glm::vec3 vsc5{0.712f,1.000f,1.000f};

constexpr glm::vec3 c_keyword{0.820f,0.500f,0.000f};

constexpr auto YELLOW = glm::vec3{0.893f, 1.0f, 0.0f};
constexpr auto RED = glm::vec3{1.0f, 0.0f, 0.0f};
constexpr auto GREEN = glm::vec3{0.0f, 1.0f, 0.0f};
constexpr auto BLUE = glm::vec3{0.0f, 0.0f, 1.0f};
constexpr auto WHITE = glm::vec3{1.0f, 1.0f, 1.0f};

using HighLight = std::pair<const char*, glm::vec3>;
constexpr std::array keywords{mp("int", c_keyword),    mp("bool", c_keyword),
                              mp("void", c_keyword),   mp("long", c_keyword),
                              mp("double", c_keyword), mp("float", c_keyword),
                              mp("struct", c_keyword), mp("using", c_keyword),
                              mp("const", c_keyword), mp("char", c_keyword),
                              mp("constexpr", c_keyword), mp("auto", c_keyword),
                              mp("#define", YELLOW), mp("#include", YELLOW)};

constexpr std::array highlight{RED, GREEN, BLUE, YELLOW, WHITE};

struct Keyword {
    std::size_t begin;
    std::size_t end;
    glm::vec3 color;
};


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


struct Word {
    std::size_t begin, end;
};

std::vector<Word> text_elements(std::string_view text);

class SimpleFont {
public:
    static SyntaxColor colors[8];
    // Static member vars
    static const int CHARACTERS_TEXTURE_SIZE; //!< Size of texture atlas (in pixels) that stores characters
    // Static member functions
    [[maybe_unused]] static std::unique_ptr<SimpleFont> setup_font(const std::string& path, int pixel_size, CharacterRange charRange = CharacterRange{.from = 32, .to = 255});
    SimpleFont(int pixelSize, std::unique_ptr<Texture>&& texture, std::vector<glyph_info>&& glyphs);

    TextVertices make_gpu_data(const std::string& text, int xpos, int ypos);
    void emplace_gpu_data(VAO* vao, const std::string& text, int xPos, int yPos);
    void emplace_gpu_data(VAO* vao, std::string_view text, int xPos, int yPos);



    std::unique_ptr<Texture> t{nullptr};
    int get_row_advance() const;
    std::vector<glyph_info> glyph_cache;
    int row_height;
private:
    // glyph_info* data = info;
    int pixel_size{};
};