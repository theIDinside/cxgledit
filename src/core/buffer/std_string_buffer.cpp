//
// Created by 46769 on 2021-01-23.
//

#include "std_string_buffer.hpp"
#include "data_manager.hpp"
#include <core/strops.hpp>
#include <ui/view.hpp>

void StdStringBuffer::move_cursor(Movement m) {
    switch (m.construct) {
        case Char:
            m.dir == CursorDirection::Forward ? char_move_forward(m.count) : char_move_backward(m.count);
            break;
        case Word:
            m.dir == CursorDirection::Forward ? word_move_forward(m.count) : word_move_backward(m.count);
            break;
        case Line:
            m.dir == CursorDirection::Forward ? line_move_forward(m.count) : line_move_backward(m.count);
            break;
        default:
            PANIC("Block and file movements not yet implemented");
    }
    state_is_pristine = false;
}
void StdStringBuffer::char_move_forward(std::size_t count) {
    /*
    auto last_col = cursor.col_pos;
    auto last_line = cursor.line;
    auto last_pos = cursor.pos;
*/
    if (cursor.pos + count >= size()) {
        auto b = store.begin();
        std::advance(b, cursor.pos);
        for (; b < store.end(); b++) {
            if (*b == '\n') {
                cursor.line++;
                cursor.col_pos = 0;
            } else {
                cursor.col_pos++;
            }
        }
        cursor.pos = size();
    } else {
        auto b = store.begin();
        std::advance(b, cursor.pos);
        auto e = store.begin() + cursor.pos + count;
        for (; b < e; b++) {
            if (*b == '\n') {
                cursor.line++;
                cursor.col_pos = 0;
            } else {
                cursor.col_pos++;
            }
            cursor.pos++;
        }
    }
    // util::println("Move from [i:{}, ln: {}, col: {}] to [i:{}, ln: {}, col: {}]", last_pos, last_line, last_col, cursor.pos, cursor.line, cursor.col_pos);
}

void StdStringBuffer::char_move_backward(std::size_t count) {
    /*
    auto last_col = cursor.col_pos;
    auto last_line = cursor.line;
    auto last_pos = cursor.pos;
*/
    if (static_cast<int>(cursor.pos) - static_cast<int>(count) <= 0) {
        cursor.reset();
    } else {
        auto next_pos = cursor.pos - count;
        for (auto index = cursor.pos; index > next_pos; index--) {
            if (store[index - 1] == '\n') { cursor.line--; }
            cursor.pos--;
        }
        auto i = cursor.pos;
        if (store[i] == '\n') i--;
        bool found = false;
        for (; i > 0 && !found; i--) {
            if (store[i] == '\n') {
                cursor.col_pos = cursor.pos - i - 1;
                found = true;
            }
        }
        if (!found) cursor.col_pos = cursor.pos;
    }
}

void StdStringBuffer::word_move_forward(std::size_t count) {
    auto sz = size();
    if (cursor.pos + 1 >= sz) {
        step_cursor_to(sz);
        return;
    }
    auto new_pos = find_next_delimiter(cursor.pos);
    count--;
    for (; count > 0; --count) { new_pos = find_next_delimiter(new_pos); }
    char_move_forward(new_pos - cursor.pos);
    // step_cursor_to(new_pos);
}

void StdStringBuffer::word_move_backward(std::size_t count) {
    if (cursor.pos - 1 <= 0) {
        step_cursor_to(0);
        return;
    }
    int new_pos = this->find_prev_delimiter(cursor.pos);
    count--;
    for (; count > 0; --count) { new_pos = find_prev_delimiter(new_pos); }
    char_move_backward(cursor.pos - new_pos);
}

int StdStringBuffer::find_line_end(int i) {
    auto sz = size();
    for (; i < sz; i++) {
        if (store[i] == '\n') { break; }
    }
    return std::min(i, int(size()));
}

int StdStringBuffer::find_line_begin(int i) {
    if (i <= 0) return 0;
    if (store[i] == '\n') {
        return i;
    } else {
        for (; i > 0; --i) {
            if (store[i] == '\n') { break; }
        }
        return std::max(0, i);
    }
}

/**
 * Returns the character *after* the newline which exists in the line prior to the line where position i is. Contrast
 * that with find_line_begin, which returs the index of the newline character of the prior line
 * @param i
 * @return
 */
