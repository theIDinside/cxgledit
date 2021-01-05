//
// Created by 46769 on 2021-01-04.
//

#pragma once
#include <vector>
#include <string>
#include <variant>

struct EditorWindow;

enum class LayoutType { Stack, Vertical, Horizontal };
template<typename T>
struct SplitDimInfo {
    T left;
    T right;
};

struct DimInfo {
    int x{0}, y{0}, w{0}, h{0};
    friend DimInfo scale(const DimInfo &dimInfo, float wRatio, float hRatio);
    static SplitDimInfo<DimInfo> split(const DimInfo &di, LayoutType layoutType);
    bool is_inside(int xpos, int ypos);
    void print();
    [[nodiscard]] std::string debug_str() const;
};

DimInfo scale(const DimInfo &dimInfo, float wRatio, float hRatio);

struct Layout {
    int id{};
    LayoutType type = LayoutType::Stack;
    DimInfo dimInfo{};
    Layout *parent{nullptr};
    Layout *left{nullptr};
    Layout *right{nullptr};
};


struct WindowLayout {
    int id;
    LayoutType type = LayoutType::Stack;
    DimInfo dimInfo;
    WindowLayout* parent;
    std::variant<WindowLayout*, EditorWindow*> left;
    std::variant<WindowLayout*, EditorWindow*> right;
};



Layout *find_by_id(Layout *root, int id);
void push_node(Layout *n, int assignedId, LayoutType t);
void update_layout_tree(Layout* root, float wRatio, float hRatio);
void promote_node(Layout*& node);
void dump_layout_tree(Layout* root);
void set_new_root(Layout*& old_root, Layout* new_root);
Layout* find_within_leaf_node(Layout* root, int xpos, int ypos);