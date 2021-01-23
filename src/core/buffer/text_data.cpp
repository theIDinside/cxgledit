//
// Created by cx on 2020-11-18.
//

#include "text_data.hpp"
#include <core/strops.hpp>

// FIXME: Fix word move forward / backward, so that cursor info data is recorded correctly. It's a mess right now
// FIXME: Fix line move backward, forward seems to work perfectly fine, line position, column info etc

#include <core/buffer/data_manager.hpp>
#include <utility>

void BufferCursor::reset() {
    line = 0;
    col_pos = 0;
    pos = 0;
}
void BufferCursor::invalidate() {
    line = -1;
    col_pos = -1;
    pos = -1;
}
BufferCursor BufferCursor::clone() const {
    return BufferCursor{.pos = pos, .line = line, .col_pos = col_pos, .buffer_id = buffer_id};
}


/// ----------- NON-PURE VIRTUAL ABSTRACT IMPL METHODS ----------------

void TextData::set_file(fs::path p) {
    file_path = p;
    meta_data.buf_name = p.filename().string();
    set_name(p.filename().string());
}

bool TextData::exist_on_disk() const { return (not file_path.empty() && fs::exists(file_path)); }

std::string TextData::fileName() { return meta_data.buf_name; }
void TextData::clear_metadata() {
    cursor.pos = 0;
    cursor.col_pos = 0;
    cursor.line = 0;
    meta_data.line_begins.clear();
    meta_data.buf_name.clear();
}

void TextData::set_name(std::string buffer_name) {
    name = std::move(buffer_name);
}
