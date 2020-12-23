//
// Created by 46769 on 2020-12-22.
//

#include "shader_library.hpp"
#include <core/core.hpp>
#include <utils/utils.hpp>

void ShaderLibrary::load_shader(ShaderConfig cfg) {
    const auto [name, vertex_path, frag_path] = cfg;
    auto shader = Shader::load_shader(vertex_path, frag_path);
    shaders[name] = shader;
    shaders[name].setup();
    util::println("Stored shader {}. {} shaders has been installed.", name, shaders.size());
}
ShaderLibrary &ShaderLibrary::get_instance() {
    static ShaderLibrary sl;
    return sl;
}

Shader *ShaderLibrary::get_shader(const std::string& key) {
    if(auto res = shaders.find(key); res != std::end(shaders)) {
        return &(res->second);
    } else {
        PANIC("No shader with key {} was found. Forced crash.", key);
    }
}
Shader *ShaderLibrary::get_text_shader() {
    return ShaderLibrary::get_instance().get_shader("text");
}
