#include "tokenizer.hpp"

void Tokenizer::run()
{
    while (!is_eof())
    {
        switch (m_input[m_cursor])
        {
        case '\n':
        case '\b':
        case '\t':
        case ' ':
            m_cursor++;
            break;
        case '+':
            m_tokens.emplace_back(make_token("+", TokenType::PLUS));
            m_cursor++;
            break;
        case '"':
            parse_string();
            break;
        case '/':
            parse_comment();
            break;
        default:
            if (isdigit(m_input[m_cursor]))
            {
                parse_number();
            }
            else
            {
                printf("Unknown char: %c\n", m_input[m_cursor]);
                m_cursor++;
            }
            break;
        }
    }
}

void Tokenizer::parse_number()
{
    size_t cursor_start = m_cursor;
    while (!is_eof() && isdigit(m_input[m_cursor]))
        m_cursor++;
    m_tokens.emplace_back(make_token(m_input.substr(cursor_start, m_cursor - cursor_start), TokenType::NUMBER));
}

void Tokenizer::parse_string()
{
    m_cursor++; //Advance "
    size_t cursor_start = m_cursor;
    while(!is_eof() && m_input[m_cursor] != '"')
        m_cursor++;
    m_tokens.emplace_back(make_token(m_input.substr(cursor_start, m_cursor - cursor_start), TokenType::STRING));
    m_cursor++; //Advance "
}

void Tokenizer::parse_comment()
{
    while (!is_eof() && m_input[m_cursor] != '\n')
        m_cursor++;
}

Token Tokenizer::make_token(std::string lex_value, TokenType type)
{
    return {lex_value, type};
}

void Tokenizer::print_token(Token token)
{
    printf("Token: Value[%s], Type[%s]\n", token.lex_value.c_str(), tokentype_to_token(token.type).c_str());
}

std::string Tokenizer::tokentype_to_token(TokenType type)
{
    switch(type) {
        case TokenType::NUMBER:
            return "TokenType::NUMBER";
        case TokenType::PLUS:
            return "TokenType::PLUS";
        case TokenType::STRING:
            return "TokenType::STRING";
        default:
            printf("Unknown token.\n");
            exit(1);
    }
}

bool Tokenizer::is_eof()
{
    return m_cursor >= m_input.length();
}