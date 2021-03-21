//
// Created by 46769 on 2020-12-22.
//

#include "font_library.hpp"
#include <core/core.hpp>
#include <ranges>
#include <algorithm>


FontLibrary &FontLibrary::get_instance() {
    static FontLibrary fl;
    return fl;
}
void FontLibrary::load_font(const FontConfig &config, bool setAsDefault) {
    const auto &[name, path, pixel_size, char_range] = config;
    if (cached_fonts.contains(name)) {
        for (auto &f : cached_fonts.at(name)) {
            if (f->get_pixel_size() == pixel_size) {
                util::println("Font by name {} with pixel size {} is already loaded", name, pixel_size);
                return;
            }
        }
        auto font = SimpleFont::setup_font(path, pixel_size, char_range);
        cached_fonts.at(name).add_font(std::move(font));
    } else {
        auto font = SimpleFont::setup_font(path, pixel_size, char_range);
        cached_fonts.emplace(name, path);
        cached_fonts.at(name).add_font(std::move(font));

        if (setAsDefault) {
            set_as_default(config.name, config.pixel_size);
            util::println("Default font set to {} {}px", default_font_key, default_font_size);
        }
        util::println("Font {} stored. {} font groups has been loaded. Default font set to {} with pixel size {}", name, cached_fonts.size(),
                      default_font_key, default_font_size);
    }
}

SimpleFont *FontLibrary::get_default_font(std::optional<int> withSize) {
    auto &fl = FontLibrary::get_instance();
    // if (fl.default_font_key.empty()) PANIC("No default font set");
    if (withSize) {
        return FontLibrary::get_instance().get_font(fl.default_font_key, *withSize);
    } else {
        return FontLibrary::get_instance().get_font(fl.default_font_key, fl.default_font_size);
    }
}

SimpleFont *FontLibrary::get_font(const std::string &key, int size) {
    auto it = cached_fonts.find(key);
    if (it != std::end(cached_fonts)) {

        auto item = std::ranges::find_if(it->second, [&](auto &fa) { return size == fa->get_pixel_size(); });
        if (item == std::end(it->second)) { return it->second.begin()->get(); }
        return item->get();
    } else {
        PANIC("No font with key {} was found. Forced crash.", key);
    }
}
const std::string &FontLibrary::get_default_font_name() { return FontLibrary::get_instance().default_font_key; }

bool FontLibrary::has_font_loaded(const std::string &key) const { return cached_fonts.contains(key); }

bool FontLibrary::font_with_size_loaded(const std::string &key, int size) const {
    if (has_font_loaded(key)) {
        return std::ranges::any_of(cached_fonts.at(key).fonts, [&](auto &f) { return f->get_pixel_size() == size; });
    } else {
        return false;
    }
}
std::optional<const LoadedFont *> FontLibrary::get_font_group(const std::string &key) const {
    if (has_font_loaded(key)) {
        return &cached_fonts.at(key);
    } else {
        return {};
    }
}

void FontLibrary::print_loaded_fonts() const {
    util::println("Loaded fonts:");
    for (auto &[name, group] : cached_fonts) {
        fmt::print("Font name: {}\nSizes: ", name);
        for (auto &f : group) { fmt::print("{} ", f->get_pixel_size()); }
        util::println("\n---");
    }
    util::println("Default font {} - Size: {}", default_font_key, default_font_size);
}
bool FontLibrary::set_as_default(const std::string &key, int pixelSize) {
    if (has_font_loaded(key)) {
        auto& fonts = cached_fonts.at(key).fonts;
        if (std::ranges::any_of(fonts, [&](auto &f) { return f->get_pixel_size() == pixelSize; })) {
            default_font_key = key;
            default_font_size = pixelSize;
        } else {
            if(std::ranges::any_of(fonts, [&](auto &f) { return f->get_pixel_size() == default_font_size; })) {
                default_font_key = key;
            } else {
                default_font_key = key;
                default_font_size = (*fonts.begin())->get_pixel_size();
            }
        }
        return true;
    } else {
        return false;
    }
}
