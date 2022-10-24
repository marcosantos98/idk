#pragma once

#include "nava.hpp"

enum class TokenType
{
    IDENTIFIER,
    MODIFIER,
    BASE_TYPE,
    OPERATOR,
    NUMBER,
    STRING,
    LP,
    RP,
    LCP,
    RCP,
    LB,
    RB,
    COMMA,
    SEMI_COLON,
    END_OF_FILE
};

struct Token
{
    String lex_value;
    TokenType type;
    size_t row;
    size_t col;
};

class Tokenizer
{
public:
    Tokenizer(String input, String path) : m_input(input), m_path(path) {}

    void run();
    void print_token(Token);
    static String tokentype_to_token(TokenType type)
    {
        switch (type)
        {
        case TokenType::NUMBER:
            return "TokenType::NUMBER";
        case TokenType::MODIFIER:
            return "TokenType::MODIFIER";
        case TokenType::BASE_TYPE:
            return "TokenType::BASE_TYPE";
        case TokenType::OPERATOR:
            return "TokenType::OPERATOR";
        case TokenType::STRING:
            return "TokenType::STRING";
        case TokenType::IDENTIFIER:
            return "TokenType::IDENTIFIER";
        case TokenType::LP:
            return "TokenType::LP";
        case TokenType::RP:
            return "TokenType::RP";
        case TokenType::LB:
            return "TokenType::LB";
        case TokenType::RB:
            return "TokenType::RB";
        case TokenType::LCP:
            return "TokenType::LCP";
        case TokenType::RCP:
            return "TokenType::RCP";
        case TokenType::COMMA:
            return "TokenType::COMMA";
        case TokenType::SEMI_COLON:
            return "TokenType::SEMI_COLON";
        case TokenType::END_OF_FILE:
            return "TokenType::END_OF_FILE";
        default:
            printf("Unknown token.\n");
            exit(1);
        }
    }

    Vec<Token> get_tokens() const
    {
        return m_tokens;
    }

private:
    String m_input;
    String m_path;
    Vec<Token> m_tokens = {};

    size_t m_cursor = 0;
    size_t m_row = 0;
    size_t m_col = 0;

    void parse_number();
    void parse_string();
    void parse_slash();
    void parse_exclamation_mark();
    void parse_plus();
    void parse_minus();
    void parse_less_sign();
    void parse_greater_sign();
    void parse_identifier();
    Token make_token(String, TokenType);
    Token with_current_token(TokenType);
    bool is_eof();
    void log_error(const char *, ...);
};
