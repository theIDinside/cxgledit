#include <array>
#include <fmt/core.h>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "defs.hpp"
#include <ui/shader.hpp>
#include <util/fileutil.hpp>

constexpr std::array FontNames = {"assets/fonts/DroidSansFallbackFull.ttf", "assets/fonts/FreeMono.ttf",
                                  "assets/fonts/UbuntuMono-R.ttf"};

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void RenderText(Shader &shader, std::string text, float x, float y,
                float advance_y, float scale, glm::vec3 color);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2 Size;        // Size of glyph
    glm::ivec2 Bearing;     // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;
unsigned int VAO, VBO;
static std::string buffer{""};
static bool do_render = true;
static auto last_key_event = 0.0f;

int main(int argc, const char **argv) {
    if (argc > 1) {
        fmt::print("Opening file: {}\n", argv[1]);
        auto file_path = get_path(argv[1]);
        if (file_path) {
            std::ifstream f{*file_path};
            buffer.assign(std::istreambuf_iterator<char>(f),
                          std::istreambuf_iterator<char>());
            do_render = true;
        }
    } else {
        fmt::print("No file in args list\n");
    }
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window =
            glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        fmt::print("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fmt::print("Failed to initialize GLAD\n");
        return -1;
    }

    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // compile and setup the shader
    // ----------------------------
    Shader shader("assets/shaders/textshader.vs", "assets/shaders/textshader.fs");
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f,
                                      static_cast<float>(SCR_HEIGHT));
    shader.use();
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));

    // FreeType
    // --------
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft)) {
        fmt::print("ERROR::FREETYPE: Could not init FreeType Library\n");
        return -1;
    }

    // DroidSansFallbackFull.ttf

    // find path to font
    auto font_name = get_path(FontNames[2]);
    if (!font_name) {
        fmt::print("ERROR::FREETYPE: Failed to load font_name\n");
        return -1;
    }

    // load font as face
    FT_Face face;
    u64 max_adv_x = 0;
    u64 max_glyph_height = 0;

    if (FT_New_Face(ft, font_name->c_str(), 0, &face)) {
        fmt::print("ERROR::FREETYPE: Failed to load font\n");
        return -1;
    } else {
        // set size to load glyphs as
        FT_Set_Pixel_Sizes(face, 0, 14);

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load first 128 characters of ASCII set
        for (unsigned char c = 0; c < 128; c++) {
            // Load character glyph
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                fmt::print("ERROR::FREETYTPE: Failed to load Glyph\n");
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,
                         face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE,
                         face->glyph->bitmap.buffer);
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                    texture,
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    static_cast<unsigned int>(face->glyph->advance.x)};
            auto last_x = max_adv_x;
            auto last_y = max_glyph_height;
            max_adv_x = std::max((u64)face->glyph->advance.x, max_adv_x);
            max_glyph_height =
                    std::max((u64)face->glyph->bitmap.rows, max_glyph_height);

            if (last_x != max_adv_x) {
                fmt::print("Glyphs had different max advance x: {} | {}\n", last_x,
                           max_adv_x);
            }
            if (last_y != max_glyph_height) {
                fmt::print("Glyphs had different max advance y: {} | {}\n", last_y,
                           max_glyph_height);
            }

            Characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    fmt::print("Max advance x: {} - y: {}", max_adv_x, max_glyph_height);
    auto max_adv_y = max_glyph_height + 2;
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // configure VAO/VBO for texture quads
    // -----------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    glfwSetCharCallback(window, [](auto win, auto codepoint) {
      auto current_t = glfwGetTime();
      auto dt = current_t - last_key_event;
      last_key_event = current_t;
      fmt::print("codepoint: {}\n", codepoint);

      auto ch = (char)codepoint;
      buffer.push_back(ch);
      do_render = true;

      /*if(codepoint >= (decltype(codepoint))' ' && codepoint <=
      (decltype(codepoint))'z') { auto ch = (char)codepoint; buffer.push_back(ch);
          do_render = true;
      }*/
    });

    glfwSetKeyCallback(
            window, [](auto window, int key, int scancode, int action, int mods) {
              // fmt::print("scancode: " << scancode << " action: " << action << "
              // mods: " << mods << '\n';
              auto current_t = glfwGetTime();
              auto dt = current_t - last_key_event;
              last_key_event = current_t;

              if (key == GLFW_KEY_ESCAPE) {
                  glfwSetWindowShouldClose(window, true);
              } else if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {

                  if (mods & GLFW_MOD_CONTROL) {
                      buffer.clear();
                  }

                  if (!buffer.empty()) {
                      buffer.pop_back();
                  }
                  do_render = true;
              } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
                  buffer.push_back('\n');
              } else if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
                  buffer.insert(buffer.size(), "    ");
              }
            });

    glfwSetWindowFocusCallback(window,
                               [](auto window, auto value) { do_render = true; });
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // input
        // -----
        if (do_render) {
            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            RenderText(shader, buffer, 5, SCR_HEIGHT - 10, max_adv_y, 1,
                       glm::vec3(0.5, 0.8f, 0.2f));
            // RenderText(shader, "This is sample text", 25.0f, 25.0f, 1.0f,
            // glm::vec3(0.5, 0.8f, 0.2f)); RenderText(shader, "(C) LearnOpenGL.com",
            // 540.0f, 570.0f, 0.45f, glm::vec3(0.3, 0.7f, 0.9f));

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse
            // moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            do_render = false;
        } else {
            glfwWaitEventsTimeout(15);
        }
    }

    glfwTerminate();
    return 0;
}

