//
// Created by 46769 on 2020-12-25.
//

#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <GLFW/glfw3.h>

class App;

enum class Commands {
    OpenFile,
    WriteFile,
    GotoLine,
    UserCommand,
    Search,
    Fail
};

enum class Cycle : int {
    Forward = GLFW_KEY_DOWN,        // down
    Backward = GLFW_KEY_UP,       // up
};

struct EditorCommand {
    Commands type;
};

[[maybe_unused]] std::optional<int> goto_is_ok(const std::string& input);

class CommandInterpreter {
public:
    CommandInterpreter(const CommandInterpreter &) = delete;
    static CommandInterpreter &get_instance();
    void validate(const std::string &input);
    void cycle_current_command_arguments(Cycle);
    bool has_command_waiting();
    void auto_complete();
    void execute_command();
    void set_current_command_read(Commands type);

    void register_application(App* pApp);
    std::optional<std::string> current_input();
    std::string command_auto_completed();
    void evaluate_current_input();

    void clear_state();
    void setup_state();
    bool cmd_is_interactive();
    bool command_can_autocomplete();

private:
    CommandInterpreter() = default;
    App* ctx = nullptr;
    Commands current_input_command;
    EditorCommand ecmd;
    bool getting_input = false;
    /// when you hit <tab> to cycle through options of a typed in command
    /// so typing open /usr/bin/fi and then hitting <tab>, would cycle through
    /// all files in /usr/bin/ with the prefix of fi, but where would we store this data, for the application to know
    /// what state our cycling is in?
    void parse_command(std::string_view str);
};
