//
// Created by 46769 on 2020-12-25.
//

#include "command.hpp"
#include "command_interpreter.hpp"
#include <fmt/format.h>
#include <ranges>

constexpr auto is_dir = [](const auto &path) { return fs::exists(path) && path.filename().empty(); };

void OpenFile::exec(App *app) {
    if (fileNameSelected) {
        app->load_file(withSamePrefix[curr_file_index]);
    } else {
        util::println("No file selected or found with that name: {}", file.string());
    }
}
void OpenFile::tab_handle() {}

OpenFile::OpenFile(const std::string &argInput) : Command("OpenFile"), file{argInput}, withSamePrefix{} {
    auto path = file.parent_path();
    if (path.empty()) {
        path = "./";
        file = path / file;
    }
    auto filePrefix = file.filename().string();

    auto keepWithPrefix = [prefix = filePrefix](auto &p) {
        auto fname = p.path().filename().string();
        if (fname.size() >= prefix.size()) {
            return fname.substr(0, prefix.size()) == prefix;
        } else {
            return p.path().filename().string() == prefix;
        }
    };

    constexpr auto to_path = [](auto &dirEntry) { return dirEntry.path(); };

    if (fs::exists(path)) {
        auto begin = fs::directory_iterator(path);
        auto end = fs::end(fs::directory_iterator(path));

        if (fs::is_directory(argInput)) {
            // we have writen for instance /foo/bar/ - so hitting tab here should cycle through everything in bar
            auto it = std::ranges::subrange(begin, end) | std::views::filter(keepWithPrefix) |
                      std::views::transform(to_path);
            std::ranges::copy(it, std::back_inserter(withSamePrefix));
        } else {
            // we have written e.g. /foo/bar/ta - hitting tab will cycle through everything with the prefix "ta" in dir
            auto it = std::ranges::subrange(begin, end) | std::views::filter(keepWithPrefix) |
                      std::views::transform(to_path);
            std::ranges::copy(it, std::back_inserter(withSamePrefix));
        }
        for (const auto &p : withSamePrefix) { fmt::print("{} \t", p.string()); }
    } else {
        util::println("PATH DOES NOT EXIST: {}", path.string());
    }
}

static bool some_validation_process;

bool OpenFile::validate() {
    if (fs::exists(file)) {
        util::println("---- file selected: [{}]", this->file.string());
        fileNameSelected = true;
        return true;
    } else {
        util::println("file selected did not exist");
        std::string msg{"Filename "};
        msg.append(file.string());
        msg.append(" doesn't exist.");
        CommandInterpreter::get_instance().destroy_current_command();
        CommandInterpreter::get_instance().destroy_cmd_and_set_new(new ErrorCommand{msg});
        return false;
    }
}
void OpenFile::next_arg() {
    if (!withSamePrefix.empty()) {
        if (!fileNameSelected) {
            fileNameSelected = true;
        } else {
            curr_file_index = ++curr_file_index % withSamePrefix.size();
        }
    }
}

void OpenFile::prev_arg() {
    if (!withSamePrefix.empty()) {
        if (!fileNameSelected) {
            fileNameSelected = true;
        } else {
            if(curr_file_index <= 0) {
                curr_file_index = withSamePrefix.size() - 1;
            } else {
                curr_file_index--;
            }
        }
    }
}

std::string OpenFile::as_auto_completed() const {
    if (fileNameSelected) {
        std::string str_rep{"open "};
        if (!withSamePrefix.empty()) {
            str_rep.append(withSamePrefix[curr_file_index].string());
        } else {
            str_rep.append(file.string());
        }
        return str_rep;
    } else {
        return std::string{"open "} + file.string();
    }
}
std::string OpenFile::actual_input() const { return "open " + file.string(); }



std::optional<std::unique_ptr<Command>> parse_command(std::string input) {
    auto str_parts = util::str::list_split_string(input);
    if (str_parts.empty()) return {};
    auto command = str_parts.front();
    str_parts.pop_front();
    return {};
}
void ErrorCommand::exec(App *app) {}
void ErrorCommand::tab_handle() {}

bool ErrorCommand::validate() { return true; }

ErrorCommand::ErrorCommand(std::string message) : Command("ErrorCommand"), msg(std::move(message)) {}
std::string ErrorCommand::as_auto_completed() const { return "ERROR"; }
std::string ErrorCommand::actual_input() const { return "ERROR: " + msg; }