//
// Created by 46769 on 2020-12-22.
//

#pragma once

#include <string>
#include <map>
#include <ui/render/font.hpp>

struct FontConfig {
    std::string name;
    std::string path;
    int pixel_size;
    CharacterRange char_range;
};

class FontLibrary {
public:
    FontLibrary(const FontLibrary&) = delete;
    static FontLibrary& get_instance();
    void load_font(const FontConfig& config, bool setAsDefault = true);
    SimpleFont* get_font(const std::string& key);

    static SimpleFont* get_default_font();

private:
    FontLibrary() {}
    std::string default_font_key;
    std::map<std::string, std::unique_ptr<SimpleFont>> cached_fonts;
};
