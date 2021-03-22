#include "app.hpp"

// Default Window Width and Window Height
constexpr auto WW = 1920;
constexpr auto WH = 1080;

int main(int argc, const char **argv) {

    if constexpr(DEBUG_IS_ON) {
        util::println("Debugging features is turned on");
    } else {
        util::println("Debugging features is turned off - Running in release mode?");
    }

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
 * TODO(feature, platform): Handle platform specific differences wrt files. Newlines under Nix = '\n', newlines under Windows: '\r\n'
 * TODO(feature, textedit): When ctrl+backspace, remove until first *non*-whitespace.
 * TODO(feature, textedit): add copy / paste commands
        - done, max copy is however single line at this point in time
         (seeing as how a GapBuffer is about to be implemented, furthering this feature right now is pointless)
 * TODO(feature, textedit): undo/redo operations.

 * TODO(abstraction, major): tie in all edits of buffer & their possible side effects behind the EditorWindow class
 * TODO(abstraction, major): tie in all edits of buffer & their possible side effects behind the EditorWindow class

 * TODO(cosmetic, minor): Add delay to syntax highlighting / lexing, so that the entire text doesn't flicker about
 * TODO(feature, filecommand): add write file command
 *  - done, but no overwrite confirmation
 * TODO(feature, filecommand): bookmarking of lines/blocks/files

 * TODO(feature, ui): add resizing of split windows
 * TODO(feature, ui): add support for layout type vertical
 * TODO(feature, ui): pop up windows / modal dialogs
 * TODO(feature, ui): block highlight

 * TODO(feature, core): when lexing the source code data, look for lines like these todo's, then parse them and add them to the buffer's meta data structure, for quick jumping/searching
 * TODO(major, core): add intelligence to editor, so that it can "reason" about the lexed and/or "parsed" source code

 * TODO(feature, navigate): search / reverse search
 * TODO(feature, navigate): history / mark ring, similiar to Emacs (and CLion. Save a history of cursor positions, for easy navigation. This should be application wide
    and not on a per-buffer basis, making jumping between last edited files more smooth

 * TODO(major): remove dependencies on GLM & GLFW, possibly freetype as well, but probably not
 *      - addendum: possibly change font dependency to STB, as this library exists on github,
 *      thus doesn't have the suckyness of freetype, which must be installed manually in the deps folder
 * TODO(feature, minor): make application file-type aware. This can have multiple uses, one is described in app.cpp when it comes to reloading
 *      settings from a .cxe file, while editing a .cxe file
 * Add some debug code for OpenGL and read up about OpenGL
 *
    GLint s;
    glGetBufferParameteriv(GL_ARRAY_BUFFER,GL_BUFFER_SIZE,&s);
    GLint s2;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER,GL_BUFFER_SIZE,&s2);
    to check if resizing of Array Buffers are done in the way I expect them to be done. This way I'll find out
    if my calculations which are hellish at points, are wrong or just bad.
 */
