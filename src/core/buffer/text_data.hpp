//
// Created by cx on 2020-11-18.
//

#pragma once
#include "bookmark.hpp"
#include "file_context.hpp"
#include <cassert>
#include <core/core.hpp>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <utils/strops.hpp>

namespace ui {
    struct View;
}

class TextData;
enum CursorDirection { Forward, Back };
enum TextRep { Char, Word, Line, Block, File };
enum Boundary { Inside, Outside };

struct Movement {
    Movement(size_t count, TextRep elem_type, CursorDirection dir) : count(count), construct(elem_type), dir(dir) {}
    size_t count;
    TextRep construct;
    CursorDirection dir;
    Boundary boundary;

    static Movement Char(size_t count, CursorDirection dir) { return Movement{count, TextRep::Char, dir}; }
    static Movement Word(size_t count, CursorDirection dir) { return Movement{count, TextRep::Word, dir}; }
    static Movement Line(size_t count, CursorDirection dir) { return Movement{count, TextRep::Line, dir}; }
    /* TODO: implement these when it's reasonable to do so
    static Movement Block(size_t count, CursorDirection dir) { return Movement{count, TextRep::Block, dir}; }
    static Movement File(CursorDirection dir) { return Movement{0, TextRep::File, dir}; }
     */
};

/// UNSAFE: Checks character +1 beyond where ch points to.

enum DelimiterSize { None, One, Two };

static inline bool is_delimiter_2(char ch) {
    return ((ch == ' ') || (ch == '\n') || (ch == '-') || (ch == '>') || (ch == '.') || ch == '(' || ch == ')');
}

static inline bool is_delimiter(char ch) {
    return !std::isalnum(ch) && ch != '_';
}

namespace fs = std::filesystem;

struct TextMetaData {
    std::vector<int> line_begins{0};
    std::string buf_name{};
    std::vector<Bookmark> bookmarks{};

    std::optional<int> get(size_t line_number) const;
};

enum class BufferTypeInfo { CommandInput, StatusBar, EditBuffer, Modal };

struct BufferCursor {
    int pos{0};
    int line{0};
    int col_pos{0};
    int buffer_id{0};
    void reset();
    void invalidate();
    BufferCursor clone() const;
};

class TextData {
public:
    BufferTypeInfo info;
    int id{0};
    std::string name;
    BufferCursor cursor{};

    TextData() = default;
    virtual ~TextData() = default;
    virtual void insert(char ch) = 0;
    virtual void insert_str(const std::string_view &data) = 0;
    virtual void insert_str_owned(const std::string& ref_data) = 0;
    virtual void clear() = 0;

    virtual void remove(const Movement &m) = 0;
    virtual void erase() = 0;

    [[nodiscard]] virtual std::size_t get_cursor_pos() const = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual std::size_t capacity() const = 0;
    [[nodiscard]] virtual bool empty() const { return size() == 0; };
    [[nodiscard]] virtual std::size_t lines_count() const = 0;

    virtual BufferCursor &get_cursor() = 0;

    virtual void step_cursor_to(size_t pos) = 0;
    virtual void move_cursor(Movement movement) = 0;

    virtual void step_to_line_end(Boundary boundary) = 0;
    virtual void step_to_line_begin(Boundary boundary) = 0;


    virtual void set_file(fs::path p);
    virtual bool exist_on_disk() const;
    virtual std::string fileName();

    virtual void rebuild_metadata() = 0;
    virtual void clear_metadata();
    virtual bool has_metadata() { return has_meta_data && not meta_data.line_begins.empty(); }
    [[nodiscard]] virtual bool is_pristine() const { return state_is_pristine; }
    virtual void set_bookmark() = 0;
#ifdef DEBUG
    [[nodiscard]] virtual std::string to_std_string() const = 0;
    virtual std::string_view to_string_view() = 0;
    virtual void load_string(std::string &&data) = 0;
    virtual void set_string(std::string& data) = 0;

    // Brute force rebuild meta data. Not really optimized but will work for now

    void print_cursor_info() const {
        util::println("Buffer cursor [i:{}, ln: {}, col: {}]", cursor.pos, cursor.line, cursor.col_pos);
    }
    void print_line_meta_data() const {
        util::println("meta data - line begin: {}", meta_data.line_begins[static_cast<unsigned long>(cursor.line)]);
        util::println("Buffer meta data up to date: {}", data_is_pristine);
    }
#endif

    // virtual void set_mark(int pos) = 0;
    // virtual void set_mark_range(int begin, int length) = 0;
    virtual void set_mark_at_cursor() = 0;
    virtual void set_mark_from_cursor(int length) = 0;
    virtual void clear_marks() = 0;
    virtual auto get_cursor_rect() const -> std::pair<BufferCursor, BufferCursor> = 0;

    BufferCursor mark;
    bool mark_set = false;
    bool has_meta_data{false};
    fs::path file_path;
    TextMetaData meta_data;
    virtual std::string_view copy_range(std::pair<BufferCursor, BufferCursor> selected_range) = 0;
    virtual void goto_next(std::string search) = 0;

    void set_name(std::string buffer_name);
    virtual FileContext file_context() const;

protected:
    /**
     * Changed this to "state_is_pristine" to also communicate that, not only the text data
     * is represented by this variable, but the entire state of this text buffer, which includes the
     * cursor. This is so we can know from the outside if the cursor has moved, and thus needs to be redrawn on the
     * frontend
     */
    bool state_is_pristine{false};
    bool data_is_pristine{false};

private:
    virtual void char_move_forward(std::size_t count) = 0;
    virtual void char_move_backward(std::size_t count) = 0;
    virtual void word_move_forward(std::size_t count) = 0;
    virtual void word_move_backward(std::size_t count) = 0;
    virtual void line_move_forward(std::size_t count) = 0;
    virtual void line_move_backward(std::size_t count) = 0;
};