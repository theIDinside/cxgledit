#include <string>
#include <string_view>
#include "someheader.hpp"
// Color scheme is *definitely* bound to change


using namespace std::string_view_literals;
constexpr std::string_view greeting{"hello world!"};

// line comments work
/*
 *  Block comments work. Doc comments also work, but currently does not render
 *  different from regular comments (which they probably should)
 */

// qualifiers kind of work, number & string literals work
const int foo = 10;
// also hex works
constexpr int hex = 0x0f1afed;
// or float

constexpr float f = 0.1123f;
int main(int argc, const char** argv) {
    std::string s;
    std::cout << greeting << ", what's going on?\n";
    auto ill_tell_ya = [](auto name) {
      // bug: name in return stmt should not be colorized
      return "I'll tell you " + name + ", what's going on: " + std::to_string(42) + "\n";
    };

    std::cout << ill_tell_ya();
}