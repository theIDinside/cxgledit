//
// Created by 46769 on 2020-12-25.
//

#include "command_interpreter.hpp"
#include <utils/utils.hpp>

CommandInterpreter &CommandInterpreter::get_instance() {
    static CommandInterpreter ci;
    return ci;
}

void CommandInterpreter::validate(const std::string &input) {
    util::println("Attempting to validate in CI, input: [{}]", input);
    auto parts = util::str::list_split_string(input, ' ');
    util::println("input split into {} parts:", parts.size());
    if (!parts.empty()) {
        auto cmd_str = parts.front();
        if (cmd_str == "open") {
            parts.pop_front();
            if (!parts.empty()) {
                auto f = parts.front();
                if (f.size() > 1 && f.substr(f.size() - 2) == "..") {
                    std::string param{f};
                    param.append("/");
                    current_command = new OpenFile{param};
                } else {
                    current_command = new OpenFile{std::string{f}};
                }
                util::println("Validated an open command");
            } else {
                current_command = new OpenFile{""};
            }
        } else {
        }
    }
}

void CommandInterpreter::cycle_current_command_arguments() {
    if (current_command) {
        current_command->next_arg();
        util::println("cmd rep: [{}]", current_command->as_auto_completed());
    }
}
void CommandInterpreter::destroy_current_command() {
    if (current_command) {
        util::println("Destroying command [{}]", current_command->as_auto_completed());
        safe_delete(current_command);
    }
}
bool CommandInterpreter::has_command_waiting() { return current_command != nullptr; }
Command *CommandInterpreter::get_currently_edited_cmd() {
    if (!current_command) {}
    return current_command;
}
void CommandInterpreter::destroy_current_command_if_no_match(std::string_view view) {
    PANIC("NOT YET IMPLEMENTED");
    /*
    if (current_command) {
        if (current_command->matches(view)) {
            current_command->update_state(view);
        } else {
            safe_delete(current_command);
        }
    }
     */
}
Command *CommandInterpreter::finalize() {
    if (auto i = dynamic_cast<OpenFile *>(current_command); i) {
        if (i->fileNameSelected) {
            i->file = i->withSamePrefix[i->curr_file_index];
        }
    }
    return current_command;
}
void CommandInterpreter::auto_complete() {
    if (current_command) {
        auto cmd = current_command->as_auto_completed();
        util::println("Auto completing command: {}", cmd);
        safe_delete(current_command);
        validate(cmd);
        // current_command->validate();
        current_command->next_arg();
        util::println("Command after it's autocompleted: {}", current_command->as_auto_completed());
        current_command->validate();
        util::println("cmd rep: [{}]", current_command->as_auto_completed());
    }
}
void CommandInterpreter::destroy_cmd_and_set_new(Command *cmd) {
    destroy_current_command();
    current_command = cmd;
}
