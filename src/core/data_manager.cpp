//
// Created by 46769 on 2020-12-30.
//

#include "data_manager.hpp"
#include <ranges>

int DataManager::buffers_count = 0;

DataManager &DataManager::get_instance() {
    static DataManager dm;
    return dm;
}
TextData *DataManager::get_by_id(int id) {
    for (auto &e : data) {
        if (e->id == id) { return e.get(); }
    }
    return nullptr;
}

TextData *DataManager::create_managed_buffer(BufferType type) {
    if (reuse_list.empty()) {
        util::println("No available buffers in re-use list. Creating new");
        switch (type) {
            case CodeInput: {
                auto bufHandle = StdStringBuffer::make_handle();
                bufHandle->has_meta_data = true;
                data.push_back(std::move(bufHandle));
                return data.back().get();
            }
            case CommandInput: {
                auto bufHandle = StdStringBuffer::make_handle();
                bufHandle->has_meta_data = false;
                data.push_back(std::move(bufHandle));
                return data.back().get();
            }
        }
    } else {
        auto last = reuse_list.begin() + reuse_list.size() - 1;
        data.push_back(std::move(*last));
        reuse_list.erase(last);
        auto item = data.back().get();
        if(type == BufferType::CodeInput) {
            item->has_meta_data = true;
        } else if(type == BufferType::CommandInput) {
            item->has_meta_data = false;
        } else {
            PANIC("You have to set Buffer type");
        }
        return item;
    }
}
int DataManager::get_new_id() {
    buffers_count++;
    return buffers_count;
}
CommandResult DataManager::request_close(int i) {
    // TODO: when a buffer gets closed, we will request DataManager to reclaim the buffer, adding it to an empty list
    // for which we can then re-use the memory for another buffer that wants to be opened
    auto buf = std::ranges::find_if(data, [i](auto &e) { return e->id == i; });

    if (buf != std::end(data)) {
        reuse_list.push_back(std::move(*buf));
        data.erase(buf);
        auto used = reuse_list.back().get();
        used->clear();
        used->has_meta_data = false;
        util::println("Handed buffer back to manager. Memory re-usable");
    }

    return CommandResult{.msg_id = 0, .status = ReqResult::Ok};
}
