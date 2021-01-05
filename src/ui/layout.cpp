//
// Created by 46769 on 2021-01-04.
//

#include "layout.hpp"
#include <core/core.hpp>
#include <stack>

constexpr auto isParentId = [](auto id) { return id < 0; };

DimInfo scale(const DimInfo &dimInfo, float wRatio, float hRatio) {
    return DimInfo{.x = dimInfo.x,
                   .y = dimInfo.y,
                   .w = int(float(dimInfo.w) * wRatio),
                   .h = int(float(dimInfo.h) * hRatio)};
}

Layout *find_by_id(Layout *root, int id) {
    if (root == nullptr) return nullptr;
    if (root->id == id) return root;
    else {
        if (root->left) {
            if (auto e = find_by_id(root->left, id); e) return e;
        }
        if (root->right) {
            if (auto e = find_by_id(root->right, id); e) return e;
        }
        return nullptr;
    }
}

void push_node(Layout *n, int assignedId, LayoutType t) {
    if (!isParentId(n->id)) {
        auto &[x, y, w, h] = n->dimInfo;
        if (t == LayoutType::Vertical) {

        } else if (t == LayoutType::Horizontal) {
            auto occupyingID = n->id;
            n->id *= -1;

            auto [left, right] = DimInfo::split(n->dimInfo, LayoutType::Horizontal);

            auto l = new Layout{};
            n->left = l;
            l->id = occupyingID;
            l->parent = n;
            l->type = t;
            l->dimInfo = left;

            auto r = new Layout{};
            n->right = r;
            r->id = assignedId;
            r->parent = n;
            r->type = t;
            r->dimInfo = right;
            n->type = t;
        } else {
            PANIC("Only horizontal layout is supported right now");
        }
    }
}
/// Recursive function that traverses layout tree and updates according to ratio passed in
void update_layout_tree(Layout *root, float wRatio, float hRatio) {
    if (root == nullptr) return;
    if (root->parent == nullptr) {
        root->dimInfo = scale(root->dimInfo, wRatio, hRatio);
        if (root->left) {
            root->left->dimInfo.y = root->dimInfo.y;
            update_layout_tree(root->left, wRatio, hRatio);
        }
        if (root->right) {
            root->right->dimInfo.x = root->left->dimInfo.x + root->left->dimInfo.w;
            update_layout_tree(root->right, wRatio, hRatio);
        }
        return;
    } else {
        switch (root->type) {
            case LayoutType::Stack:
                PANIC("Stacking split windows not supported. (possibly never will)");
                break;
            case LayoutType::Vertical:
                PANIC("Vertical layout not yet supported");
                break;
            case LayoutType::Horizontal: {
                // left children keep their x & y pos, and gets updated first, so right sub child can query the left's new position & dim data
                if (root->parent->left == root) {
                    root->dimInfo = scale(root->dimInfo, wRatio, hRatio);
                    root->dimInfo.x = root->parent->dimInfo.x;
                    root->dimInfo.y = root->parent->dimInfo.y;
                } else {
                    util::println("\t\t Root->parent->left dim: {}", root->parent->left->dimInfo.debug_str());
                    root->dimInfo.x = root->parent->left->dimInfo.x + root->parent->left->dimInfo.w;
                    root->dimInfo.y = root->parent->dimInfo.y;
                    root->dimInfo = scale(root->dimInfo, wRatio, hRatio);
                    util::println("\t\t Node dim: {}", root->dimInfo.debug_str());
                }
                update_layout_tree(root->left, wRatio, hRatio);
                update_layout_tree(root->right, wRatio, hRatio);
                break;
            }
        }
    }
}

