#include "app.hpp"
// Default Window Width and Window Height
constexpr auto WW = 1024;
constexpr auto WH = 768;


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
 * TODO(feature, minor): add write file command
 * TODO(feature, minor): add copy / paste commands
 * TODO(feature, minor): add select (so one can copy)
 * TODO(feature, minor): add page up / page down scrolling
 * TODO(feature, minor): add resizing of split windows
 * TODO(feature, minor): add support for layout type vertical
 * TODO(feature, minor): pop up windows / modal dialogs
 * TODO(feature, minor): when lexing the source code data, look for lines like these todo's, then parse them and add them to the buffer's meta data structure, for quick jumping/searching
 * TODO(feature, minor): bookmarking of lines/blocks/files
 * TODO(feature, minor): search / reverse search
 * TODO(feature, minor): block highlight
 * TODO(feature, major): add intelligence to editor, so that it can "reason" about the lexed and/or "parsed" source code
 */
