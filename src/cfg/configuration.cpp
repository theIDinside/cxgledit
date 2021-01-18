//
// Created by 46769 on 2021-01-17.
//
#include "configuration.hpp"
#include <fstream>
#include <sstream>

// TODO: this looks horrible. But it works for now
ConfigFileData cfg_parse(std::string &&file_data) {
    ConfigFileData cfgFile{.raw_file_data = std::move(file_data), .parsed_data = {}};
    // auto& data = *ptrData;
    auto &[data, parsed, _] = cfgFile;
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
    return cfgFile;
}
std::string to_string(StrView str) { return std::string{str}; }

std::string serialize(const Configuration &cfg) {
    std::stringstream ss{};
    ss << "[views]\n";
    ss << "background_color = " << cfg.views.bg_color << ";\n";
    ss << "foreground_color = " << cfg.views.fg_color << ";\n";
    ss << "font_pixel_size = " << "\"" << cfg.views.font_pixel_size << "\";\n";
    ss << "horizontal_layout_only = " << "\"" << (cfg.views.horizontal_layout_only ? "on" : "off") << "\";\n\n";

    ss << "[window]\n";
    ss << "width = " << "\"" << cfg.window.width << "\";\n";
    ss << "height = " << "\"" << cfg.window.height << "\";\n";
    ss << "monitors = " << "\"" << cfg.window.monitors << "\";\n";


    return ss.str();
}

Color parse_color(const std::string &str_item) {
    std::stringstream ss{str_item};
    Color c;
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
        auto strWidth = configFileData.get_str_value("window", "width").value_or("1920");
        auto strHeight = configFileData.get_str_value("window", "height").value_or("1024");
        auto strMon = configFileData.get_str_value("window", "monitors").value_or("1");
        cfg.window.width = std::stoi(strWidth);
        cfg.window.height = std::stoi(strHeight);
        cfg.window.monitors = std::stoi(strMon);
    }

    if (configFileData.has_table("views")) {
        auto strBackgroundColor = configFileData.get_str_value("views", "background_color").value_or("0.05 0.052 0.0742123");
        auto strForegroundColor = configFileData.get_str_value("views", "foreground_color").value_or("1 1 1");
        auto strFontPixelSize = configFileData.get_str_value("views", "font_pixel_size").value_or("24");
        auto strHorizontalLayoutOnly = configFileData.get_str_value("views", "horizontal_layout_only").value_or("on");

        cfg.views.bg_color = parse_color(strBackgroundColor);
        cfg.views.fg_color = parse_color(strForegroundColor);
        cfg.views.font_pixel_size = std::stoi(strFontPixelSize);
        cfg.views.horizontal_layout_only = (strHorizontalLayoutOnly == "on");
    }
    cfg.file_path = configFileData.file_path;
    return cfg;
}
Configuration Configuration::make_default() {
    Configuration c;
    c.file_path = "assets/config.cxe";
    return c;
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
