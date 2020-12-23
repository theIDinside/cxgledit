#include "app.hpp"
#include <GLFW/glfw3.h>
#include <core/core.hpp>
#include <fmt/core.h>
#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <mutex>
#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>
#include <utils/utils.hpp>

static int wh = 768, ww = 1024;
static GLFWwindow* win = nullptr;
static glm::mat4 projection;

void render(const std::string& text, SimpleFont* font, Shader& fontShader, GLuint VAO, GLuint VBO, int atx, int aty) {
    auto vertexData = font->make_gpu_data(text, atx, wh - aty);
    glm::vec3 col{1.0, 0.5, 0.0};
    fontShader.use();
    font->t->bind();
    projection = glm::ortho(0.0f, static_cast<float>(ww), 0.0f, static_cast<float>(wh));

    glBindVertexArray(VAO);
    // glUniform3f(glGetUniformLocation(shader.ID, "textColor"), col.x, col.y, col.z);
    fontShader.setVec3("textColor", col);
    fontShader.setMat4("projection", projection);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, vertexData.bytes_size(), vertexData.data, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.bytes_size(), vertexData.data);
    glDrawArrays(GL_TRIANGLES, 0, vertexData.vertex_count);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    vertexData.destroy();
}

void render(const std::string& text, SimpleFont* font, Shader& fontShader, VAO* vao, int atx, int aty) {
    font->emplace_gpu_data(vao, text, atx, wh - aty);
    glm::vec3 col{1.0, 0.5, 0.0};
    fontShader.use();
    font->t->bind();
    projection = glm::ortho(0.0f, static_cast<float>(ww), 0.0f, static_cast<float>(wh));
    vao->bind_all();
    // glUniform3f(glGetUniformLocation(shader.ID, "textColor"), col.x, col.y, col.z);
    fontShader.setVec3("textColor", col);
    fontShader.setMat4("projection", projection);
    vao->flush_and_draw();
}

auto init_glfw(int win_width, int win_height) {

    static std::once_flag init_flag;
    static GLFWwindow* window;

    auto framebuffer_size_callback = [](GLFWwindow *window, int width, int height) {
      fmt::print("New width: {}\t New height: {}", width, height);
      wh = height;
      ww = width;
      projection = glm::ortho(0.0f, float(ww), 0.0f, float(wh));
      glViewport(0, 0, width, height);
    };

    std::call_once(init_flag, [&]() {
      glfwInit();
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      window = glfwCreateWindow(win_width, win_height, "Drawing Text Better", nullptr, nullptr);
      if (window == nullptr) {
          glfwTerminate();
          PANIC("Failed to create GLFW window\n");
      }
      glfwMakeContextCurrent(window);
      glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
      if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) { PANIC("Failed to initialize GLAD\n"); }
      glEnable(GL_CULL_FACE);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    });

    if (window == nullptr) {
        glfwTerminate();
        PANIC("Failed to create GLFW window\n");
    }
    return window;
}


// This is _simply_ to test - I am obviously not going to have std::string as text buffer.
static std::string text_buffer{""};


int main() {
    auto app = App::create(ww, wh);
    // app->load_file("main_2.cpp");
    app->run_loop();
    return 0;
}
