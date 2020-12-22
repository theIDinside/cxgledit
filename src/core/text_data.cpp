//
// Created by cx on 2020-11-18.
//

#include "text_data.hpp"
#include "strops.hpp"

void TextData::BufferCursor::reset() {
    line = 0;
    col_pos = 0;
    pos = 0;
}

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
}
void StdStringBuffer::char_move_forward(std::size_t count) {
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
}
void StdStringBuffer::char_move_backward(std::size_t count) {
    if (static_cast<int>(cursor.pos) - static_cast<int>(count) <= 0) {
        fmt::print("RESETTIGN CURSOR\n");
        cursor.reset();
    } else {
        auto next_pos = cursor.pos - count;
        for (auto index = cursor.pos; index > next_pos; index--) {
            if (store[index-1] == '\n') { cursor.line--; }
            cursor.pos--;
        }
        auto i = cursor.pos;
        if(store[i] == '\n') i--;
        bool found = false;
        for(; i > 0 && !found; i--) {
            if(store[i] == '\n') {
                cursor.col_pos = cursor.pos - i - 1;
                found = true;
            }
        }
        if(!found) cursor.col_pos = cursor.pos;
    }
}

void StdStringBuffer::word_move_forward(std::size_t count) {
    auto e = store.end();
    auto b = store.begin() + cursor.pos;
    if (*b == ' ') {
        b++;
        cursor.pos++;
        cursor.col_pos++;
    } else if (*b == '\n') {
        b++;
        cursor.pos++;
        cursor.line++;
        cursor.col_pos = 0;
    }
    while (b < e && count > 0) {
        if (*b == ' ' || *b == '\n' || *b == '-' || *b == '.') {
            auto cit = b;
            cit++;
            auto cols = cursor.col_pos;
            auto lines = 0;
            auto positions = 0;
            bool consecutive = false;
            while (cit < e && std::isspace(*cit)) {
                if (*cit == ' ') {
                    cols++;
                    positions++;
                    consecutive = true;
                }
                if (*cit == '\n') {
                    cols = 0;
                    positions++;
                    lines++;
                    consecutive = true;
                }
                cit++;
            }
            if (consecutive) {
                cursor.pos += positions;
                cursor.col_pos = cols;
                cursor.line += lines;
            } else {
                if (auto deref_token_it = b; *b == '-' && ++deref_token_it != e) {
                    if (*deref_token_it == '>') {
                        cursor.pos++;
                        cursor.col_pos++;
                        b = deref_token_it;
                    }
                }
                --count;
            }
        }
        cursor.pos++;
        cursor.col_pos++;
        b++;
    }
}
void StdStringBuffer::word_move_backward(std::size_t count) {
    auto pos_from_end = store.size() - cursor.pos;
    auto e = store.rend();
    auto b = store.rbegin() + pos_from_end;
    if (*b == ' ') {
        b++;
        cursor.pos--;
    } else if (*b == '\n') {
        b++;
        cursor.pos--;
        cursor.line--;
    }
    while (b < e && count > 0) {
        if (*b == ' ' || *b == '\n' || *b == '-' || *b == '>' || *b == '.') {
            auto cit = b;
            cit++;
            auto lines = 0;
            auto positions = 0;
            bool consecutive = false;
            while (cit < e && std::isspace(*cit)) {
                if (*cit == ' ') {
                    positions--;
                    consecutive = true;
                }
                if (*cit == '\n') {
                    positions--;
                    lines--;
                    consecutive = true;
                }
                cit++;
            }
            if (consecutive) {
                cursor.pos += positions;
                cursor.line += lines;
            } else {
                if (auto deref_token_it = b; *b == '>' && ++deref_token_it != e) {
                    if (*deref_token_it == '-') {
                        cursor.pos--;
                        cursor.col_pos--;
                        b = deref_token_it;
                    }
                }
                --count;
            }
        }
        cursor.pos--;
        b++;
    }
    auto prior = store.rfind('\n', cursor.pos);
    if (prior == std::string::npos) {
        cursor.col_pos = cursor.pos;
    } else {
        cursor.col_pos = cursor.pos - prior;
    }
}

