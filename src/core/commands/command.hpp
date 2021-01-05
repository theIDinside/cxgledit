//
// Created by 46769 on 2020-12-25.
//

#pragma once


#include <core/strops.hpp>
#include <filesystem>
#include <list>
#include <memory>
#include <optional>
#include <string>

namespace fs = std::filesystem;

class App;
class TextData;

struct Command {
    explicit Command(std::string name) : name(std::move(name)) {}
    virtual ~Command() = default;
    std::string name;
    virtual bool exec(App *, TextData* buffer) = 0;
    virtual bool validate() { return true; }
    virtual void next_arg() {}
    virtual void prev_arg() {}

    virtual std::string as_auto_completed() const = 0;
    virtual std::string actual_input() const = 0;
};

struct ErrorCommand : public Command {
    explicit ErrorCommand(std::string message);
    ~ErrorCommand() override;
    bool exec(App *app, TextData* buffer) override;

    bool validate() override;
    [[nodiscard]] std::string actual_input() const override;
    [[nodiscard]] std::string as_auto_completed() const override;
    std::string msg;
    App* app;
};

struct OpenFile : public Command {
    explicit OpenFile(const std::string &argInput);
    bool exec(App *app, TextData* buffer) override;
    ~OpenFile() override = default;
    std::string actual_input() const override;
    bool validate() override;
    void next_arg() override;
    void prev_arg() override;
    [[nodiscard]] std::string as_auto_completed() const override;
    fs::path file;

    std::vector<fs::path> withSamePrefix;
    std::size_t curr_file_index = 0;
    bool fileNameSelected = false;
};

struct WriteFile : public Command {
    explicit WriteFile(const std::string& file, bool over_write = false);
    ~WriteFile() override = default;
    bool exec(App *app, TextData* buffer) override;
    bool validate() override;
    void next_arg() override;
    void prev_arg() override;
    [[nodiscard]] std::string as_auto_completed() const override;
    [[nodiscard]] std::string actual_input() const override;
    fs::path fileName;
    bool over_write{false};
};

std::optional<std::unique_ptr<Command>> parse_command(std::string input);