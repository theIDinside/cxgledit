//
// Created by 46769 on 2020-12-25.
//

#include "command_interpreter.hpp"
#include "file_manager.hpp"
#include <algorithm>
#include <app.hpp>

#include <fmt/format.h>
#include <string>
#include <ui/editor_window.hpp>
#include <ui/view.hpp>

CommandInterpreter &CommandInterpreter::get_instance() {
    static CommandInterpreter ci;
    return ci;
}

void CommandInterpreter::validate(const std::string &input) {}

void CommandInterpreter::cycle_current_command_arguments(Cycle direction) {
    if (getting_input) {
        if (direction == Cycle::Forward) {
            switch (ecmd.type) {
                case Commands::OpenFile: {
                    auto &fm = FileManager::get_instance();
                    fm.next_suggestion();
                    break;
                }
                case Commands::WriteFile:
                case Commands::GotoLine:
                case Commands::Fail:
                    break;
            }
        } else if (direction == Cycle::Backward) {
            switch (ecmd.type) {
                case Commands::OpenFile: {
                    auto &fm = FileManager::get_instance();
                    fm.prev_suggestion();
                    break;
                }
                case Commands::WriteFile:
                case Commands::GotoLine:
                case Commands::Fail:
                    break;
            }
        }
    }
}

bool CommandInterpreter::has_command_waiting() { return getting_input; }

void CommandInterpreter::auto_complete() {
    if (getting_input) {
        switch (current_input_command) {
            case Commands::OpenFile: {
                auto completed = command_auto_completed();
                ctx->get_command_view()->input_buffer->clear();
                ctx->get_command_view()->input_buffer->insert_str(completed);
                auto &fm = FileManager::get_instance();
                fm.set_user_input(completed);
            } break;
            case Commands::WriteFile: {

            } break;
            case Commands::GotoLine: {

            } break;
            case Commands::Fail: {
            } break;
        }
    }
}

void CommandInterpreter::set_current_command_read(Commands type) {
    switch (type) {
        case Commands::OpenFile: {
            auto &fm = FileManager::get_instance();
            fm.clear_state();
            fm.setup_state();
        } break;
        case Commands::WriteFile: {
            auto buffer = ctx->get_active_window()->get_text_buffer();
            ctx->get_command_view()->input_buffer->clear();
            ctx->get_command_view()->input_buffer->insert_str(buffer->file_path.string());
        } break;
        case Commands::GotoLine:
            break;
        case Commands::Fail:
            break;
        case Commands::UserCommand:
            break;
        case Commands::Search:
            break;
    }
    getting_input = true;
    ecmd = EditorCommand{.type = type};
    current_input_command = type;
}

void CommandInterpreter::execute_command() {
    switch (ecmd.type) {
        case Commands::GotoLine:
            if (auto l = goto_is_ok(ctx->get_command_view()->input_buffer->to_std_string()); l) {
                ctx->editor_window_goto(l.value());
            }
            break;
        case Commands::OpenFile: {
            auto &fm = FileManager::get_instance();
            auto [p, s] = fm.get_suggestion();
            if (fs::is_regular_file(s.value_or(p))) {
                if (s) {
                    ctx->load_file(s.value());
                } else {
                    util::println("File doesn't exist: {}", p);
                }
            }
        } break;
        case Commands::WriteFile:
            ctx->fwrite_active_to_disk(ctx->get_command_view()->input_buffer->to_std_string());
            break;
        case Commands::Fail:
            PANIC("Not yet implemented command");
            break;
        case Commands::UserCommand:
            parse_command(ctx->get_command_view()->input_buffer->to_std_string());
            break;
        case Commands::Search:
            auto search_for = ctx->get_command_view()->input_buffer->to_std_string();
            ctx->find_next_in_active(search_for);
            ctx->last_searched = search_for;
            break;
    }
    ctx->restore_input();
    getting_input = false;
}

