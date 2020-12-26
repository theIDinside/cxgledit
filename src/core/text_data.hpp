//
// Created by cx on 2020-11-18.
//

#pragma once
#include "core.hpp"
#include <cassert>
#include <memory>
#include <string>
#include <string_view>
#include <utils/strops.hpp>
#include <utility>
#include <filesystem>


class TextData;
enum CursorDirection { Forward, Back };
enum TextRep { Char, Word, Line, Block, File };

struct Movement {
    Movement(size_t count, TextRep elem_type, CursorDirection dir) : count(count), construct(elem_type), dir(dir) {}
    size_t count;
    TextRep construct;
    CursorDirection dir;

    static Movement Char(size_t count, CursorDirection dir) { return Movement{count, TextRep::Char, dir}; }
    static Movement Word(size_t count, CursorDirection dir) { return Movement{count, TextRep::Word, dir}; }
    static Movement Line(size_t count, CursorDirection dir) { return Movement{count, TextRep::Line, dir}; }
};

/// Abstract class. This is used here, so we can swap out the horrible std::string
/// as the underlying data structure, once we got an interesting program up and running.

class TextDataIterator;


/// UNSAFE: Checks character +1 beyond where ch points to.

enum DelimiterSize { None, One, Two };

static inline bool is_delimiter(char ch) {
    if (ch == ' ') {
        return true;
    } else if (ch == '\n') {
        return true;
    } else if (ch == '-') {
        return true;
    } else if (ch == '>') {
        return true;
    } else if(ch == '.') {
        return true;
    }
    return false;
}

namespace fs = std::filesystem;

class TextData {
public:
    int id{0};
    struct BufferCursor {
        std::size_t pos{0};
        std::size_t line{0};
        std::size_t col_pos{0};
        void reset();
    } cursor;

    class TextDataIterator {
    public:
        using self = TextDataIterator;
        using reference = char &;
        using value_type = char;
        using pointer = char *;
        using difference_type = size_t;
        using iterator_category = std::input_iterator_tag;

        explicit TextDataIterator(TextData *buf, size_t index = 0) : ptr_buf(buf), m_index(index) {}

        TextDataIterator(const TextDataIterator &) = default;
        /// post increment
        self operator++() {
            auto it = *this;
            m_index++;
            return it;
        }
        /// pre-increment
        self operator++(int) {
            this->m_index++;
            return *this;
        }

        self operator+(std::size_t i) {
            auto it = *this;
            it.m_index += i;
            return it;
        }

        value_type operator*() const { return ptr_buf->get_at(m_index); }
        reference operator*() { return ptr_buf->get_at(m_index); }

        size_t get_buffer_index() const { return m_index; }

        SPACE_SHIP(TextDataIterator, m_index)
    private:
        TextData *ptr_buf;
        size_t m_index;
    };
    class RTextDataIterator {
    public:
        using self = RTextDataIterator;
        using reference = char &;
        using value_type = char;
        using pointer = char *;
        using difference_type = size_t;
        using iterator_category = std::input_iterator_tag;

        explicit RTextDataIterator(TextData *buf, int index, size_t it_count = 1)
                : ptr_buf(buf), m_index(index), iterated_count(it_count) {
            assert(index != buf->size());
        }

        RTextDataIterator(const RTextDataIterator &) = default;
        self operator++() {
            // fmt::print("?? {}/{}\n", m_index, iterated_count);
            auto it = *this;
            iterated_count++;
            m_index--;
            return it;
        }
        self& operator++(int) {
            this->iterated_count++;
            this->m_index--;
            return *this;
        }

        self& operator+(int i) {
            m_index -= i;
            iterated_count += i;
            return *this;
        }

        value_type operator*() const {
            assert(m_index >= 0);
            return ptr_buf->get_at(m_index);
        }
        reference operator*() {
            assert(m_index >= 0);
            return ptr_buf->get_at(m_index);
        }

        size_t get_buffer_index() const { return m_index; }

        SPACE_SHIP(RTextDataIterator, iterated_count)
    private:
        TextData *ptr_buf;
        int m_index;
        size_t iterated_count;
    };

    TextData() = default;
    virtual ~TextData() = default;
    virtual void insert(char ch) = 0;
    virtual void insert(const std::string_view &data) = 0;
    virtual void clear() = 0;
    virtual void remove() = 0;
    virtual void remove(const Movement &m) = 0;
    virtual void erase() = 0;
    virtual std::optional<size_t> get_item_pos_from(const Movement &m) = 0;
    virtual void set_cursor(std::size_t pos) = 0;
    virtual void set_line(std::size_t pos) = 0;
    virtual char &get_at(std::size_t pos) = 0;
    virtual std::optional<char> get_value_at_safe(std::size_t pos) = 0;
    virtual char *get_at_ptr(std::size_t pos) = 0;
    [[nodiscard]] virtual std::size_t get_cursor_pos() const = 0;

