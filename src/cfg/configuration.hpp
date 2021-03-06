//
// Created by 46769 on 2021-01-17.
//

#pragma once
#include <algorithm>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "types/cursor_options.hpp"
#include <core/math/vector.hpp>
#include <ui/managers/font_library.hpp>


namespace fs = std::filesystem;

class FontConfig;

using StrView = std::string_view;
using Table = std::map<StrView, StrView>;
using ParsedView = std::map<std::string, Table>;
ParsedView parse_config_data(const std::string& data);
std::vector<FontConfig> parse_font_configs(const fs::path& cfg);
struct ConfigFileData {
    std::string raw_file_data;
    ParsedView parsed_data;
    fs::path file_path;

    [[nodiscard]] std::optional<std::string> get_str_value(const std::string& table, std::string_view key) const;
    [[nodiscard]] bool has_table(const std::string& table) const;
    [[nodiscard]] static ConfigFileData load_cfg_data(const fs::path& file_path = "assets/cxconfig.cxe");
};

ConfigFileData cfg_parse(std::string&& data);
std::string to_string(StrView str);

/// The actual string data, the configuration file holds, and in parsed table format, though not parsed to actual values,
/// the application has any use for just yet



enum class LexerState {
    Table,
    Key,
    Value,
    KeyValueParsed,
    Start,
};

template<typename It>
concept CharIterator = requires(It it) {
    it++;
    *it;
    StrView{it, it};
};

template<CharIterator It>
std::string parse_table_name(It begin, It end) {
    StrView sview{begin, end};
    sview.remove_prefix(1);
    return to_string(sview);
}

template<CharIterator It>
StrView parse_key(It begin, It end) {
    std::string_view sview{begin, end};
    auto iterator = std::find_if(sview.rbegin(), sview.rend(), [](auto e) { return std::isalpha(e); });
    auto dist = std::distance(sview.rbegin(), iterator);

    auto key_begins_iterator = std::find_if(sview.begin(), sview.end(), [](auto e) { return std::isalpha(e); });
    auto begin_dist = std::distance(sview.begin(), key_begins_iterator);
    sview.remove_suffix(dist);
    sview.remove_prefix(begin_dist);
    return sview;
}

template<CharIterator It>
StrView parse_value(It begin, It end) {
    std::string_view sview{begin, end};
    auto rev_iterator = std::find_if(sview.rbegin(), sview.rend(), [](auto e) { return std::isalpha(e) || e == '"'; });
    auto end_dist = std::distance(sview.rbegin(), rev_iterator);
    sview.remove_suffix(end_dist);
    auto iterator = std::find_if(sview.begin(), sview.end(), [](auto e) { return e == '"'; });
    auto dist = std::distance(sview.begin(), iterator);
    sview.remove_prefix(dist);
    return sview;
}

struct FontConfiguration {
    std::string name;
    std::string asset_path;
    std::vector<int> sizes;
};

struct Configuration {
    // We don't pollute the namespace with these small-context types
    struct Views {
        RGBColor bg_color{0.05f, 0.052f, 0.0742123f};
        RGBColor fg_color{1.0f, 1.0f, 1.0f};
        int font_pixel_size = 24;
        bool horizontal_layout_only = true;
    } views;

    struct Window {
        int width{1024};
        int height{768};
        int monitors{1};
    } window;

    struct Cursor {
        RGBAColor color;
        // Variant style options, defined in ./types/<header>.hpp
        CaretStyleOption cursor_style;
    } cursor;

    fs::path file_path;
    static Configuration from_parsed_map(const ConfigFileData& parsedView);
    static Configuration make_default();

};

std::string serialize(const Configuration& cfg);