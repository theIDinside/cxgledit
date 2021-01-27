//
// Created by 46769 on 2021-01-09.
//

#include "syntax_highlighting.hpp"
#include <cctype>
#include <ui/render/font.hpp>
#include <utils/utils.hpp>

static Context lex_ctx = Context::Block;

static TokenType last_lexed;
static bool using_keyword_preceded = false;
static bool using_namespace_found = false;

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
        case TokenType::Statement:
            return "Statement";
        case TokenType::Include:
            return "Include";
        case TokenType::Macro:
            return "Macro";
        case TokenType::Qualifier:
            return "Qualifier";
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

// This doesn't just return true for qualifiers, but keywords that live in qualifier-ish space, meaning, their
// locality in the text source code is similar to that of a qualifier, such as the keyword "using" which always comes before
// a type, namespace or something like that
static bool is_qualifier_ish(std::string_view token) {
    return (token == "const" || token == "constexpr" || token == "consteval" || token == "static" ||
            token == "volatile" || token == "unsigned" || token == "using");
}

std::optional<Token> named(std::string_view &text, std::size_t pos) {
    auto sz = text.size();
    auto begin = pos;
    for (auto i = pos; i < sz - 1; i++) {
        if (not named_ch_ok(text[i])) {
            if (is_qualifier_ish(text.substr(begin, i - begin))) {
                last_lexed = TokenType::Qualifier;
                if (text.substr(begin, i - begin) == "using") using_keyword_preceded = true;
                return Token{begin, i, last_lexed};
            }
            if (text[i] == '(') {
                lex_ctx = Context::FunctionSignature;
                last_lexed = TokenType::Function;
                return Token{begin, i, TokenType::Function};
            } else if (text[i] == ':' && text[i + 1] == ':') {
                if (using_keyword_preceded) using_namespace_found = true;
                last_lexed = TokenType::Namespace;
                lex_ctx = Context::Type;
                return Token{begin, i, TokenType::Namespace};
            } else {
                if (lex_ctx == Context::FunctionSignature) {
                    if (last_lexed == TokenType::Function || last_lexed == TokenType::Parameter ||
                        last_lexed == TokenType::Qualifier) {
                        last_lexed = TokenType::ParameterType;
                        return Token{begin, i, TokenType::ParameterType};
                    } else {
                        last_lexed = TokenType::Parameter;
                        return Token{begin, i, TokenType::Parameter};
                    }
                } else if (lex_ctx == Context::Type) {
                    auto l = (last_lexed == TokenType::Namespace) ? TokenType::Keyword : TokenType::Variable;
                    lex_ctx = (last_lexed == TokenType::Namespace) ? Context::Type : Context::Free;
                    if (using_namespace_found) {
                        using_keyword_preceded = false;
                        using_namespace_found = false;
                        lex_ctx = Context::Free;
                        l = TokenType::Keyword;
                        last_lexed = l;
                        return Token{begin, i, l};
                    } else {
                        last_lexed = l;
                        return Token{begin, i, l};
                    }
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
    auto closing_delim_found = false;
    for (auto i = pos; i < sz - 2; i++) {
        last_lexed = TokenType::Comment;
        lex_ctx = Context::Block;
        if (text[i] == '*') closing_delim_found = true;
        else if (text[i] == '/') {
            if (closing_delim_found) {
                return Token{begin, i + 1, TokenType::Comment};
            } else {
                closing_delim_found = false;
            }
        }
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
    // FN_MICRO_BENCH();
    std::vector<Token> result;
    std::vector<ColorFormatInfo> keywords_ranges;

    auto sz = text.size();
    // TODO: actually measure this. I *know* that if this is going to be thousands of elements long,
    //  reserving space will help, even without having measured, it's simple to reason about; every time it runs out of space
    //  the strategy for std::vector is to double it's size, so that becomes 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
    //  and every time, the new elements have to be copied to the new memory range. If the file had tens of thousands of elements,
    //  this could get noticibly slower, by milliseconds, still, I should measure this
    result.reserve(sz / 3);// if we guess that a token average length is 3 characters, we get this reserved number
    if (sz < 2) return result;
    for (auto i = 0u; i < sz - 2; i++) {
        if (std::isspace(text[i])) continue;
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
        } else if (text[i] == ')' && lex_ctx == Context::FunctionSignature) {
            lex_ctx = Context::Block;
        } else if (text[i] == ';') {
            lex_ctx = Context::Free;
            last_lexed = TokenType::Statement;
        }
    }
    return result;
}

std::vector<ColorFormatInfo> format_tokens(const std::vector<Token> &tokens) {
    std::vector<ColorFormatInfo> result;
    result.reserve(tokens.size());
    for (const auto &[begin, end, type] : tokens) {
        switch (type) {
            case TokenType::Qualifier:
            case TokenType::Keyword:
            case TokenType::Namespace:
            case TokenType::ParameterType:
                result.emplace_back(ColorFormatInfo{begin, end, {0.820f, 0.500f, 0.000f}});
                break;
                // case TokenType::Variable:
                // case TokenType::Parameter:
                // case TokenType::Function:
            case TokenType::StringLiteral:
                result.emplace_back(ColorFormatInfo{begin, end, DARKER_GREEN});
                break;
            case TokenType::NumberLiteral:
                result.emplace_back(ColorFormatInfo{begin, end, BLUE});
                break;
            case TokenType::Comment:
                result.emplace_back(ColorFormatInfo{begin, end, LIGHT_GRAY});
                break;
            case TokenType::Macro:
                result.emplace_back(ColorFormatInfo{begin, end, YELLOW});
                break;
            case TokenType::Include:
                result.emplace_back(ColorFormatInfo{begin, end, DARKER_GREEN});
                break;
            default:
                break;
        }
    }
    return result;
}

std::vector<ColorFormatInfo> color_format_tokenize(std::string_view text) {
    std::vector<ColorFormatInfo> result;
    auto sz = text.size();
    // TODO: actually measure this. I *know* that if this is going to be thousands of elements long,
    //  reserving space will help, even without having measured, it's simple to reason about; every time it runs out of space
    //  the strategy for std::vector is to double it's size, so that becomes 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096
    //  and every time, the new elements have to be copied to the new memory range. If the file had tens of thousands of elements,
    //  this could get noticibly slower, by milliseconds, still, I should measure this
    result.reserve(sz / 3);// if we guess that a token average length is 3 characters, we get this reserved number
    if (sz < 2) return result;
    for (auto i = 0u; i < sz - 2; i++) {
        if (std::isspace(text[i])) continue;
        if (text[i] == '/') {
            if (text[i + 1] == '/') {
                auto token = line_comment(text, i);
                if (token) {
                    result.push_back(ColorFormatInfo{token->begin, token->end, LIGHT_GRAY});
                    i = token->end;
                }
            } else if (text[i + 1] == '*') {
                auto token = block_comment(text, i);
                if (token) {
                    result.push_back(ColorFormatInfo{token->begin, token->end, LIGHT_GRAY});
                    i = token->end;
                }
            }
        } else if (std::isdigit(text[i])) {
            auto token = number_literal(text, i);
            if (token) {
                result.push_back(ColorFormatInfo{token->begin, token->end, BLUE});
                i = token->end;
            }
        } else if (text[i] == '"') {
            auto token = string_literal(text, i);
            if (token) {
                result.push_back(ColorFormatInfo{token->begin, token->end, DARKER_GREEN});
                i = token->end;
            }
        } else if (text[i] == '<' && last_lexed == TokenType::Macro) {
            auto token = string_literal(text, i);
            if (token) {
                result.push_back(ColorFormatInfo{token->begin, token->end, DARKER_GREEN});
                i = token->end;
            }
        } else if (std::isalpha(text[i]) && last_lexed == TokenType::Namespace) {
            auto token = named(text, i);
            if (token) {
                result.push_back(ColorFormatInfo{token->begin, token->end, {0.820f, 0.500f, 0.000f}});
                i = token->end;
            }
        } else if (std::isalpha(text[i])) {
            auto token = named(text, i);
            if (token) {
                switch (token->type) {
                    case TokenType::Qualifier:
                    case TokenType::Keyword:
                    case TokenType::Namespace:
                    case TokenType::ParameterType:
                        result.push_back(ColorFormatInfo{token->begin, token->end, {0.820f, 0.500f, 0.000f}});
                        break;
                    default:
                        result.push_back(ColorFormatInfo{token->begin, token->end, {1, 1, 1}});
                        break;
                }
                i = token->end;
            }
        } else if (text[i] == '#') {
            auto token = macro(text, i);
            if (token) {
                result.push_back(ColorFormatInfo{token->begin, token->end, YELLOW});
                i = token->end;
            }
        } else if (text[i] == ')' && lex_ctx == Context::FunctionSignature) {
            lex_ctx = Context::Block;
        } else if (text[i] == ';') {
            lex_ctx = Context::Free;
            last_lexed = TokenType::Statement;
        }
    }
    return result;
}
std::vector<ColorFormatInfo> color_format_tokenize_range(const char *begin, std::size_t len, std::size_t offset_of) {
    auto res = color_format_tokenize(std::string_view{begin, len});
    for(auto& e : res) {
        e.begin += offset_of;
        e.end += offset_of;
    }
    return res;
}
