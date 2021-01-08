//
// Created by 46769 on 2021-01-07.
//
#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

struct SelectionResult {
    std::string prefix;
    std::optional<std::string> suggestion;
};

class FileManager {
public:
    FileManager(const FileManager&) = delete;
    static FileManager& get_instance();
    void set_user_input(std::string_view v);
    [[nodiscard]] SelectionResult get_suggestion() const;
    void next_suggestion();
    void prev_suggestion();
    void clear_state();
    void setup_state();
private:
    FileManager() = default;
    std::string prefix; // basically, user input
    fs::path current_path;
    std::vector<fs::path> withSamePrefix;
    int selected_file = 0;
};