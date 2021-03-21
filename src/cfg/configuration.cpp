//
// Created by 46769 on 2021-01-17.
//
#include "configuration.hpp"

#include <fstream>
#include <sstream>

ParsedView parse_config_data(const std::string &data) {
    ParsedView parsed{};
    StrView key, val;
    std::string current_table{};
    LexerState state = LexerState::Table;
    auto e = data.end();
    for (auto i = data.begin(); i < e; i++) {
        if (*i == '[') {
            state = LexerState::Table;
        } else if (std::isalpha(*i)) {
            state = LexerState::Key;
        }

        if (state == LexerState::Table) {
            auto lookahead = i;
            bool ok = false;
            while (lookahead != e || *lookahead != '\n') {
                if (*lookahead == ']') {
                    ok = true;
                    break;
                }
                lookahead++;
            }
            if (not ok) throw std::runtime_error{"parsing of table name failed"};
            current_table = parse_table_name(i, lookahead);
            parsed.emplace(current_table, Table{});
            state = LexerState::Key;
            i = lookahead;
        } else if (state == LexerState::Key) {
            auto lookahead = i;
            while (lookahead != e && *lookahead != '=') { lookahead++; }
            if (lookahead == e) throw std::runtime_error{"parsing of key failed"};
            auto k = parse_key(i, lookahead);
            state = LexerState::Value;
            i = lookahead;
            key = k;
        } else if (state == LexerState::Value) {
            auto lookahead = i;
            bool expr_ok = false;
            while (lookahead != e) {
                if (*lookahead == ';') {
                    expr_ok = true;
                    break;
                }
                lookahead++;
            }
            if (not expr_ok) throw std::runtime_error{"key value expressions must end with a ';'"};
            auto v = parse_value(i, lookahead);
            state = LexerState::KeyValueParsed;
            i = lookahead;
            val = v;
        }
        if (state == LexerState::KeyValueParsed) {
            auto insert_key = key;
            auto insert_val = val;
            parsed[current_table][insert_key] = insert_val;
        }
    }
    return parsed;
}

// TODO: this looks horrible. But it works for now
ConfigFileData cfg_parse(std::string &&file_data) {
    ConfigFileData cfgFile{.raw_file_data = std::move(file_data), .parsed_data = {}};
    // auto& data = *ptrData;
    auto &[data, parsed, _] = cfgFile;
    parsed = parse_config_data(data);
    return cfgFile;
}
std::string to_string(StrView str) { return std::string{str}; }

std::string serialize(const Configuration &cfg) {
    std::stringstream ss{};
    ss << "[views]\n";
    ss << "background_color = " << cfg.views.bg_color << ";\n";
    ss << "active_background_color = " << cfg.views.bg_color << ";\n";
    ss << "foreground_color = " << cfg.views.fg_color << ";\n";
    ss << "font_pixel_size = "
       << "\"" << cfg.views.font_pixel_size << "\";\n";
    ss << "horizontal_layout_only = "
       << "\"" << (cfg.views.horizontal_layout_only ? "on" : "off") << "\";\n\n";

    ss << "[window]\n";
    ss << "width = "
       << "\"" << cfg.window.width << "\";\n";
    ss << "height = "
       << "\"" << cfg.window.height << "\";\n";
    ss << "monitors = "
       << "\"" << cfg.window.monitors << "\";\n";

    int default_caret_line_width = 6;
    auto caret_style_opt = serialize_caret_option(cfg.cursor.cursor_style, default_caret_line_width);
    ss << "[cursor]\n";
    ss << "color = "
            << "\"" << cfg.cursor.color << "\";\n";
    ss << "caret_style = "
       << "\"" << caret_style_opt << "\";\n";
    ss << "caret_line_width = "
       << "\"" << default_caret_line_width << "\";\n";

    return ss.str();
}
RGBColor parse_rgb_color(const std::string &str_item) {
    std::stringstream ss{str_item};
    RGBColor c;
    int idx = 0;
    std::string t;
    while (std::getline(ss, t, ' ')) {
        auto res = std::stod(t);
        c[idx] = res;
        idx++;
    }
    return c;
}
RGBAColor parse_rgba_color(const std::string &str_item) {
    std::stringstream ss{str_item};
    RGBAColor c;
    int idx = 0;
    std::string t;
    while (std::getline(ss, t, ' ')) {
        auto res = std::stod(t);
        c[idx] = res;
        idx++;
    }
    return c;
}

