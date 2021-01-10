//
// Created by 46769 on 2021-01-09.
//

#include "syntax_highlighting.hpp"
#include <cctype>

static Context lex_ctx = Context::Block;
static TokenType last_lexed;

#ifdef DEBUG
std::string token_ident_to_string(TokenType type) {
    switch (type) {
        case TokenType::Keyword:
            return "Keyword";
        case TokenType::Namespace:
            return "Namespace";
        case TokenType::Variable:
            return "Variable";
        case TokenType::ParameterType:
            return "FunctionParamType";
        case TokenType::Parameter:
            return "FunctionParam";
        case TokenType::Function:
            return "Function";
        case TokenType::StringLiteral:
            return "StringLiteral";
        case TokenType::NumberLiteral:
            return "NumberLiteral";
        case TokenType::Comment:
            return "Comment";
        case TokenType::Illegal:
            return "Illegal";
    }
}
#endif

std::optional<Token> number_literal(std::string_view &text, std::size_t pos) {
    auto sz = text.size();
    auto begin = pos;

    for (auto i = pos; i < sz; i++) {
        if (text[i] == 'x' || text[i] == 'X') {
            for (auto j = i + 1; j < sz; j++) {
                if (not std::isxdigit(text[j])) {
                    lex_ctx = Context::Free;
                    last_lexed = TokenType::NumberLiteral;
                    return Token{begin, j, TokenType::NumberLiteral};
                }
            }
        }

        if (not std::isdigit(text[i]) && not(text[i] == '.' || text[i] == 'f' || text[i] == 'l')) {
            lex_ctx = Context::Free;
            last_lexed = TokenType::NumberLiteral;
            return Token{begin, i, TokenType::NumberLiteral};
        }
    }
    return {};
}

std::optional<Token> string_literal(std::string_view &text, std::size_t pos) {
    auto sz = text.size();
    auto begin = pos;
    auto quotes = 2;
    for (auto i = pos; i < sz; i++) {
        auto ch = text[i];
        if (text[i] == '"') quotes--;
        if (text[i] == '"' && text[i - 1] != '\\' && quotes == 0) {
            lex_ctx = Context::Free;
            last_lexed = TokenType::StringLiteral;
            return Token{begin, i + 1, TokenType::StringLiteral};
        }
        if (text[i] == '>' && last_lexed == TokenType::Macro) {
            lex_ctx = Context::Block;
            last_lexed = TokenType::Include;
            std::string res{text.substr(begin, i - begin)};
            return Token{begin, i + 1, TokenType::Include};
        }
    }
    return {};
}

constexpr auto named_ch_ok = [](auto ch) { return std::isalpha(ch) || (ch == '_') || (std::isdigit(ch)); };

std::optional<Token> named(std::string_view &text, std::size_t pos) {
    auto sz = text.size();
    auto begin = pos;
    for (auto i = pos; i < sz; i++) {
        if (not named_ch_ok(text[i])) {
            if (text[i] == '(') {
                lex_ctx = Context::FunctionSignature;
                last_lexed = TokenType::Function;
                return Token{begin, i, TokenType::Function};
            } else if (text[i] == ':' && text[i + 1] == ':') {
                last_lexed = TokenType::Namespace;
                lex_ctx = Context::Type;
                return Token{begin, i, TokenType::Namespace};
            } else {
                if (lex_ctx == Context::FunctionSignature) {
                    if (last_lexed == TokenType::Function || last_lexed == TokenType::Parameter) {
                        last_lexed = TokenType::ParameterType;
                        return Token{begin, i, TokenType::ParameterType};
                    } else {
                        last_lexed = TokenType::Parameter;
                        return Token{begin, i, TokenType::Parameter};
                    }
                } else if (lex_ctx == Context::Type) {
                    auto l = (last_lexed == TokenType::Namespace) ? TokenType::Keyword : TokenType::Variable;
                    lex_ctx = (last_lexed == TokenType::Namespace) ? Context::Type : Context::Free;
                    last_lexed = l;
                    return Token{begin, i, l};
                } else {
                    lex_ctx = Context::Type;
                    last_lexed = TokenType::Keyword;
                    return Token{begin, i, TokenType::Keyword};
                }
            }
        }
    }
    return {};
}