void StdStringBuffer::line_move_forward(std::size_t count) {
    auto curr_column = cursor.col_pos;
    auto pos = cursor.pos;
    auto sz = size();
    for(; pos < sz && count > 0; pos++) {
        if(store[pos] == '\n') count--;
    }
    for(; pos < sz && store[pos] != '\n' && curr_column > 0; pos++, --curr_column);
    step_cursor_to(pos);
}
void StdStringBuffer::line_move_backward(std::size_t count) {
    int curr_column = cursor.col_pos;
    auto pos = cursor.pos;
    auto line_end = 0;
    int cnt = count;
    for(; pos > 0 && (cnt + 1) > 0; pos--) {

        if(store[pos] == '\n') {
            cnt--;
            if(cnt == 0) line_end = pos;
        }
    }
    auto sz = size();
    for(; pos < sz && pos <= line_end && curr_column > 0; pos++, --curr_column);
    step_cursor_to(pos);
}

void StdStringBuffer::remove() {
    assert(cursor.pos != 0 && "you have fucked up the index... out of bounds");
    cursor.pos--;
    cursor.col_pos--;
    if (store[cursor.pos] == '\n') {
        cursor.line--;
        auto i = store.rfind('\n', cursor.pos);
        if (i == std::string::npos) {
            cursor.col_pos = cursor.pos;
        } else {
            cursor.col_pos = cursor.pos - i;
        }
    }
    store.erase(cursor.pos, 1);
}
void StdStringBuffer::erase() {
    store.erase(cursor.pos, 1);
}
void StdStringBuffer::insert(const std::string_view &data) {
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
}
void StdStringBuffer::clear() {
    store.clear();
    cursor.pos = 0;
    cursor.col_pos = 0;
    cursor.line = 0;
}
void StdStringBuffer::insert(char ch) {
    if (cursor.pos == store.capacity() || store.size() == store.capacity()) {
        fmt::print("buffer resized from {};{} to", store.length(), store.capacity());
        store.reserve(store.capacity() * 2);
        fmt::print(" {};{}\n", store.length(), store.capacity());
    }
    store.insert(store.begin() + cursor.pos, ch);
    cursor.pos++;
    cursor.col_pos++;
    if (ch == '\n') {
        cursor.line++;
        cursor.col_pos = 0;
    }
}
void StdStringBuffer::set_cursor(std::size_t pos) { cursor.pos = pos; }
size_t StdStringBuffer::get_cursor_pos() const { return cursor.pos; }
std::size_t StdStringBuffer::size() const { return store.size(); }

std::unique_ptr<TextData> StdStringBuffer::make_handle() { return std::make_unique<StdStringBuffer>(); }
std::unique_ptr<TextData> StdStringBuffer::make_handle(std::string data) {
    auto buf = std::unique_ptr<StdStringBuffer>();

    return buf;
}

TextData *StdStringBuffer::make_buffer() { return new StdStringBuffer; }
size_t StdStringBuffer::rfind_from(size_t pos, char ch) const { return store.rfind(ch, pos); }
std::optional<size_t> StdStringBuffer::find_from(size_t pos, char ch) const {
    auto res = store.find(ch, pos);
    if(res == std::string::npos) return {};
    else return res;
}
size_t StdStringBuffer::rfind_from(size_t pos, std::string_view v) const { return store.rfind(v, pos); }
size_t StdStringBuffer::find_from(size_t pos, std::string_view v) const { return store.find(v, pos); }
void StdStringBuffer::set_line(std::size_t pos) { cursor.line = pos; }

char &StdStringBuffer::get_at(std::size_t pos) {
    // fmt::print("pos < store.size(): {} < {}\n", pos, store.size());
    assert(pos < store.size());
    return store[pos];
}

std::optional<char> StdStringBuffer::get_value_at_safe(std::size_t pos) {
    if (pos == store.size()) return {};
    else {
        return store[pos];
    }
}
size_t StdStringBuffer::npos() const { return std::string::npos; }
TextData::BufferCursor &StdStringBuffer::get_cursor() { return cursor; }


