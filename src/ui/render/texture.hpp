//
// Created by 46769 on 2020-12-20.
//

#pragma once
#include <glad/glad.h>
#include <memory>
#include <optional>

using byte = unsigned char;

struct GlyphTexture {
    static std::unique_ptr<GlyphTexture> setup_texture_info();
    static std::unique_ptr<GlyphTexture> make_from_data(const byte *data, int width, int height, int bytesPerPixel);

    void bind() const;

    const GLuint id;
    int width{0};
    int height{0};
    int bpp{0};
    bool id_generated{false};
    bool finalized{false};
};