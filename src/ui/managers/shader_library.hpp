//
// Created by 46769 on 2020-12-22.
//

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <ui/render/shader.hpp>

namespace fs = std::filesystem;
struct ShaderConfig {
    std::string name;
    fs::path vs_path;
    fs::path fs_path;
};

class ShaderLibrary {
public:
    static ShaderLibrary &get_instance();
    void load_shader(ShaderConfig cfg);
    [[nodiscard]] Shader *get_shader(const std::string &key);
    [[nodiscard]] static Shader *get_text_shader();

private:
    ShaderLibrary() = default;
    std::map<std::string, Shader> shaders;
};