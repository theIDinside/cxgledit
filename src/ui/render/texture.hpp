//
// Created by 46769 on 2020-12-20.
//

#pragma once
#include <glad/glad.h>
#include <optional>
#include <memory>

using byte = unsigned char;

struct Texture {
    static std::unique_ptr<Texture> setup_texture_info();
    static std::unique_ptr<Texture> make_from_data(const byte* data, int width, int height, int bytesPerPixel);
    std::optional<Texture> load_from_file(const char* path);

    void bind(int textureUnit=0) const;

    const GLuint id;
    int width{0};
    int height{0};
    int bpp{0};
    bool id_generated{false};
    bool finalized{false};
};