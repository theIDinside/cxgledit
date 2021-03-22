//
// Created by 46769 on 2021-01-07.
//

#include "file_manager.hpp"
#include <ranges>
#include <algorithm>

FileManager &FileManager::get_instance() {
    static FileManager fm;
    return fm;
}
void FileManager::set_user_input(std::string_view v) {
    if (not v.empty()) {
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
#ifdef _WIN32
            /// NOTE(important): This *have* to be wrapped in a *horrible* if-statement checking for "con"
            /// because Windows is an absolute shit OS, so trying to check for a directory which has the name/prefix "con"
            /// *will* crash the standard library, as simply "con" is an illegal name in Windows. so fs::is_directory("con")
            /// *will* crash the program. Holy shit that Windows is this hilariously bad in 2021.
            if (filePrefix != "con") {
                if (fs::is_directory(prefix)) {
                    auto it = std::ranges::subrange(begin, end) | std::views::filter(keepWithPrefix) |
                              std::views::transform(to_path);
                    std::ranges::copy(it, std::back_inserter(withSamePrefix));
                } else {
                    auto it = std::ranges::subrange(begin, end) | std::views::filter(keepWithPrefix) |
                              std::views::transform(to_path);
                    std::ranges::copy(it, std::back_inserter(withSamePrefix));
                }
            }
#else
            for(const auto& it : fs::directory_iterator(path)) {
                if(keepWithPrefix(it)) {
                    withSamePrefix.push_back(to_path(it));
                }
            }
#endif
        }
    }
}
SelectionResult FileManager::get_suggestion() const {
    if (not withSamePrefix.empty()) {
        if (fs::is_directory(withSamePrefix[selected_file])) {
            auto path_str = withSamePrefix[selected_file].generic_string();
            path_str.push_back('/');
            return SelectionResult{prefix, path_str};
        } else {
            return SelectionResult{prefix, withSamePrefix[selected_file].generic_string()};
        }
    } else {
        return SelectionResult{prefix, {}};
    }
}
void FileManager::next_suggestion() {
    if (not withSamePrefix.empty()) {
        selected_file++;
        selected_file = selected_file % withSamePrefix.size();
    }
}
void FileManager::prev_suggestion() {
    if (not withSamePrefix.empty()) {
        selected_file--;
        if (selected_file < 0) { selected_file = withSamePrefix.size() - 1; }
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