static bool blending = true;
static bool key_pressed = false;
static bool had_input = false;

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width
    // and height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

template <typename T, typename U> constexpr auto mp(T t, U u) {
    return std::pair<T, U>{t, u};
}

constexpr auto YELLOW = glm::vec3{1.0f, 1.0f, 0.0f};
constexpr auto RED = glm::vec3{1.0f, 0.0f, 0.0f};
constexpr auto GREEN = glm::vec3{0.0f, 1.0f, 0.0f};
constexpr auto BLUE = glm::vec3{0.0f, 0.0f, 1.0f};
constexpr auto WHITE = glm::vec3{1.0f, 1.0f, 1.0f};

constexpr std::array keywords{mp("int", RED),    mp("bool", RED),
                              mp("void", RED),   mp("long", RED),
                              mp("double", RED), mp("float", RED),
                              mp("struct", RED), mp("using", BLUE),
                              mp("string", RED), mp("#include", YELLOW)};

constexpr std::array highlight{RED, GREEN, BLUE, YELLOW, WHITE};

struct Keyword {
    std::size_t begin;
    std::size_t end;
    glm::vec3 color;
};

// render line of text
// -------------------
void RenderText(Shader &shader, std::string text, float x, float y, float adv_y,
                float scale, glm::vec3 color) {
    // activate corresponding render state
    auto start_x = x;
    auto start_y = y;
    shader.use();
    auto sz = text.size();
    std::vector<std::size_t>
            del; // delimiters are separated by whitespaces, (), {} or []
    auto ptr = text.c_str();
    for (auto i = 0; i < sz; i++, ptr++) {
        if (!std::isalpha(*ptr) && *ptr != '_' && *ptr != '#') {
            del.push_back(i);
        }
    }

    std::vector<std::pair<size_t, size_t>> items;
    auto begin = 0;
    for (auto pos : del) {
        items.emplace_back(begin, pos);
        begin = pos + 1;
    }

    std::vector<std::pair<size_t, size_t>> _keywords;
    std::vector<Keyword> keywords_ranges;

    for (const auto &[begin, end] : items) {
        fmt::print("{}-{}\n", begin, end);
        auto rng = text.substr(begin, end - begin);
        for (const auto &[word, color] : keywords) {
            if (word == rng) {
                fmt::print("Found keyword {} at {}-{}\n", rng, begin, end);
                keywords_ranges.emplace_back(Keyword{begin, end, color});
            }
        }
    }
    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y,
                color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    // iterate through all characters
    auto item_it = keywords_ranges.begin();
    auto pos = 0;
    bool is_keyword = false;
    bool syntax_set = false;
    for (auto c = text.begin(); c != text.end(); c++, pos++) {
        if (item_it != keywords_ranges.end()) {
            auto &kw = *item_it;
            auto [begin, end, col] = *item_it;
            if (pos > end) {
                item_it++;
                if (item_it != keywords_ranges.end()) {
                    begin = item_it->begin;
                    end = item_it->end;
                    col = item_it->color;
                }
            }
            if (pos >= begin && pos < end) {
                auto &c = highlight[0];
                if (!syntax_set) {
                    fmt::print("Setting highlight\n");
                    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), col.x,
                                col.y, col.z);
                    syntax_set = true;
                }
            } else {
                if (syntax_set) {
                    fmt::print("Setting normal\n");
                    glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x,
                                color.y, color.z);
                    syntax_set = false;
                }
            }
        }
        if (*c == '\n') {
            y -= adv_y;
            x = start_x;
            continue;
        }

        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x; // * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
                {xpos, ypos + h, 0.0f, 0.0f},    {xpos, ypos, 0.0f, 1.0f},
                {xpos + w, ypos, 1.0f, 1.0f},

                {xpos, ypos + h, 0.0f, 0.0f},    {xpos + w, ypos, 1.0f, 1.0f},
                {xpos + w, ypos + h, 1.0f, 0.0f}};
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(
                GL_ARRAY_BUFFER, 0, sizeof(vertices),
                vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64
        // pixels)
        x += (ch.Advance >>
                         6); // * scale; // bitshift by 6 to get value in pixels (2^6 = 64
        // (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
