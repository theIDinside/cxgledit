#include "app.hpp"
// Default Window Width and Window Height
constexpr auto WW = 1024;
constexpr auto WH = 768;

int main(int argc, const char **argv) {
    auto app = App::create(WW, WH);
    if (argc > 1) { app->load_file(argv[1]); }
    app->run_loop();
    return 0;
}
