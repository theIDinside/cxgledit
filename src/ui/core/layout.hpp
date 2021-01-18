//
// Created by 46769 on 2021-01-04.
//

#pragma once

#include <string>

/// Forward declarations
namespace ui {
    struct EditorWindow;
}

namespace ui::core {

    /// Screen coordinates
    struct ScreenPos {
        int x, y;
    };

    /// Sets children of a Layout to be laid out according to these rules
    enum class LayoutType { Stack, Vertical, Horizontal };

    /// Utility struct, basically a pair
    template<typename T>
    struct SplitDimInfo {
        T left;
        T right;
    };

    /// Dimensions and top-left corner position of a screen object, such as an EditorWindow, Statusbar etc
    struct DimInfo {
        int x{0}, y{0}, w{0}, h{0};
        friend DimInfo scale(const DimInfo &dimInfo, float wRatio, float hRatio);
        static SplitDimInfo<DimInfo> split(const DimInfo &di, LayoutType layoutType);
        bool is_inside(int xpos, int ypos);
        [[nodiscard]] std::string debug_str() const;
    };

    DimInfo scale(const DimInfo &dimInfo, float wRatio, float hRatio);

    /// Layout tree structure for all screen objects, such as EditorWindow, Statusbar etc
    struct Layout {
        /// The editor window's ID which is tied to this layout
        int id{};
        /// The layout type of it's children (left/right), whether they are split vertically/horizontally, or maximized by stacking on top of each other
        LayoutType type = LayoutType::Stack;
        /// Actual dimensions of the layout
        DimInfo dimInfo{};
        /// Tree structure bookkeeping
        Layout *parent{nullptr};
        Layout *left{nullptr};
        Layout *right{nullptr};
    };

    /// Functions for operating on a Layout tree
    /// Find an Layout object in the tree, starting at root with id
    Layout *find_by_id(Layout *root, int id);

    /// Creates a new node with id=assignedId, and LayoutType t and pushes it down into sub-tree root *n
    /// making the node at n, be replaced with a new Branch node, while n gets pushed into left subtree, new node
    /// gets pushed into right subtree
    void push_node(Layout *n, int assignedId, LayoutType t);

    /// Updates size all layout children of Layout* root, with the factor of wRatio and hRatio. The layout's
    /// anchor positions (top-left corner screen position) will be updated accordingly as well.
    void update_layout_tree(Layout* root, float wRatio, float hRatio);

    /// Resizes root to width, height, and updates children accordlingly
    void root_resize_to(Layout* root, int width, int height);
    /// Promotes a node to a parent. This is done when one of two children L/R (left/right) is deleted or removed
    /// When that happens, the other child gets promoted one level/one step up in the tree. So each node will _always_
    /// either have 2 valid children (making it a 'Branch' node), or none at all (making it a 'leaf' node) (*except* for the tree's ancestor root, it can have 1 child)
    void promote_node(Layout*& node);

    /// Debug printing of layout tree structure
    void dump_layout_tree(Layout* root);
    /// Seeding a new root at memory position old_root, making the pointer in that memory space, point to new_root
    void set_new_root(Layout*& old_root, Layout* new_root);

    /// Find's a leaf node within root, which (xpos, ypos) existt in. This is basically an AABB algorithm
    Layout* find_within_leaf_node(Layout* root, int xpos, int ypos);
}