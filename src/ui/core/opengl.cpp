//
// Created by 46769 on 2021-01-11.
//

#include "opengl.hpp"
#include <fmt/core.h>

void glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message,
                   const void *userParam) {
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes

    fmt::print("---------------\n");
    fmt::print("Debug message ({}): {}", id, message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             fmt::print("Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   fmt::print("Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: fmt::print("Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     fmt::print("Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     fmt::print("Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER:           fmt::print("Source: Other"); break;
    }
    fmt::print("\n");

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               fmt::print("Type: Error");
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: fmt::print("Type: Deprecated Behaviour");
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  fmt::print("Type: Undefined Behaviour");
        case GL_DEBUG_TYPE_PORTABILITY:         fmt::print("Type: Portability");
        case GL_DEBUG_TYPE_PERFORMANCE:         fmt::print("Type: Performance");
        case GL_DEBUG_TYPE_MARKER:              fmt::print("Type: Marker");
        case GL_DEBUG_TYPE_PUSH_GROUP:          fmt::print("Type: Push Group");
        case GL_DEBUG_TYPE_POP_GROUP:           fmt::print("Type: Pop Group");
        case GL_DEBUG_TYPE_OTHER:               fmt::print("Type: Other");
    }
    fmt::print("\n");

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         fmt::print("Severity: high");
        case GL_DEBUG_SEVERITY_MEDIUM:       fmt::print("Severity: medium");
        case GL_DEBUG_SEVERITY_LOW:          fmt::print("Severity: low");
        case GL_DEBUG_SEVERITY_NOTIFICATION: fmt::print("Severity: notification");
    }
    fmt::print("\n");
    fmt::print("\n");
}
