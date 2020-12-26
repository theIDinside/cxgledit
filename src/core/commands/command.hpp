//
// Created by 46769 on 2020-12-25.
//

#pragma

#include <app.hpp>
#include <core/strops.hpp>
#include <filesystem>
#include <list>
#include <memory>
#include <optional>
#include <string>

namespace fs = std::filesystem;
struct Command {
    explicit Command(std::string name) : name(std::move(name)) {}
    virtual ~Command() = default;
    std::string name;
    virtual void exec(App*) = 0;
    virtual void tab_handle() = 0;
    virtual Command *validate() { return this; }
    virtual void next_arg() {}

    virtual std::string as_auto_completed() const = 0;
    virtual std::string actual_input() const = 0;
    virtual bool matches(std::string_view view) const = 0;
    virtual void update_state(std::string_view view) = 0;
};

struct ErrorCommand : public Command {
    explicit ErrorCommand(std::string message);
    ~ErrorCommand() override = default;
    void exec(App *app) override;
    void tab_handle() override;
    bool matches(std::string_view view) const override;
    void update_state(std::string_view view) override;
    Command *validate() override;
    std::string actual_input() const override;
    [[nodiscard]] std::string as_auto_completed() const override;
    std::string msg;
};

struct OpenFile : public Command {
    explicit OpenFile(const std::string &path);
    void exec(App *app) override;
    void tab_handle() override;
    ~OpenFile() override = default;
    bool matches(std::string_view view) const override;
    void update_state(std::string_view view) override;
    std::string actual_input() const override;
    Command *validate() override;
    void next_arg() override;
    [[nodiscard]] std::string as_auto_completed() const override;
    fs::path file;

    std::vector<fs::path> withSamePrefix;
    std::size_t curr_file_index = 0;
    bool fileNameSelected = false;
};

std::optional<std::unique_ptr<Command>> parse_command(std::string input);