std::optional<Token> line_comment(std::string_view &text, std::size_t pos) {
    auto sz = text.size();
    auto begin = pos;
    for (auto i = pos; i < sz; i++) {
        last_lexed = TokenType::Comment;
        lex_ctx = Context::Block;
        if (text[i] == '\n') { return Token{begin, i, TokenType::Comment}; }
    }
    return Token{begin, sz, TokenType::Comment};
}

std::optional<Token> block_comment(std::string_view &text, std::size_t pos) {
    auto sz = text.size();
    auto begin = pos;
    for (auto i = pos; i < sz; i++) {
        last_lexed = TokenType::Comment;
        lex_ctx = Context::Block;
        if (text[i] == '\n') { return Token{begin, i, TokenType::Comment}; }
    }
    return Token{begin, sz, TokenType::Comment};
}

using namespace std::string_view_literals;
constexpr auto keyword_include = "#include"sv;
constexpr auto kw_inc_length = keyword_include.size();

std::optional<Token> macro(std::string_view &text, std::size_t pos) {
    static auto escape_character_found = false;
    auto begin = pos;

    auto scan_ahead = std::find_if(text.begin() + pos, text.end(), [](auto ch) {
        if (ch == '\\') escape_character_found = true;
        if (ch == '\n' && not escape_character_found) {
            return true;
        } else if (ch == '\n' && escape_character_found) {
            escape_character_found = false;
        }
        return false;
    });
    auto distance = std::distance(text.begin() + pos, scan_ahead);
    if (text.substr(pos, kw_inc_length) == keyword_include) {
        std::string s{text.substr(pos, kw_inc_length)};
        lex_ctx = Context::Macro;
        last_lexed = TokenType::Macro;
        return Token{begin, begin + kw_inc_length, last_lexed};
    }
    return {};
}

std::vector<Token> tokenize(std::string_view text) {
    std::vector<Token> result;
    auto sz = text.size();
    if (sz < 2) return result;
    for (auto i = 0u; i < sz - 1; i++) {
        if (text[i] == '/') {
            if (text[i + 1] == '/') {
                auto token = line_comment(text, i);
                if (token) {
                    result.push_back(*token);
                    i = token->end;
                }
            } else if (text[i + 1] == '*') {
                auto token = block_comment(text, i);
                if (token) {
                    result.push_back(*token);
                    i = token->end;
                }
            }
        } else if (std::isdigit(text[i])) {
            auto token = number_literal(text, i);
            if (token) {
                result.push_back(*token);
                i = token->end;
            }
        } else if (text[i] == '"') {
            auto token = string_literal(text, i);
            if (token) {
                result.push_back(*token);
                i = token->end;
            }
        } else if (text[i] == '<' && last_lexed == TokenType::Macro) {
            auto token = string_literal(text, i);
            if (token) {
                result.push_back(*token);
                i = token->end;
            }
        } else if (std::isalpha(text[i]) && last_lexed == TokenType::Namespace) {
            auto token = named(text, i);
            if (token) {
                result.push_back(*token);
                i = token->end;
            }
        } else if (std::isalpha(text[i])) {
            auto token = named(text, i);
            if (token) {
                result.push_back(*token);
                i = token->end;
            }
        } else if (text[i] == '#') {
            auto token = macro(text, i);
            if (token) {
                result.push_back(*token);
                i = token->end;
            }
        }
        if (text[i] == ')' && lex_ctx == Context::FunctionSignature) { lex_ctx = Context::Block; }
    }
    return result;
}