int StdStringBuffer::find_line_start(Boundary boundary, int i) {
    i -= 1;
    if(boundary == Boundary::Outside) {
        if (i <= 0) return 0;
        if (store[i] == '\n') {
            return i;
        } else {
            for (; i > 0; --i) {
                if (store[i] == '\n') { break; }
            }
            if (i <= 0) return 0;// BOF - Beginning of file
            else
                return i + 1;// BOL - Beginning of line
        }
    } else {
        if (i <= 0) return 0;
        if (store[i] == '\n') {
            return i + 1;
        } else {
            for (; i > 0; --i) {
                if (store[i] == '\n') { break; }
            }
            if (i <= 0) return 0;// BOF - Beginning of file
            else
                return i + 1;// BOL - Beginning of line
        }
    }
}

void StdStringBuffer::line_move_forward(std::size_t count) {
    auto last_col = cursor.col_pos;
    auto pos = cursor.pos;
    auto sz = size();
    for (; pos < sz && count > 0; pos++) {
        if (store[pos] == '\n') count--;
    }
    if (pos + last_col < sz) {
        auto line_end = this->find_line_end(pos);
        if (pos + last_col > line_end) {
            step_cursor_to(line_end);
        } else {
            step_cursor_to(pos + last_col);
        }
    } else {
        step_cursor_to(sz);
    }
}

void StdStringBuffer::line_move_backward(std::size_t count) {
    int curr_column = cursor.col_pos;
    auto pos = cursor.pos;
    if (store[pos] == '\n') count++;
    for (; pos > 0 && count > 0; pos--) {
        if (store[pos] == '\n') {
            count--;
            if (count == 0) break;
        }
    }
    auto line_length = (pos - find_line_start(Boundary::Inside, pos));
    if (curr_column > line_length) {
        char_move_backward(cursor.pos - pos);
    } else {
        auto p = find_line_start(Boundary::Inside, pos) + curr_column;
        char_move_backward(cursor.pos - p);
    }
}

void StdStringBuffer::erase() { store.erase(cursor.pos, 1); }

void StdStringBuffer::insert_str(const std::string_view &data) {
    if (store.capacity() <= store.size() + data.size()) { store.reserve(store.capacity() * 2); }
    store.insert(cursor.pos, data);
    auto inc = data.size();
    cursor.pos += inc;
    if (auto nlines = count_elements(data, '\n'); nlines) {
        auto &ref = nlines.value();
        auto data_index = ref.back().found_at_idx;
        auto nlines_count = ref.size();
        cursor.line += nlines_count;
        if (cursor.pos > data_index) {
            cursor.col_pos = std::abs(static_cast<int>(cursor.pos) - static_cast<int>(data_index));
        }
    } else {
        cursor.col_pos += inc;
    }
    // FIXME: do this more optimally. Since we don't what the data contains, we just rebuild entire meta data for now
    if (has_meta_data) rebuild_metadata();
}
void StdStringBuffer::clear() {
    store.clear();
    file_path.clear();
    state_is_pristine = false;
    data_is_pristine = false;
    clear_metadata();
}
void StdStringBuffer::insert(char ch) {
    auto &md_lines = meta_data.line_begins;
    if (cursor.pos == store.capacity() || store.size() >= store.capacity()) { store.reserve(store.capacity() * 2); }
    store.insert(store.begin() + cursor.pos, ch);

    if (ch == '\n') {
        if (has_meta_data) {
            md_lines.insert(md_lines.begin() + cursor.line, cursor.pos);
            std::for_each(md_lines.begin() + cursor.line + 1, md_lines.end(), [](auto &e) { e += 1; });
            for (auto &b :
                 meta_data
                         .bookmarks) {// we only bookkeep line numbers for bookmarks, as the metadata has a separate index for line begins
                if (b.line_number > cursor.line) { b.line_number++; }
            }
        }
        cursor.line++;
        cursor.col_pos = 0;
    } else {
        if (has_meta_data) {
            if (cursor.line + 1 < md_lines.size()) {
                std::for_each(md_lines.begin() + cursor.line + 1, md_lines.end(), [](auto &e) { e += 1; });
            }
        }
        cursor.col_pos++;
    }
    cursor.pos++;
    this->state_is_pristine = false;
    data_is_pristine = false;
}

size_t StdStringBuffer::get_cursor_pos() const { return cursor.pos; }
std::size_t StdStringBuffer::size() const { return store.size(); }

