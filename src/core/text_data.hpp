//
// Created by cx on 2020-11-18.
//

#pragma once
#include "core.hpp"
#include <cassert>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <utils/strops.hpp>

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

/// Abstract class. This is used here, so we can swap out the horrible std::string
/// as the underlying data structure, once we got an interesting program up and running.

class TextDataIterator;

/// UNSAFE: Checks character +1 beyond where ch points to.

enum DelimiterSize { None, One, Two };

static inline bool is_delimiter(char ch) {
    return ((ch == ' ') || (ch == '\n') || (ch == '-') || (ch == '>') || ch == '.');
}

namespace fs = std::filesystem;

struct TextMetaData {
    std::vector<int> line_begins{};
    std::string buf_name{};
};

enum class BufferTypeInfo { CommandInput, StatusBar, EditBuffer };

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
    BufferCursor cursor{};

    TextData() = default;
    virtual ~TextData() = default;
    virtual void insert(char ch) = 0;
    virtual void insert_str(const std::string_view &data) = 0;
    virtual void insert_str_owned(const std::string& ref_data) = 0;
    virtual void clear() = 0;

    virtual void remove(const Movement &m) = 0;
    virtual void erase() = 0;

    virtual std::size_t get_cursor_pos() const = 0;

    virtual std::size_t size() const = 0;
    virtual bool empty() const { return size() == 0; };
    virtual std::size_t lines_count() const = 0;

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
    virtual bool is_pristine() const { return state_is_pristine; }
#ifdef DEBUG
    virtual std::string to_std_string() const = 0;
    virtual std::string_view to_string_view() = 0;
    virtual void load_string(std::string &&data) = 0;
    virtual void set_string(std::string& data) = 0;

    // Brute force rebuild meta data. Not really optimized but will work for now

    void print_cursor_info() const {
        util::println("Buffer cursor [i:{}, ln: {}, col: {}]", cursor.pos, cursor.line, cursor.col_pos);
    }
    void print_line_meta_data() const {
        util::println("meta data - line begin: {}", meta_data.line_begins[cursor.line]);
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

class StdStringBuffer : public TextData {
public:
    explicit StdStringBuffer() : TextData(), store{} {}
    ~StdStringBuffer() override = default;

    /// EDITING
    void insert(char ch) override;
    void insert_str(const std::string_view &data) override;
    void insert_str_owned(const std::string &ref_data) override;
    void clear() override;

    void remove(const Movement &m) override;
    void erase() override;

    /// CURSOR OPS
    size_t get_cursor_pos() const override;
    std::size_t size() const override;
    void move_cursor(Movement m) override;
    void step_cursor_to(size_t pos) override;
    BufferCursor &get_cursor() override;

    /// INIT CALLS
    static std::unique_ptr<TextData> make_handle();

    /// SEARCH OPS
    size_t lines_count() const override;

    void step_to_line_end(Boundary boundary) override;
    void step_to_line_begin(Boundary boundary) override;

#ifdef DEBUG
    std::string to_std_string() const override;
    std::string_view to_string_view() override;
    void load_string(std::string &&data) override;
    void set_string(std::string& data) override;
#endif

    void rebuild_metadata() override;

    // void set_mark(int pos) override;
    // void set_mark_range(int begin, int length) override;
    void set_mark_at_cursor() override;
    void set_mark_from_cursor(int length) override;
    void clear_marks() override;

    std::pair<BufferCursor, BufferCursor> get_cursor_rect() const override;
    std::string_view copy_range(std::pair<BufferCursor, BufferCursor> selected_range) override;

private:
    void char_move_forward(std::size_t count) override;
    void char_move_backward(std::size_t count) override;
    void word_move_forward(std::size_t count) override;
    void word_move_backward(std::size_t count) override;
    void line_move_forward(std::size_t count) override;
    void line_move_backward(std::size_t count) override;
    void remove_ch_forward(size_t i);
    void remove_ch_backward(size_t i);
    void remove_word_forward(size_t i);
    void remove_word_backward(size_t i);
    void remove_line_forward(size_t i);
    void remove_line_backward(size_t i);
    std::string store;

    // This variable is set/checked every time a view wants to display. So once
    // vertex data is generated, this is set to true, until any text is inserted to the buffer
    // at which point it is set to false. This way we don't have to reconstruct vertex data every render cycle.
    int find_line_begin(int i);
    int find_line_end(int i);
    int find_next_delimiter(int i);
    int find_prev_delimiter(int i);
    int find_line_start(int i);
};