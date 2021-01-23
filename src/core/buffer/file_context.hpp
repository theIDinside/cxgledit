//
// Created by 46769 on 2021-01-23.
//

#pragma once
#include <filesystem>

namespace fs = std::filesystem;

enum ContexTypes {
    CPPHeader,
    CPPSource,
    Config,
    Unhandled
};

struct FileContext {
    ContexTypes type;
    fs::path path;
};