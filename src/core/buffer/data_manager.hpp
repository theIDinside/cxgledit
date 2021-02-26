//
// Created by 46769 on 2020-12-30.
//

#pragma once

#include <vector>
#include <core/buffer/text_data.hpp>

template<typename T>
using Boxed = std::unique_ptr<T>;

enum class BufferType { CodeInput, CommandInput, StatusBar };

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
    TextData* create_managed_buffer(BufferType type);
    TextData *create_free_buffer(BufferType type);

    int get_new_id();
    CommandResult request_close(int i);
    [[nodiscard]] int reuseable_buffers() const;
    bool is_managed(int buffer_id);
    void print_all_managed();

private:
    std::vector<Boxed<TextData>> data;
    std::vector<Boxed<TextData>> reuse_list;
    static int buffers_count;
    DataManager() : data{} {}
};