    [[nodiscard]] virtual size_t rfind_from(size_t pos, char ch) const = 0;
    [[nodiscard]] virtual std::optional<size_t> find_from(size_t pos, char ch) const = 0;
    [[nodiscard]] virtual size_t rfind_from(size_t pos, std::string_view v) const = 0;
    [[nodiscard]] virtual size_t find_from(size_t pos, std::string_view v) const = 0;
    [[nodiscard]] virtual size_t npos() const = 0;
    [[nodiscard]] virtual std::size_t size() const = 0;
    [[nodiscard]] virtual bool empty() const { return size() == 0; };
    virtual std::size_t lines_count() const = 0;

    virtual BufferCursor &get_cursor() = 0;

    virtual void step_cursor_to(size_t pos) = 0;
    virtual void move_cursor(Movement movement) = 0;


    virtual TextDataIterator begin() { return TextDataIterator{this}; }
    virtual TextDataIterator end() { return TextDataIterator{this, size()}; }

    virtual RTextDataIterator rbegin() { return RTextDataIterator{this, (int)size()-1, 0}; }
    virtual RTextDataIterator rend() { return RTextDataIterator{this, 0, size()}; }

#ifdef DEBUG
    [[nodiscard]] virtual std::string to_std_string() const = 0;
    [[nodiscard]] virtual std::string_view to_string_view() = 0;
    virtual void load_string(std::string &&data) = 0;
#endif

    virtual bool is_pristine() const {
        return display_pristine;
    }

protected:
    std::size_t m_lines;
    bool display_pristine{false};
private:
    virtual void char_move_forward(std::size_t count) = 0;
    virtual void char_move_backward(std::size_t count) = 0;
    virtual void word_move_forward(std::size_t count) = 0;
    virtual void word_move_backward(std::size_t count) = 0;
    virtual void line_move_forward(std::size_t count) = 0;
    virtual void line_move_backward(std::size_t count) = 0;
    fs::path file_name;
};

class StdStringBuffer : public TextData {
public:
    explicit StdStringBuffer() : TextData(), store{} {}
    ~StdStringBuffer() override = default;

    /// EDITING
    void insert(char ch) override;
    void insert(const std::string_view &data) override;
    void clear() override;
    void remove() override;
    void remove(const Movement &m) override;
    void erase() override;

    /// CURSOR OPS
    std::optional<size_t> get_item_pos_from(const Movement &m) override;
    void set_cursor(std::size_t pos) override;
    [[nodiscard]] size_t get_cursor_pos() const override;
    [[nodiscard]] std::size_t size() const override;
    void move_cursor(Movement m) override;
    void step_cursor_to(size_t pos) override;
    void set_line(std::size_t pos) override;
    BufferCursor &get_cursor() override;


    /// INIT CALLS
    static std::unique_ptr<TextData> make_handle();
    static std::unique_ptr<TextData> make_handle(std::string data);
    static TextData *make_buffer();

    /// SEARCH OPS
    [[nodiscard]] size_t rfind_from(size_t pos, char ch) const override;
    [[nodiscard]] std::optional<size_t> find_from(size_t pos, char ch) const override;
    [[nodiscard]] size_t rfind_from(size_t pos, std::string_view v) const override;
    [[nodiscard]] size_t find_from(size_t pos, std::string_view v) const override;
    size_t lines_count() const override;

    char &get_at(std::size_t pos) override;
    std::optional<char> get_value_at_safe(std::size_t pos) override;
    char *get_at_ptr(std::size_t pos) override;
    [[nodiscard]] size_t npos() const override;

#ifdef DEBUG
    [[nodiscard]] std::string to_std_string() const override;
    [[nodiscard]] std::string_view to_string_view() override;
    void load_string(std::string &&data) override;
#endif

private:
    void char_move_forward(std::size_t count) override;
    void char_move_backward(std::size_t count) override;
    void word_move_forward(std::size_t count) override;
    void word_move_backward(std::size_t count) override;
    void line_move_forward(std::size_t count) override;
    void line_move_backward(std::size_t count) override;
    void remove_ch_forward(size_t i);
    void remove_ch_backward(size_t i);
    std::string store;
    // This variable is set/checked every time a view wants to display. So once
    // vertex data is generated, this is set to true, until any text is inserted to the buffer
    // at which point it is set to false. This way we don't have to reconstruct vertex data every render cycle.
};