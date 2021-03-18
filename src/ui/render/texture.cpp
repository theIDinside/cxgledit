//
// Created by 46769 on 2020-12-20.
//

#include "texture.hpp"

std::unique_ptr<GlyphTexture> GlyphTexture::setup_texture_info() {
    GLuint id;
    glGenTextures(1, &id);
    auto tex = std::make_unique<GlyphTexture>(GlyphTexture{.id = id, .id_generated = true, .finalized = false});
    return tex;
}

std::unique_ptr<GlyphTexture> GlyphTexture::make_from_data(const unsigned char *data, int width, int height, int bytesPerPixel) {
    auto t = GlyphTexture::setup_texture_info();
    t->bind();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);// disable byte-alignment restriction
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    t->width = width;
    t->height = height;
    t->bpp = bytesPerPixel;
    t->finalized = true;
    t->id_generated = true;
    glGenerateMipmap(GL_TEXTURE_2D);
    return t;
}

void GlyphTexture::bind() const { glBindTexture(GL_TEXTURE_2D, id); }