Configuration Configuration::from_parsed_map(const ConfigFileData &configFileData) {
    Configuration cfg;// defaulted values
    if (configFileData.has_table("window")) {
        const auto strWidth = configFileData.get_str_value("window", "width").value_or("1920");
        const auto strHeight = configFileData.get_str_value("window", "height").value_or("1024");
        const auto strMon = configFileData.get_str_value("window", "monitors").value_or("1");
        cfg.window.width = std::stoi(strWidth);
        cfg.window.height = std::stoi(strHeight);
        cfg.window.monitors = std::stoi(strMon);
    }

    if (configFileData.has_table("views")) {
        const auto strBackgroundColor = configFileData.get_str_value("views", "background_color").value_or("0.05 0.052 0.0742123");
        const auto strActiveBackgroundColor = configFileData.get_str_value("views", "active_background_color").value_or("0.071 0.102 0.1242123");
        const auto strForegroundColor = configFileData.get_str_value("views", "foreground_color").value_or("1 1 1");
        const auto strFontPixelSize = configFileData.get_str_value("views", "font_pixel_size").value_or("24");
        const auto strHorizontalLayoutOnly = configFileData.get_str_value("views", "horizontal_layout_only").value_or("on");

        cfg.views.active_bg = parse_rgb_color(strActiveBackgroundColor);
        cfg.views.bg_color = parse_rgb_color(strBackgroundColor);
        cfg.views.fg_color = parse_rgb_color(strForegroundColor);
        cfg.views.font_pixel_size = std::stoi(strFontPixelSize);
        cfg.views.horizontal_layout_only = (strHorizontalLayoutOnly == "on");

    }
    if (configFileData.has_table("cursor")) {
        const auto caret_color = configFileData.get_str_value("cursor", "color").value_or("0.3 0 0.5 0.5");
        const auto caret_style_str = configFileData.get_str_value("cursor", "caret_style").value_or("line");
        const auto caret_line_width_str = configFileData.get_str_value("cursor", "caret_line_width").value_or("6");

        if (caret_style_str == "block") {
            cfg.cursor.cursor_style = CaretStyleBlock{};
        } else {
            auto caret_line_width = 6;// default value if user wrote a NAN in the cfg
            try {
                caret_line_width = std::stoi(caret_line_width_str);
            } catch (std::exception &e) {// user wrote a non-number thus parsing it crashed
                // TODO: print errors to window/message bar for user, with (possibly) some diagnostics
                cfg.cursor.cursor_style = CaretStyleLine{caret_line_width};
            }
            cfg.cursor.cursor_style = CaretStyleLine{caret_line_width};
        }
        cfg.cursor.color = parse_rgba_color(caret_color);
    }
    cfg.file_path = configFileData.file_path;
    return cfg;
}
Configuration Configuration::make_default() {
    Configuration c;
    c.file_path = "assets/config.cxe";
    return c;
}

std::vector<FontConfig> parse_font_configs(const fs::path &cfg) {
    std::vector<FontConfig> result;
    std::ifstream f{cfg};
    std::string buf;
    buf.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    auto parsed = parse_config_data(buf);
    for(auto& [font_name, table] : parsed) {
        if(!table.contains("asset")) {
            util::println("Failed to parse config data for {} due to asset path not being defined. usage: \n\tasset = \"... path ...\";");
        } else {
            auto asset_path = table.at("asset");
            asset_path.remove_suffix(1);
            asset_path.remove_prefix(1);
            if(!table.contains("sizes")) {
                util::println("No sizes set for font, using 1 default (18). Setting example: \n\tsizes = \"[12 13 18]\"");
                result.emplace_back(FontConfig{font_name, std::string{asset_path}, 18});
            } else {
                auto sizes_data = table.at("sizes");
                sizes_data.remove_prefix(1);
                sizes_data.remove_suffix(1);
                std::string d{sizes_data};
                std::stringstream ss{d};
                std::vector<int> sizes;
                std::string t;
                while (std::getline(ss, t, ' ')) {
                    auto res = std::stoi(t);
                    sizes.push_back(res);
                }
                for(auto s : sizes) {
                    result.emplace_back(FontConfig{font_name, std::string{asset_path}, s});
                }
            }

        }
    }
    util::println("Parsed {} font configurations", result.size());
    return result;
}

std::optional<std::string> ConfigFileData::get_str_value(const std::string &table, std::string_view key) const {
    if (parsed_data.contains(table)) {
        const auto &tableRef = parsed_data.at(table);
        if (tableRef.contains(key)) {
            std::string res{tableRef.at(key)};
            // remove the "" around the data
            res.erase(std::remove(res.begin(), res.end(), '"'), res.end());
            return {res};
        } else {
            return {};
        }
    } else {
        util::println("did not find value for key {}", key);
        return {};
    }
}
bool ConfigFileData::has_table(const std::string &table) const { return parsed_data.contains(table); }

ConfigFileData ConfigFileData::load_cfg_data(const fs::path &file_path) {
    std::ifstream f{file_path};
    std::string buf;
    buf.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    auto res = cfg_parse(std::move(buf));
    res.file_path = file_path;
    return res;
}
