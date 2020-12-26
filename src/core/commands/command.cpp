//
// Created by 46769 on 2020-12-25.
//

#include "command.hpp"
#include <fmt/format.h>
#include <ranges>
#include <utils/utils.hpp>
#include <core/core.hpp>


constexpr auto is_dir = [](const auto &path) { return fs::exists(path) && path.filename().empty(); };

void OpenFile::exec(App* app) {
    if(fileNameSelected) {
        app->load_file(withSamePrefix[curr_file_index]);
    } else {
        util::println("No file selected or found with that name: {}", file.string());
    }
}
void OpenFile::tab_handle() {}

OpenFile::OpenFile(const std::string &p) : Command("OpenFile"), file{p}, withSamePrefix{} {
    util::println("Command input: [{}]", p);
    auto path = file.parent_path();
    if(path.empty()) {
        path = "./";
        file = path / file;
    }
    util::println("File [{}] - Path: [{}]", file.string(), path.string());
    auto filePrefix = file.filename().string();
    auto keepWithPrefix = [prefix = filePrefix](auto &p) {
      auto fname = p.path().stem().string();
      if (fname.size() >= prefix.size()) { return fname.substr(0, prefix.size()) == prefix; }
      return false;
    };

    constexpr auto to_path = [](auto &dirEntry) { return dirEntry.path(); };

    auto begin = fs::directory_iterator(path);
    auto end = fs::end(fs::directory_iterator(path));

    if (fs::is_directory(p)) {
        // we have writen for instance /foo/bar/ - so hitting tab here should cycle through everything in bar
        auto it = std::ranges::subrange(begin, end) | std::views::transform(to_path);
        std::ranges::copy(it, std::back_inserter(withSamePrefix));
    } else {
        // we have written e.g. /foo/bar/ta - hitting tab will cycle through everything with the prefix "ta" in dir
        auto it =
                std::ranges::subrange(begin, end) | std::views::filter(keepWithPrefix) | std::views::transform(to_path);
        std::ranges::copy(it, std::back_inserter(withSamePrefix));
    }
}

static bool some_validation_process;

Command *OpenFile::validate() {
    if (some_validation_process) {
        return this;
    } else {
        delete this;
        std::string msg{"Filename "};
        msg.append(file.string());
        msg.append(" doesn't exist.");
        return new ErrorCommand{msg};
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
std::string OpenFile::as_auto_completed() const {
    if (fileNameSelected) {
        std::string str_rep{"open "};
        if(!withSamePrefix.empty())
            str_rep.append(withSamePrefix[curr_file_index].string());
        return str_rep;
    } else {
        return std::string{"open "} + file.string();
    }
}
std::string OpenFile::actual_input() const {
    return "open " + file.string();
}

bool OpenFile::matches(std::string_view view) const {
    auto filestr = this->file.string();
    if(view.substr(0, filestr.size()) == filestr) {
        for(fs::path p : this->withSamePrefix) {
            auto tstr = p.string();
            if(view == tstr.substr(0, view.size())) {
                return true;
            }
        }
        return false;
    } else {
        return false;
    }
}

void OpenFile::update_state(std::string_view view) {
    PANIC("Not implemented");
}

std::optional<std::unique_ptr<Command>> parse_command(std::string input) {
    auto str_parts = util::str::list_split_string(input);
    if (str_parts.empty()) return {};
    auto command = str_parts.front();
    str_parts.pop_front();
    return {};
}
void ErrorCommand::exec(App* app) {}
void ErrorCommand::tab_handle() {}

Command *ErrorCommand::validate() { return nullptr; }

ErrorCommand::ErrorCommand(std::string message) : Command("ErrorCommand"), msg(std::move(message)) {}
std::string ErrorCommand::as_auto_completed() const { return "ERROR"; }
std::string ErrorCommand::actual_input() const {
    return "ERROR: " + msg;
}
bool ErrorCommand::matches(std::string_view view) const { return false; }
void ErrorCommand::update_state(std::string_view view) {}
