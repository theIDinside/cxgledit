//
// Created by 46769 on 2021-01-07.
//

#include "file_manager.hpp"
#include <ranges>
#include <core/core.hpp>

FileManager &FileManager::get_instance() {
    static FileManager fm;
    return fm;
}
void FileManager::set_user_input(std::string_view v) {
    if(not v.empty()) {
        clear_state();
        setup_state();
        prefix = v;
        auto p = fs::path{v};
        auto path = p.parent_path();
        auto file = fs::path{prefix}.filename();

        auto filePrefix = file.generic_string();

        auto keepWithPrefix = [prefix = filePrefix](auto &p) {
            auto fname = p.path().filename().generic_string();
            if (fname.size() >= prefix.size()) {
                return fname.substr(0, prefix.size()) == prefix;
            } else {
                return p.path().filename().generic_string() == prefix;
            }
        };

        constexpr auto to_path = [](auto &dirEntry) { return dirEntry.path(); };

        if (fs::exists(path) && fs::is_directory(path)) {
            auto begin = fs::directory_iterator(path);
            auto end = fs::end(fs::directory_iterator(path));

            if (fs::is_directory(prefix)) {
                auto it = std::ranges::subrange(begin, end) | std::views::filter(keepWithPrefix) |
                          std::views::transform(to_path);
                std::ranges::copy(it, std::back_inserter(withSamePrefix));
            } else {
                auto it = std::ranges::subrange(begin, end) | std::views::filter(keepWithPrefix) |
                          std::views::transform(to_path);
                std::ranges::copy(it, std::back_inserter(withSamePrefix));
            }
            // for (const auto &p : withSamePrefix) { fmt::print("{} \t", p.generic_string()); }
        } else {
            util::println("PATH DOES NOT EXIST: {}", path.generic_string());
        }
    }
}
SelectionResult FileManager::get_suggestion() const {
    if(not withSamePrefix.empty()) {
        return SelectionResult {
                prefix,
                withSamePrefix[selected_file].generic_string()
        };
    } else {
        return SelectionResult{prefix, {}};
    }
}
void FileManager::next_suggestion() {
    if(not withSamePrefix.empty()) {
        selected_file++;
        selected_file = selected_file % withSamePrefix.size();
    }
}
void FileManager::prev_suggestion() {
    if(not withSamePrefix.empty()) {
        selected_file--;
        if(selected_file < 0) {
            selected_file = withSamePrefix.size() - 1;
        }
    }
}
void FileManager::clear_state() {
    prefix.clear();
    withSamePrefix.clear();
    selected_file = 0;

}
void FileManager::setup_state() {
    current_path = fs::path{fs::current_path().generic_string() + "/"};
    prefix = current_path.generic_string();
}
