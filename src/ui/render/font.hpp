//
// Created by 46769 on 2020-12-20.
//

#pragma once

#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

#include <core/text_data.hpp>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "texture.hpp"
#include "vertex_buffer.hpp"

namespace ui {
    namespace core {
        class ScreenPos;
    }
    class View;
}

struct SyntaxColor {
    GLfloat r{0.0f}, g{0.0f}, b{0.0f};
};

template<typename T, typename U>
constexpr auto mp(T t, U u) {
    return std::pair<T, U>{t, u};
}

constexpr glm::vec3 vred{1.0f, 0.0f, 0.0f};
constexpr glm::vec3 vgreen{0.f, 1.0f, 0.0f};
constexpr glm::vec3 vblue{
        0.0f,
        0.0f,
        1.0f,
};
constexpr glm::vec3 vsc1{0.820f, 0.500f, 0.000f};
constexpr glm::vec3 vsc2{1.000f, 0.300f, 0.600f};
constexpr glm::vec3 vsc3{0.300f, 0.231f, 0.800f};
constexpr glm::vec3 vsc4{0.031f, 0.921f, 0.140f};
constexpr glm::vec3 vsc5{0.712f, 1.000f, 1.000f};

constexpr glm::vec3 c_keyword{0.820f, 0.500f, 0.000f};

constexpr auto YELLOW = glm::vec3{0.893f, 1.0f, 0.0f};
constexpr auto RED = glm::vec3{1.0f, 0.0f, 0.0f};
constexpr auto GREEN = glm::vec3{0.0f, .70f, 0.0f};
constexpr auto DARKER_GREEN = glm::vec3{0.0f, 0.73f, 0.0f};
constexpr auto BLUEISH = glm::vec3{0.0f, 0.034f, .90f};
constexpr auto BLUE = glm::vec3{0.0f, 0.0f, .790f};
constexpr auto WHITE = glm::vec3{1.0f, 1.0f, 1.0f};
constexpr auto GRAY = glm::vec3{0.5f, 0.5f, 0.5f};
constexpr auto LIGHT_GRAY = glm::vec3{0.65f, 0.65f, 0.65f};

using HighLight = std::pair<const char *, glm::vec3>;
constexpr std::array keywords{mp("int", c_keyword),        mp("bool", c_keyword),      mp("void", c_keyword),
                              mp("long", c_keyword),       mp("double", c_keyword),    mp("float", c_keyword),
                              mp("struct", c_keyword),     mp("using", c_keyword),     mp("const", c_keyword),
                              mp("char", c_keyword),       mp("constexpr", c_keyword), mp("auto", c_keyword),
                              mp("this", c_keyword),       mp("static", c_keyword),    mp("class", c_keyword),
                              mp("public", c_keyword),     mp("private", c_keyword),   mp("protected", c_keyword),
                              mp("unsigned", c_keyword),   mp("signed", c_keyword),    mp("mutable", c_keyword),
                              mp("volatile", c_keyword),   mp("return", c_keyword),    mp("signed", c_keyword),
                              mp("nullptr", c_keyword),    mp("final", c_keyword),     mp("virtual", c_keyword),
                              mp("override", c_keyword),   mp("true", c_keyword),      mp("false", c_keyword),
                              mp("if", c_keyword),         mp("else", c_keyword),      mp("namespace", c_keyword),
                              mp("friend", c_keyword),     mp("operator", c_keyword),  mp("explicit", c_keyword),
                              mp("#define", YELLOW),       mp("#include", YELLOW),     mp("sizeof", c_keyword),
                              mp("static_cast", c_keyword)};

constexpr std::array highlight{RED, GREEN, BLUE, YELLOW, WHITE};

struct ColorFormatInfo {
    std::size_t begin;
    std::size_t end;
    glm::vec3 color;
};

struct CharacterRange {
    unsigned int from, to;
};

#define NUM_GLYPHS 128

struct glyph_info {
    int x0, y0, x1, y1;// coords of glyph in the texture atlas
    int x_off, y_off;  // left & top bearing when rendering
    int advance;       // x advance when rendering
    glm::ivec2 size, bearing;
};

static int row_advance = 0;

struct ColorizeTextRange {
    std::size_t begin, length;
    glm::vec3 color;
};

class SimpleFont {
public:
    static SyntaxColor colors[8];
    // Static member vars
    static const int CHARACTERS_TEXTURE_SIZE;//!< Size of texture atlas (in pixels) that stores characters
    // Static member functions
    [[maybe_unused]] static std::unique_ptr<SimpleFont>
    setup_font(const std::string &path, int pixel_size,
               CharacterRange charRange = CharacterRange{.from = 32, .to = 255});
    SimpleFont(int pixelSize, std::unique_ptr<Texture> &&texture, std::vector<glyph_info> &&glyphs);

    void create_vertex_data_in(VAO *vao, ui::View *view, int xpos, int ypos);

    void emplace_colorized_text_gpu_data(VAO *vao, std::string_view text, int xPos, int yPos,
                                         std::optional<std::vector<ColorizeTextRange>> colorData);

    std::unique_ptr<Texture> t{nullptr};
    [[nodiscard]] int get_row_advance() const;
    std::vector<glyph_info> glyph_cache;
    int row_height;
    int max_glyph_width;
    int max_glyph_height;

    void create_vertex_data_for(ui::View *pView, ui::core::ScreenPos startingTopLeftPos);

private:
    // glyph_info* data = info;
    int pixel_size{};
};