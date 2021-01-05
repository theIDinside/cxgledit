//
// Created by 46769 on 2020-12-25.
//

#include "command.hpp"
#include "command_interpreter.hpp"
#include <fmt/format.h>
#include <ranges>
#include <fstream>
#include <core/text_data.hpp>
#include <app.hpp>

constexpr auto is_dir = [](const auto &path) { return fs::exists(path) && path.filename().empty(); };

bool OpenFile::exec(App *app, TextData* buffer) {
    if (fileNameSelected) {
        auto load_file = withSamePrefix[curr_file_index];
        if(buffer->empty()) {
            if(!fs::exists(load_file)) {
                return false;
            } else {
                auto abspath = fs::absolute(load_file);
                auto fsz = fs::file_size(load_file);
                auto askForFileSz = file_size(abspath.string().c_str());
                util::println("Active text buffer is empty. Filesystem reported file size of {} ({}) bytes. Loading data into it.", askForFileSz.value(), fsz);
                std::string tmp;
                if(askForFileSz)
                    tmp.reserve(askForFileSz.value() * 2);

                std::ifstream f{load_file};
                tmp.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
                util::println("Loaded {} bytes from file {}", tmp.size(), file.string());
                auto act_view = app->get_active_view();
                act_view->set_name(load_file.string());
                buffer->load_string(std::move(tmp));
                buffer->set_file(load_file);
                return true;
            }
        } else {

            auto prior_view = app->get_active_view();
            auto prior_buf = prior_view->get_text_buffer();
            util::println("Active view name: '{}'. Active buffer: {}", prior_view->name, prior_buf->id);

            app->new_editor_window(SplitStrategy::VerticalSplit);
            // app->new_buffer_and_view_set_as_active();
            auto new_view = app->get_active_view();
            auto buf = new_view->get_text_buffer();
            new_view->set_name(load_file.string());
            std::string tmp;
            std::ifstream f{load_file};
            tmp.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
            util::println("Loaded {} bytes from file {} and placed it in new view.", tmp.size(), file.string());
            buf->load_string(std::move(tmp));
            buf->set_file(load_file);
            return true;
        }
    } else {
        util::println("No file selected or found with that name: {}", file.string());
        return false;
    }
}

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

    if (fs::exists(path) && fs::is_directory(path)) {
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
            if (curr_file_index <= 0) {
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
bool ErrorCommand::exec(App *app, TextData* buffer) {
    util::println("Setting error message");
    app->set_error_message(msg);
    app = app;
    return true;
}

bool ErrorCommand::validate() { return true; }

ErrorCommand::ErrorCommand(std::string message) : Command("ErrorCommand"), msg(std::move(message)) {}
std::string ErrorCommand::as_auto_completed() const { return msg; }
std::string ErrorCommand::actual_input() const { return "Error: " + msg; }

ErrorCommand::~ErrorCommand() {

}

bool WriteFile::exec(App *app, TextData* buffer) {
    if(!over_write && fs::exists(fileName)) {
        util::println("File exists and write protection is on");
        CommandInterpreter::get_instance().destroy_cmd_and_set_new(new ErrorCommand{fmt::format("File {} exists [add ! prefix to command to override]", fileName.filename().string())});
        return false;
    } else {
        std::ofstream outf{fileName};
        auto write_data = app->get_active_view()->get_text_buffer()->to_string_view();
        outf << write_data;
        outf.close();

        auto checked_bytes_written = file_size(fileName.string().c_str());
        if(checked_bytes_written) {
            util::println("Wrote {} bytes to file", checked_bytes_written.value());
        } else {
            util::println("Could not retrieve file size");
        }
        return true;
    }
}
bool WriteFile::validate() { return Command::validate(); }
void WriteFile::next_arg() { Command::next_arg(); }
void WriteFile::prev_arg() { Command::prev_arg(); }


std::string WriteFile::as_auto_completed() const { return "write " + fileName.string(); }
std::string WriteFile::actual_input() const { return as_auto_completed(); }

WriteFile::WriteFile(const std::string &file, bool over_write)
    : Command("WriteFile"), fileName(file), over_write(over_write) {}
