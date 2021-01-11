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
    font_sizes.emplace(pixel_size, cached_fonts[name].get());
    if (setAsDefault) { default_font_key = name; }
    util::println("Font {} stored. {} fonts has been loaded. Default font set to {}", name, cached_fonts.size(),
                  default_font_key);
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
std::optional<SimpleFont*> FontLibrary::get_font_with_size(int size) {
    if(auto item = font_sizes.find(size); item != std::end(font_sizes)) {
        return item->second;
    }
    return {};
}