void StdStringBuffer::remove(const Movement &m) {
    switch (m.construct) {
        case Char:
            m.dir == CursorDirection::Forward ? this->remove_ch_forward(m.count) : this->remove_ch_backward(m.count);
            break;
        case Word:
            // TODO: m.dir == CursorDirection::Forward ? this->remove_word_forward(m.count) : this->remove_word_backward(m.count);
            break;
        case Line:
            // TODO: m.dir == CursorDirection::Forward ? this->remove_line_forward(m.count) : this->remove_line_backward(m.count);
            break;
        default:
            PANIC("Removing other than char, word, line is an error at this point");
    }
}
void StdStringBuffer::remove_ch_forward(size_t i) {
    if (cursor.pos + i < store.size()) {
        auto e = cursor.pos + i;
        auto lines_deleted = 0;
        for (auto index = cursor.pos; index <= e; index++) {
            if (store[index] == '\n') lines_deleted++;
        }
        store.erase(cursor.pos, i);
        cursor.line -= lines_deleted;
    } else {
        auto lines_deleted = 0;
        auto sz = store.size();
        for (auto index = cursor.pos; index < sz; index++) {
            if (store[index] == '\n') lines_deleted++;
        }
        store.erase(cursor.pos);
        cursor.line -= lines_deleted;
    }
}
void StdStringBuffer::remove_ch_backward(size_t i) {
    if ((int) cursor.pos - (int) i >= 0) { store.erase(cursor.pos - i, i); }
}
// TODO(simon): MAJOR CLEAN UP NEEDED!
std::optional<size_t> StdStringBuffer::get_item_pos_from(const Movement &m) {
    if(empty()) return {};
    assert(m.count != 0);
    auto amount = m.count;
    auto result_pos = cursor.pos;
    switch (m.construct) {
        case Char:
            break;
        case Word:
            if (m.dir == CursorDirection::Forward) {
                if(cursor.pos == size()) return {};
                auto it = begin();
                std::advance(it, cursor.pos);
                if (it == end()) { return size(); }
                if (is_delimiter(*it)) {
                    ++it;
                    amount--;
                    if (amount == 0) {
                        fmt::print("Returning {}\n", it.get_buffer_index());
                        return it.get_buffer_index();
                    }
                }
                for (; it < end() && amount > 0; it++) {
                    if (is_delimiter(*it)) {
                        if (*it == '-') {
                            auto n = it;
                            n++;
                            if (*n == '>') {
                                result_pos++;
                                ++it;
                            }
                        }
                        amount--;
                        if (amount == 0) return it.get_buffer_index();
                    }
                }
                return {};
            } else {
                if(cursor.pos == 0) return {};
                int i = std::max(0, (int) cursor.pos - 1);
                for (; i > 0 && amount > 0; --i) {
                    const auto &c = store[i];
                    if (is_delimiter(c)) {
                        if (c == '>') {
                            auto look_ahead = i - 1;
                            if (store[look_ahead] == '-') {
                                result_pos--;
                                --i;
                            }
                        }
                        amount--;
                        if (amount == 0) return i;
                    }
                }
                return {};
            }
        case Line:
            break;
        case Block:
            break;
        case File:
            break;
    }
}
char *StdStringBuffer::get_at_ptr(std::size_t pos) { return store.data() + pos; }
void StdStringBuffer::step_cursor_to(size_t pos) {
    if (empty() || pos == cursor.pos) return;
    fmt::print("step to {} from {}\n", pos, cursor.pos);
    assert(pos <= size());
    if (pos > cursor.pos) {// moving cursor forward
        for (; cursor.pos < pos; cursor.pos++) {
            if (get_value_at_safe(cursor.pos).value_or('\0') == '\n') {
                cursor.line++;
                cursor.col_pos = 0;
            } else {
                cursor.col_pos++;
            }
        }
    } else {
        if (cursor.pos == size()) {
            cursor.pos--;
            cursor.col_pos--;
        }
        for (; cursor.pos > pos; cursor.pos--) {
            if (get_at(cursor.pos) == '\n') {
                cursor.line--;
                auto prior_line_end = store.rfind('\n', cursor.pos - 1);
                cursor.col_pos = cursor.pos - prior_line_end;
            } else {
                cursor.col_pos--;
            }
        }
    }
}

#ifdef DEBUG
std::string StdStringBuffer::to_std_string() const { return store; }
std::string_view StdStringBuffer::to_string_view() const { return store; }
void StdStringBuffer::load_string(std::string &&data) {
    m_lines = str::count_newlines(data.data(), data.size());
    util::println("Read {} lines of text", m_lines);
    store = std::move(data);
}

#endif