void promote_node(Layout *&node) {
    if (node->parent == nullptr) {
        util::println("Node {} is root", node->id);
        return;
    }
    const auto pDims = node->parent->dimInfo;
    auto dim = node->dimInfo;
    auto parent = node->parent;
    auto grandparent = node->parent->parent;

    if (grandparent == nullptr) {
        auto wRatio = pDims.w / node->dimInfo.w;
        auto hRatio = pDims.h / node->dimInfo.h;
        node->dimInfo.x = pDims.x;
        node->dimInfo.y = pDims.y;
        util::println("Upgrading dim to root. From [{}] -> [{}]", dim.debug_str(), pDims.debug_str());
        update_layout_tree(node, wRatio, hRatio);
        node->parent = node;
        // std::swap(node->parent, node);
        delete node;
    } else {
        if (isParentId(node->id)) {
            auto wRatio = node->parent->dimInfo.w / node->dimInfo.w;
            auto hRatio = node->parent->dimInfo.h / node->dimInfo.h;
            node->dimInfo = pDims;
            node->left->dimInfo.x = node->dimInfo.x;
            node->left->dimInfo.y = node->dimInfo.y;
            update_layout_tree(node, wRatio, hRatio);
            if (parent == grandparent->left) {
                grandparent->left = node;
            } else {
                grandparent->right = node;
            }
        } else {
            util::println("Upgrading dim from [{}] -> [{}]", dim.debug_str(), pDims.debug_str());
            if (parent == grandparent->left) {
                grandparent->left = node;
                node->dimInfo = pDims;
            } else {
                grandparent->right = node;
                node->dimInfo = pDims;
            }
        }
        node->parent = grandparent;
        util::println("New dimension: {}", node->dimInfo.debug_str());
        delete parent;
    }
}
void dump_layout_tree(Layout *root) {
    if (root != nullptr) {
        if (root->parent == nullptr) {
            auto rootInfo = fmt::format("{} - Pos: [{}, {}], Dim: [{}, {}]", root->id, root->dimInfo.x, root->dimInfo.y,
                                        root->dimInfo.w, root->dimInfo.h);
            std::string separator(rootInfo.size(), '-');
            util::println("Root (screen): \n{}\n{}", rootInfo, separator);
        } else {
            util::println("{} - Pos: [{}, {}], Dim: [{}, {}]", root->id, root->dimInfo.x, root->dimInfo.y,
                          root->dimInfo.w, root->dimInfo.h);
        }
        dump_layout_tree(root->left);
        dump_layout_tree(root->right);
    }
}
/// Takes old_root by referece, so we actually set the original pointer (which has the lifetime/exists in the struct App)
/// and sets this to point to new_root, while deleting the object which old_root pointed to
void set_new_root(Layout *&old_root, Layout *new_root) {
    new_root->dimInfo = old_root->dimInfo;
    new_root->parent = nullptr;
    auto toDelete = old_root;
    old_root = new_root;
    delete toDelete;
}
Layout *find_within_leaf_node(Layout *root, int xpos, int ypos) {
    if (root == nullptr) return nullptr;
    if (isParentId(root->id)) {
        auto isInside = root->dimInfo.is_inside(xpos, ypos);
        if (isInside) {
            auto res = find_within_leaf_node(root->left, xpos, ypos);
            if (res) return res;
            res = find_within_leaf_node(root->right, xpos, ypos);
            if (res) return res;
            return nullptr;
        } else {
            return nullptr;
        }
    } else {
        auto isInside = root->dimInfo.is_inside(xpos, ypos);
        if (isInside) {
            auto tmp = root;
            auto res = find_within_leaf_node(root->left, xpos, ypos);
            if (res) return res;
            res = find_within_leaf_node(root->right, xpos, ypos);
            if (res) return res;
            return tmp;
        } else {
            return nullptr;
        }
    }
}

SplitDimInfo<DimInfo> DimInfo::split(const DimInfo &di, LayoutType layoutType) {
    switch (layoutType) {
        case LayoutType::Stack:
            PANIC("Can't split a Stacking window");
            break;
        case LayoutType::Vertical:
            PANIC("Vertical layout not yet supported");
            break;
        case LayoutType::Horizontal: {
            auto &[x_aPos, y, w, h] = di;
            auto x_bPos = x_aPos + w / 2;
            DimInfo right{.x = x_bPos, .y = y, .w = w / 2, .h = h};
            DimInfo LEFT{.x = x_aPos * 1, .y = y, .w = w / 2, .h = h};
            auto result = SplitDimInfo<DimInfo>{LEFT, right};
            return result;
        };
    }
}

void DimInfo::print() { util::println("Pos: [{}, {}], Dim: [{}, {}]", x, y, w, h); }
std::string DimInfo::debug_str() const {
    return fmt::format("Pos: ({}, {}) \t Dimension [{} x {}]", this->x, this->y, this->w, this->h);
}

bool DimInfo::is_inside(int xpos, int ypos) {
    return (xpos >= x && xpos <= x + w) && (ypos >= (y - h) && ypos < y);
}
