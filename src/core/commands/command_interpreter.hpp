//
// Created by 46769 on 2020-12-25.
//

#pragma once

#include "command.hpp"
#include <string>
#include <string_view>
#include <array>
#include <list>
#include <optional>

class CommandInterpreter {
public:
    CommandInterpreter(const CommandInterpreter&) = delete;
    static CommandInterpreter& get_instance();
    void validate(const std::string& input);
    void cycle_current_command_arguments();
    void destroy_current_command();
    Command* finalize();

    std::optional<std::string_view> next_history_input() const;
    std::optional<std::string_view> prev_history_input() const;
    bool has_command_waiting();
    Command* get_currently_edited_cmd();
    void destroy_cmd_and_set_new(Command* cmd);
    void destroy_current_command_if_no_match(std::string_view view);

    void auto_complete();

private:
    CommandInterpreter() = default;
    Command* current_command = nullptr;

    /// when you hit <tab> to cycle through options of a typed in command
    /// so typing open /usr/bin/fi and then hitting <tab>, would cycle through
    /// all files in /usr/bin/ with the prefix of fi, but where would we store this data, for the application to know
    /// what state our cycling is in?
    std::list<std::string> temporary_arguments{};
    std::array<std::string, 100> input_history{};
    std::size_t history_length{0};
    std::size_t current_history_index{0};
};


