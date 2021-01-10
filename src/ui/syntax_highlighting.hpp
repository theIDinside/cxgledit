//
// Created by 46769 on 2021-01-09.
//

#pragma once
#include <vector>
#include <optional>
#include <string_view>

enum class TokenType {
    Illegal,
    Keyword,
    Namespace,
    Variable,
    ParameterType,
    Parameter,
    Function,
    StringLiteral,
    NumberLiteral,
    Comment,
    Include,
    Macro,
    Qualifier,
    Statement// const, consteval, constexpr, static, volatile
};

struct Token {
    std::size_t begin, end;
    TokenType type = TokenType::Keyword;
};

enum class Context {
    Free,
    StringLiteral,
    FunctionSignature,
    Type,
    Block,
    Macro
};

#ifdef DEBUG
std::string token_ident_to_string(TokenType type);
#endif

/// TODO: these functions are going to take my custom data buffer. Once I write it. Gapbuffer is easy to implement and I've done it before, so
///  that isn't really hard but it comes with it's own set of problems. Right now we'll just deal with std::string and string_views

std::vector<Token> tokenize(std::string_view text);