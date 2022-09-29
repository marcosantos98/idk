#pragma once

#include <vector>
#include <string>

enum class TokenType
{
    PLUS,
    NUMBER,
    STRING
};

struct Token
{
    std::string lex_value;
    TokenType type;
};

class Tokenizer
{
public:
    Tokenizer(std::string input) : m_input(input) {}

    void run();
    void print_token(Token);

    std::vector<Token> get_tokens() const
    {
        return m_tokens;
    }

private:
    std::string m_input;
    std::vector<Token> m_tokens = {};

    uint64_t m_cursor = 0;

    void parse_number();
    void parse_string();
    void parse_comment();
    Token make_token(std::string, TokenType);
    std::string tokentype_to_token(TokenType);
    bool is_eof();
};
