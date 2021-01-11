//
// Created by 46769 on 2021-01-11.
//

#pragma once
#include <GLFW/glfw3.h>
#include <glad/glad.h>

void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
                            const char *message, const void *userParam);