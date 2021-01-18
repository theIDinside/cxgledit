#pragma once

#include <core/math/matrix.hpp>
#include <core/math/vector.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>


namespace fs = std::filesystem;

enum Uniforms { Projection };

class Shader {
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader() noexcept = default;
    explicit Shader(unsigned id) noexcept;
    Shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr) noexcept;
    Shader(const Shader &copy) noexcept = default;
    // activate the shader
    // ------------------------------------------------------------------------
    void use() const;
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const;
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const;
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const;
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const Vec2f &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const Vec3f &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const Vec4f &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w);
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const Matrix& mat) const;
    void set_projection(const Matrix& mat);
    void set_fillcolor(Vec4f color);


    void setup();
    void setup_fillcolor_ids();

    static Shader load_shader(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr);
    static Shader load_shader(fs::path vertexPath, fs::path fragmentPath, fs::path geometryPath = "");

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    static void checkCompileErrors(unsigned int shader, std::string type);
    int projection;
    int fillcolor;
};