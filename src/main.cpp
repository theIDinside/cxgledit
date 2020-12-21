#include <GLFW/glfw3.h>
#include <fmt/core.h>
#include <glad/glad.h>
#include <mutex>
#include <ui/render/font.hpp>
#include <ui/render/shader.hpp>
#include <utils/utils.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>

static int wh = 768, ww = 1024;
static GLFWwindow* win = nullptr;
static glm::mat4 projection;

void render(const std::string& text, SimpleFont* font, Shader& fontShader, GLuint VAO, GLuint VBO, int atx, int aty) {
    auto vertexData = font->make_gpu_data(text, atx, wh - aty);
    glm::vec3 col{1.0, 0.5, 0.0};
    fontShader.use();
    font->t->bind();
    glBindVertexArray(VAO);
    // glUniform3f(glGetUniformLocation(shader.ID, "textColor"), col.x, col.y, col.z);
    fontShader.setVec3("textColor", col);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, vertexData.bytes_size(), vertexData.data, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexData.bytes_size(), vertexData.data);
    glDrawArrays(GL_TRIANGLES, 0, vertexData.vertex_count);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    vertexData.destroy();
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

    auto SWEDISH_LAST_ALPHA_CHAR_UNICODE = 0x00f6u;

    text_buffer.reserve(100000);
    fmt::print("Creating window");
    win = init_glfw(ww, wh);
    projection = glm::ortho(0.0f, float(ww), 0.0f, float(wh));
    auto font = SimpleFont::setup_font("assets/fonts/Ubuntu-R.ttf", 24, CharacterRange{.from = 0, .to = SWEDISH_LAST_ALPHA_CHAR_UNICODE });
    auto text_shader = Shader::load_shader("assets/shaders/textshader.vs", "assets/shaders/textshader.fs");
    text_shader.use();

    text_shader.setMat4("projection", projection);
    std::string tmp_buffer;

    auto file_path = "main_2.cpp";
    std::ifstream f{file_path};
    text_buffer.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    fmt::print("Loaded file with {} bytes\n", text_buffer.size());

    auto VAO = 0u;
    auto VBO = 0u;
    constexpr auto CHARACTERS_DISPLAY_DEFAULT_LENGTH = 1000000; // default setup buffer for 30k characters
    constexpr auto TEXT_DISPLAY_BUFFER_SIZE = sizeof(float) * 6 * 4 * CHARACTERS_DISPLAY_DEFAULT_LENGTH;
    fmt::print("Creating a buffer of {} bytes\n", TEXT_DISPLAY_BUFFER_SIZE);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER, TEXT_DISPLAY_BUFFER_SIZE, nullptr, GL_DYNAMIC_DRAW);
    // glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    static constexpr auto PRESS_MASK = GLFW_PRESS | GLFW_REPEAT;
    static constexpr auto pressed_or_repeated = [](auto action) -> bool { return action & PRESS_MASK; };

    glfwSetCharCallback(win, [](auto win, auto codepoint) {
        text_buffer.push_back(codepoint);
    });

    glfwSetKeyCallback(win, [](auto window, int key, int scancode, int action, int mods) {
      if (pressed_or_repeated(action)) {
        switch(key) {
            case GLFW_KEY_ENTER:
                text_buffer.push_back('\n');
                break;
            case GLFW_KEY_TAB:
                text_buffer.insert(text_buffer.size(), "    ");
                break;
        }
      }
    });

    while(!glfwWindowShouldClose(win)) {
        auto t1 = std::chrono::high_resolution_clock::now();
        render(text_buffer, font.get(), text_shader, VAO, VBO, 10, 100);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        fmt::print("Frame time: {}us\n",duration);
        fflush(stdout);
        glfwSwapBuffers(win);
        glfwWaitEventsTimeout(1);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        text_shader.setMat4("projection", projection);
    }


    return 0;
}