std::unique_ptr<TextData> StdStringBuffer::make_handle() {
    auto new_buffer_id = DataManager::get_instance().get_new_id();
    auto buf = std::make_unique<StdStringBuffer>();
    buf->id = new_buffer_id;
    buf->cursor.buffer_id = buf->id;
    return buf;
}

TextData *StdStringBuffer::make_non_owning() {
    auto new_buffer_id = DataManager::get_instance().get_new_id();
    auto buf = new StdStringBuffer;
    buf->id = new_buffer_id;
    buf->cursor.buffer_id = buf->id;
    buf->has_meta_data = false;
    return buf;
}

BufferCursor &StdStringBuffer::get_cursor() { return cursor; }

void StdStringBuffer::remove(const Movement &m) {
    switch (m.construct) {
        case Char:
            m.dir == CursorDirection::Forward ? remove_ch_forward(m.count) : remove_ch_backward(m.count);
            break;
        case Word:
            m.dir == CursorDirection::Forward ? remove_word_forward(m.count) : remove_word_backward(m.count);
            break;
        case Line:
            // TODO: m.dir == CursorDirection::Forward ? this->remove_line_forward(m.count) : this->remove_line_backward(m.count);
            break;
        default:
            PANIC("Removing other than char, word, line is an error at this point");
    }
    this->state_is_pristine = false;
    data_is_pristine = false;
}

void StdStringBuffer::remove_ch_forward(size_t i) {
    auto line_deleted = false;
    if (cursor.pos + i < store.size()) {
        auto e = cursor.pos + i;
        for (auto index = cursor.pos; index < e && not line_deleted; index++) {
            if (store[index] == '\n') { line_deleted = true; }
        }
        store.erase(cursor.pos, i);
    } else {
        auto sz = store.size();
        for (auto index = cursor.pos; index < sz && not line_deleted; index++) {
            if (store[index] == '\n') { line_deleted = true; }
        }
        store.erase(cursor.pos);
    }

    if (line_deleted) {
        rebuild_metadata();
    } else {
        if (has_meta_data) {
            std::for_each(meta_data.line_begins.begin() + cursor.line, meta_data.line_begins.end(),
                          [i](auto &e) { e -= i; });
        }
    }
}
void StdStringBuffer::remove_ch_backward(size_t i) {
    if ((int) cursor.pos - (int) i >= 0) {
        step_cursor_to(cursor.pos - i);
        store.erase(cursor.pos, i);
        rebuild_metadata();
        this->state_is_pristine = false;
        data_is_pristine = false;
    }
}

void StdStringBuffer::remove_word_forward(size_t count) {
    auto sz = size();
    if (cursor.pos + 1 >= sz) {
        store.erase(cursor.pos);
        return;
    }
    auto new_pos = find_next_delimiter(cursor.pos);
    count--;
    for (; count > 0; --count) { new_pos = find_next_delimiter(new_pos); }
    store.erase(cursor.pos, new_pos - cursor.pos);
}

void StdStringBuffer::remove_word_backward(size_t count) {
    if (cursor.pos - 1 <= 0) {
        remove_ch_backward(1);
        return;
    }
    if (find_prev_delimiter(cursor.pos) == cursor.pos - 1) {
        remove_ch_backward(1);
        return;
    }
    auto new_pos = find_prev_delimiter(cursor.pos);
    count--;
    for (; count > 0; --count) { new_pos = find_prev_delimiter(new_pos); }
    auto distance = cursor.pos - new_pos;
    if (new_pos != 0) {
        remove_ch_backward(distance - 1);
    } else {
        remove_ch_backward(distance);
    }
}

// TODO(simon): MAJOR CLEAN UP NEEDED!

