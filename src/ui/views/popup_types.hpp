//
// Created by cx on 2021-03-25.
//

#pragma once
enum ModalContentsType {
    ActionList,
    Bookmarks,
    Item
};
enum class PopupActionType {
    Insert,     // for when selecting an item in the popup list, to insert that text at the cursor
    AppCommand, // for when the users selects an item meant to run a command that the App class understands
};