//
// Created by 46769 on 2020-12-22.
//

#include "font_library.hpp"
#include <core/core.hpp>
FontLibrary &FontLibrary::get_instance() {
    static FontLibrary fl;
    return fl;
}
void FontLibrary::load_font(const FontConfig &config, bool setAsDefault) {
    const auto &[name, path, pixel_size, char_range] = config;
    auto font = SimpleFont::setup_font(path, pixel_size, char_range);
    cached_fonts.emplace(name, std::move(font));
    if (setAsDefault) { default_font_key = name; }
    util::println("Font {} stored. {} fonts has been loaded. Default font set to {}", name, cached_fonts.size(),
                  default_font_key);
    assert(cached_fonts.count(default_font_key) != 0 && "Storing the font somehow errored");
}

SimpleFont *FontLibrary::get_default_font() {
    return FontLibrary::get_instance().get_font(FontLibrary::get_instance().default_font_key);
}

SimpleFont *FontLibrary::get_font(const std::string &key) {
    auto it = cached_fonts.find(key);
    if (it != std::end(cached_fonts)) {
        return it->second.get();
    } else {
        PANIC("No font with key {} was found. Forced crash.", key);
    }
}
