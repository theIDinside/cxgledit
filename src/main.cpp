#include "app.hpp"

// Default Window Width and Window Height
constexpr auto WW = 1920;
constexpr auto WH = 1080;

int main(int argc, const char **argv) {
    auto app = App::initialize(WW, WH);
    if (argc > 1) { app->load_file(argv[1]); }
    app->run_loop();
    return 0;
}

/*
 * todos that aren't fixmes or doesn't have some locality somewhere in the code, basically features to implement
 *
 * Features to add. The feature description minor/major does not mean to say anything about it's importance, or necessity nor it's urgency
 * but is simply meant to give a hint about what works need to be put behind it and how much other features it might spawn. Minor features are not features which will
 * father other features so to speak.
 * TODO(cosmetic, minor): Add delay to syntax highlighting / lexing, so that the entire text doesn't flicker about
 * TODO(feature, minor): add write file command
 *  - done, but no overwrite affirmation
 * TODO(feature, minor): add copy / paste commands
        - done, max copy is however single line at this point in time
         (seeing as how a GapBuffer is about to be implemented, furthering this feature right now is pointless)
 * TODO(feature, minor): add resizing of split windows
 * TODO(feature, minor): add support for layout type vertical
 * TODO(feature, minor): pop up windows / modal dialogs
 * TODO(feature, minor): when lexing the source code data, look for lines like these todo's, then parse them and add them to the buffer's meta data structure, for quick jumping/searching
 * TODO(feature, minor): bookmarking of lines/blocks/files
 * TODO(feature, minor): search / reverse search
 * TODO(feature, minor): block highlight
 * TODO(feature, major): add intelligence to editor, so that it can "reason" about the lexed and/or "parsed" source code
 *
 * TODO(major): remove dependencies on GLM & GLFW, possibly freetype as well, but probably not
 *
 * Add some debug code for OpenGL and read up about OpenGL
 *
    GLint s;
    glGetBufferParameteriv(GL_ARRAY_BUFFER,GL_BUFFER_SIZE,&s);
    GLint s2;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER,GL_BUFFER_SIZE,&s2);
    to check if resizing of Array Buffers are done in the way I expect them to be done. This way I'll find out
    if my calculations which are hellish at points, are wrong or just bad.
 */
