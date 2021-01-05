//
// Created by 46769 on 2021-01-03.
//

#include "view_manager.hpp"
ViewManager &ViewManager::get_instance() {
    static ViewManager vm;
    return vm;
}