void CommandInterpreter::evaluate_current_input() {
    auto str = ctx->get_command_view()->input_buffer->to_std_string();
    switch (this->current_input_command) {
        case Commands::OpenFile: {
            auto &fm = FileManager::get_instance();
            fm.set_user_input(str);
            break;
        }
        case Commands::WriteFile:
            break;
        case Commands::GotoLine:
            break;
        case Commands::Fail:
            break;
    }
}
void CommandInterpreter::register_application(App *pApp) { this->ctx = pApp; }

/// Returns what the user has input so far. It's only partially validated
std::optional<std::string> CommandInterpreter::current_input() {
    switch (ecmd.type) {
        case Commands::OpenFile: {
            auto &fm = FileManager::get_instance();
            auto [prefix, suggestion] = fm.get_suggestion();
            return prefix;
        }
        case Commands::WriteFile: {
            auto file_str_path = ctx->get_active_window()->get_text_buffer()->file_path.string();
            return file_str_path;
        }
        case Commands::Search:
            [[fallthrough]];
        case Commands::GotoLine:
            return ctx->get_command_view()->input_buffer->to_std_string();
        case Commands::Fail:
            break;
        case Commands::UserCommand:
            break;
    }
    return {};
}

/// This function returns the full well-formed command, autocompleted by using current user input. If you want
/// what the user actually has typed in so far, call current_input() instead
std::string CommandInterpreter::command_auto_completed() {
    switch (ecmd.type) {
        case Commands::OpenFile: {
            auto &fm = FileManager::get_instance();
            auto [prefix, suggestion] = fm.get_suggestion();
            return suggestion.value_or(prefix);
        }
        case Commands::Search:
            [[fallthrough]];
        case Commands::WriteFile:
            [[fallthrough]];
        case Commands::UserCommand:
            [[fallthrough]];
        case Commands::GotoLine:
            [[fallthrough]];
            return ctx->get_command_view()->input_buffer->to_std_string();
        case Commands::Fail:
            break;
    }
}

void CommandInterpreter::clear_state() {
    FileManager::get_instance().clear_state();
    getting_input = false;
}
void CommandInterpreter::setup_state() {
    FileManager::get_instance().setup_state();
    getting_input = true;
}
bool CommandInterpreter::cmd_is_interactive() {
    switch (this->ecmd.type) {
        case Commands::Search:
        case Commands::OpenFile:
        case Commands::WriteFile:
        case Commands::UserCommand:
            return true;
        case Commands::GotoLine:
        case Commands::Fail:
            return false;
    }
}
using namespace std::string_view_literals;

void CommandInterpreter::parse_command(std::string_view str) {
    auto delim = str.find(' ');
    auto cmd_str_rep = str.substr(0, delim);
    if (cmd_str_rep == "font"sv) {
        str.remove_prefix(delim + 1);
        if (auto pos = str.find(' '); pos == std::string_view::npos) {
            try {
                auto v = std::stoi(std::string{str});
                auto f = FontLibrary::get_instance().get_font_with_size(v);
                if (f) {
                    ctx->get_active_window()->view->set_font(*f);
                } else {
                    ctx->get_command_view()->draw_error_message(fmt::format("No font with size {} found!", v));
                }
            } catch (...) { util::println("parsing string to number failed"); }
        }
    } else if (cmd_str_rep == "kbreload") {
        ctx->reload_keybindings();
    }
}
bool CommandInterpreter::command_can_autocomplete() {
    switch (ecmd.type) {
        case Commands::OpenFile:
        case Commands::UserCommand:
            return true;
        case Commands::WriteFile:
        case Commands::GotoLine:
        case Commands::Fail:
            break;
    }
    return false;
}

std::optional<int> goto_is_ok(const std::string &input) {
    util::println("Validating input for GoTo command");
    auto inputOk = not input.empty() &&
                   std::all_of(input.c_str(), input.c_str() + input.size(), [](auto c) { return std::isdigit(c); });
    if (inputOk) {
        auto line_no = std::stoi(input);
        return line_no;
    } else {
        util::println("Input not good: '{}'", input);
        return {};
    }
}
