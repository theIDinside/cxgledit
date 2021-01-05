//
// Created by 46769 on 2020-12-30.
//

#pragma once

#include <vector>
#include <core/text_data.hpp>

template<typename T>
using Boxed = std::unique_ptr<T>;

enum BufferType { CodeInput, CommandInput };

enum ReqResult {
    Ok,
    Error
};

struct CommandResult {
    int msg_id;
    ReqResult status;
};


class DataManager {
public:
    static DataManager& get_instance();
    TextData* get_by_id(int id);
    TextData*create_managed_buffer(BufferType type);
    int get_new_id();
    CommandResult request_close(int i);
private:
    std::map<int, std::string> error_stats;
    std::vector<Boxed<TextData>> data;
    std::vector<Boxed<TextData>> reuse_list;
    static int buffers_count;
    DataManager() : error_stats(), data{} {}
};
