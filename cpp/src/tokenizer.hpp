#pragma once

#include <vector>
#include <string>

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
    COMMA,
    SEMI_COLON,
    END_OF_FILE
};

struct Token
{
    std::string lex_value;
    TokenType type;
    size_t row;
    size_t col;
};

class Tokenizer
{
public:
    Tokenizer(std::string input) : m_input(input) {}

    void run();
    void print_token(Token);
    std::string tokentype_to_token(TokenType);

    std::vector<Token> get_tokens() const
    {
        return m_tokens;
    }

private:
    std::string m_input;
    std::vector<Token> m_tokens = {};

    size_t m_cursor = 0;
    size_t m_row = 0;
    size_t m_col = 0;

    void parse_number();
    void parse_string();
    void parse_slash();
    void parse_exclamation_mark();
    void parse_plus();
    void parse_less_sign();
    void parse_identifier();
    Token make_token(std::string, TokenType);
    Token with_current_token(TokenType);
    bool is_eof();
};