void StdStringBuffer::step_cursor_to(size_t pos) {
    if (pos == 0) {
        cursor.pos = 0;
        cursor.line = 0;
        cursor.col_pos = 0;
        state_is_pristine = false;
        return;
    }
    if (empty() || pos == cursor.pos) return;
    assert(pos <= size() && pos >= 0);
    auto distance = std::abs(AS(pos, int) - cursor.pos);
    if (distance > 30) {
        if (has_meta_data && data_is_pristine) {
            util::println("Using meta data to move cursor");
            if (pos < cursor.pos) {
                auto i = 0;
                auto lineToGoTo = 0;
                for (; i <= cursor.line; i++) {
                    if (pos >= meta_data.line_begins[i] && pos <= meta_data.line_begins[i + 1]) {
                        lineToGoTo = i;
                        break;
                    }
                }
                cursor.line = lineToGoTo;
                cursor.pos = AS(pos, int);
            } else {
                auto i = cursor.line;
                auto last_item = meta_data.line_begins.size() - 1;
                auto lineToGoTo = 0;
                if (meta_data.line_begins[i] == cursor.pos) i++;
                for (; i < last_item; i++) {
                    if (pos >= meta_data.line_begins[i] && pos <= meta_data.line_begins[i + 1]) {
                        lineToGoTo = i;
                        break;
                    }
                }
                if (i == last_item) lineToGoTo = meta_data.line_begins.size() - 1;
                cursor.line = lineToGoTo;
                cursor.pos = AS(pos, int);
            }
        } else {
            util::println("Meta data incomplete, scanning buffer for movement...");
            if (pos > cursor.pos) {
                while (cursor.pos != pos) {
                    if (store[cursor.pos] == '\n') cursor.line++;
                    cursor.pos++;
                }
            } else {
                while (cursor.pos != pos) {
                    cursor.pos--;
                    if (store[cursor.pos] == '\n') { cursor.line--; }
                }
            }
        }
    } else {
        if (pos > cursor.pos) {
            while (cursor.pos != pos) {
                if (store[cursor.pos] == '\n') cursor.line++;
                cursor.pos++;
            }
        } else {
            while (cursor.pos != pos) {
                cursor.pos--;
                if (store[cursor.pos] == '\n') { cursor.line--; }
            }
        }
    }

    auto col_pos_res = cursor.pos - find_line_start(Boundary::Inside, cursor.pos);
    cursor.col_pos = std::max(0, col_pos_res);
    state_is_pristine = false;
}

#ifdef DEBUG
std::string StdStringBuffer::to_std_string() const { return store; }
std::string_view StdStringBuffer::to_string_view() {
    state_is_pristine = true;
    return store;
}
void StdStringBuffer::load_string(std::string &&data) {
    auto line_indices = str::count_newlines(data.data(), data.size());
    this->meta_data = TextMetaData{std::move(line_indices)};
    auto l = std::unique(this->meta_data.line_begins.begin(), this->meta_data.line_begins.end());
    this->meta_data.line_begins.erase(l, meta_data.line_begins.end());
    store = std::move(data);
    state_is_pristine = false;
    data_is_pristine = true;
}

void StdStringBuffer::set_string(std::string &data) {
    auto line_indices = str::count_newlines(data.data(), data.size());
    this->meta_data = TextMetaData{std::move(line_indices)};
    store.reserve(data.size() * 4);
    for (auto c : data) store.push_back(c);
    state_is_pristine = false;
    data_is_pristine = true;
}

#endif

size_t StdStringBuffer::lines_count() const {
    return map_or(count_elements(this->store, '\n'), 0, [](auto &&el) { return el.size(); });
}
int StdStringBuffer::find_next_delimiter(int i) {
    auto sz = size();
    if (is_delimiter(store[i])) {
        // we are already standing on whitespace... scan until we are no longer on whitespace
        while (i < sz) {
            if (not is_delimiter(store[i])) return i;
            i++;
        }
        return i;
    } else {
        if (i + 1 < sz) {
            i++;
            for (; i < sz; i++) {
                if (is_delimiter(store[i])) { return i; }
            }
            return i;
        } else {
            return sz;
        }
    }
}

int StdStringBuffer::find_prev_delimiter(int i) {
    if (i - 1 > 0) {
        --i;
        for (; i > 0; --i) {
            if (is_delimiter(store[i])) { return i; }
        }
        return i;
    } else {
        return 0;
    }
}
void StdStringBuffer::step_to_line_end(Boundary boundary) {
    int sz = size();
    bool found = false;
    auto newpos = cursor.pos;
    for (auto i = cursor.pos; i < sz && !found; i++) {
        if (store[i] == '\n') {
            switch (boundary) {
                case Inside:
                    found = true;
                    newpos = i - 1;
                    break;
                case Outside:
                    found = true;
                    newpos = i;
                    break;
            }
        }
    }
    step_cursor_to(std::min(newpos, sz));
}

void StdStringBuffer::step_to_line_begin(Boundary boundary) {
    for (auto i = cursor.pos - 1; i >= 0; --i) {
        if (store[i] == '\n') {
            switch (boundary) {
                case Inside:
                    step_cursor_to(i + 1);
                    return;
                case Outside:
                    step_cursor_to(i);
                    return;
            }
        }
    }
    step_cursor_to(0);
}

