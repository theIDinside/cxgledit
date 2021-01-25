//
// Created by 46769 on 2020-12-22.
//

#pragma once

#include <map>
#include <string>
#include <ui/render/font.hpp>
#include <set>
#include <utility>

using FontRef = std::unique_ptr<SimpleFont>;

template <typename FontTypeSmartPtr> concept IsFontRef = requires(FontTypeSmartPtr a) {
    a->get_row_advance();
};

struct FontConfig {
    std::string name;
    std::string path;
    int pixel_size;
    CharacterRange char_range{.from = 0, .to = SWEDISH_LAST_ALPHA_CHAR_UNICODE};
};


struct LoadedFont {
    using FontSet = std::set<FontRef>;
    fs::path asset_path;
    std::set<FontRef> fonts{};
    explicit LoadedFont(fs::path path) : asset_path(std::move(path)) {}

    void add_font(FontRef&& font) {
        fonts.emplace(std::move(font));
    }

    auto begin() {
        return fonts.begin();
    }

    auto end() {
        return fonts.end();
    }

    auto begin() const {
        return fonts.begin();
    }

    auto end() const {
        return fonts.end();
    }

    auto cbegin() const {
        return fonts.begin();
    }

    auto cend() const {
        return fonts.end();
    }
};

class FontLibrary {
public:
    FontLibrary(const FontLibrary &) = delete;
    static FontLibrary &get_instance();
    void load_font(const FontConfig &config, bool setAsDefault = true);
    SimpleFont *get_font(const std::string &key, int size);
    bool has_font_loaded(const std::string &key) const;
    bool font_with_size_loaded(const std::string &key, int size) const;
    std::optional<const LoadedFont*> get_font_group(const std::string& key) const;
    static SimpleFont *get_default_font(std::optional<int> withSize = {});
    static const std::string& get_default_font_name();
    bool set_as_default(const std::string& key, int pixelSize);
    // DEBUG
    void print_loaded_fonts() const;
private:
    FontLibrary() {}
    std::string default_font_key;
    int default_font_size = 0;
    std::map<std::string, LoadedFont> cached_fonts;
};
