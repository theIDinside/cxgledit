//
// Created by 46769 on 2020-12-22.
//

#pragma once

#include <map>
#include <string>
#include <ui/render/font.hpp>
#include <set>

struct FontConfig {
    std::string name;
    std::string path;
    int pixel_size;
    CharacterRange char_range;
};

class FontLibrary {
public:
    FontLibrary(const FontLibrary &) = delete;
    static FontLibrary &get_instance();
    void load_font(const FontConfig &config, bool setAsDefault = true);
    SimpleFont *get_font(const std::string &key);
    std::optional<SimpleFont*> get_font_with_size(int size);
    static SimpleFont *get_default_font();
private:
    FontLibrary() {}
    std::string default_font_key;
    std::map<std::string, std::unique_ptr<SimpleFont>> cached_fonts;
    std::map<int, SimpleFont*> font_sizes;
};
