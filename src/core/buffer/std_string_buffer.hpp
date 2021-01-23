//
// Created by 46769 on 2021-01-23.
//

#pragma once
#include "text_data.hpp"

class StdStringBuffer : public TextData {
public:
    explicit StdStringBuffer() : TextData(), store{} {}
    ~StdStringBuffer() override;

    void register_view_callback(ui::View *view) override;

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
    size_t capacity() const override;
    void move_cursor(Movement m) override;
    void step_cursor_to(size_t pos) override;
    BufferCursor &get_cursor() override;

    /// INIT CALLS
    static std::unique_ptr<TextData> make_handle();
    static TextData*make_non_owning();

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

    FileContext file_context() const override;

    std::pair<BufferCursor, BufferCursor> get_cursor_rect() const override;
    std::string_view copy_range(std::pair<BufferCursor, BufferCursor> selected_range) override;
    void goto_next(std::string search) override;
    std::string store;
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

    std::string cached_search;

    // This variable is set/checked every time a view wants to display. So once
    // vertex data is generated, this is set to true, until any text is inserted to the buffer
    // at which point it is set to false. This way we don't have to reconstruct vertex data every render cycle.
    int find_line_begin(int i);
    int find_line_end(int i);
    int find_next_delimiter(int i);
    int find_prev_delimiter(int i);
    int find_line_start(int i);
};