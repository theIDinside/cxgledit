//
// Created by 46769 on 2021-01-03.
//

#pragma once
struct ViewManager {
public:
    static ViewManager& get_instance();

private:
    ViewManager() = default;
};