void StdStringBuffer::rebuild_metadata() {
    if (has_meta_data && (data_is_pristine == false) && info == BufferTypeInfo::EditBuffer) {
        if (has_meta_data) {
            auto line_indices = str::count_newlines(store.data(), store.size());
            this->meta_data.line_begins = std::move(line_indices);
        }
        data_is_pristine = true;
    } else if (info == BufferTypeInfo::Modal) {
        if (has_meta_data) {
            auto line_indices = str::count_newlines(store.data(), store.size());
            this->meta_data.line_begins = std::move(line_indices);
        }
    }
    state_is_pristine = false;
    data_is_pristine = true;
}

void StdStringBuffer::set_mark_from_cursor(int length) {
    auto tmp = cursor;
    step_cursor_to(cursor.pos + length);
    mark = cursor;
    cursor = tmp;
    mark_set = true;
}
void StdStringBuffer::clear_marks() {
    mark_set = false;
    mark.reset();
}

void StdStringBuffer::set_mark_at_cursor() {
    mark = cursor;
    mark_set = true;
}

std::pair<BufferCursor, BufferCursor> StdStringBuffer::get_cursor_rect() const {
    if (mark_set) {
        if (mark.pos < cursor.pos) {
            return std::make_pair(mark.clone(), cursor.clone());
        } else {
            return std::make_pair(cursor.clone(), mark.clone());
        }
    } else {
        return std::make_pair(cursor.clone(), cursor.clone());
    }
}
std::string_view StdStringBuffer::copy_range(std::pair<BufferCursor, BufferCursor> selected_range) {
    auto &[b, e] = selected_range;
    return {store.data() + b.pos, AS(e.pos - b.pos, size_t)};
}
void StdStringBuffer::insert_str_owned(const std::string &ref_data) {
    for (auto ch : ref_data) { insert(ch); }
    if (has_meta_data) rebuild_metadata();
}

StdStringBuffer::~StdStringBuffer() {
    if (DataManager::get_instance().is_managed(id)) {
        DataManager::get_instance().print_all_managed();
        // TODO: this is just here, so we always can be sure that during the life time of the program, the buffers don't accidentally destroy themselves
        //  this will be removed.
        PANIC("ERROR. BUFFER IS TRYING TO DESTROY ITSELF. THAT IS HANDLED BY DATAMANAGER. ID: {}", id);
    }
}

void StdStringBuffer::goto_next(std::string search) {
    auto pos = store.find(search, cursor.pos + 1);
    auto oldpos = cursor.pos;
    if (pos != std::string::npos) {
        cached_search = search;
        util::println("found '{}' at {}: [{}]", search, pos, store.substr(pos, search.size()));
        step_cursor_to(pos);
        util::println("Move {} -> {}", oldpos, cursor.pos);
    }
}
size_t StdStringBuffer::capacity() const { return store.capacity(); }

void StdStringBuffer::set_bookmark() {
    auto &bm = meta_data.bookmarks;
    if (std::ranges::any_of(bm, [l = cursor.line](auto &b) { return b.line_number == l; })) { return; }
    auto line_begin = find_line_start(Boundary::Inside, cursor.pos);
    auto line_end = find_line_end(cursor.pos);

    // TODO: i can't be bothered to fix this right now. Due to how text data works, find_line_start and begin
    //  returns cursor positions on different boundary types. find_line_end finds returns boundary_inside,
    //  find_line_start finds boundary_outside, meaning it finds the previous soonest '\n and adds +1 to that
    //  while find_line_end finds the next '\n' and subtracts 1 from that, since when we want a line,
    //  we want the actual logical representation of that data (the actual characters, not the computer data, i.e. newlines,
    //  tabs, etc)
    if(line_begin > line_end) {
        return;
    }

    std::string_view v{store.c_str() + line_begin, static_cast<std::size_t>(line_end - line_begin)};
    auto it = std::ranges::find_if(v, [](auto e) { return !std::isspace(e); });

    v.remove_prefix(std::distance(v.begin(), it));
    if (not v.empty()) {
        std::string line_contents{v};
        bm.emplace_back(cursor.line, std::move(line_contents));
        // TODO: this really is wasting CPU time. if we keep it sorted, we should find
        std::sort(bm.begin(), bm.end(), [](auto &ba, auto &bb) { return ba.line_number < bb.line_number; });
        util::println("Set bookmark at {}: '{}'", cursor.line, meta_data.bookmarks.back().line_contents);
    }
}
