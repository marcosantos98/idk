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
        case '-':
        case '%':
        case '*':
        case '=':
            m_tokens.emplace_back(with_current_token(TokenType::OPERATOR));
            break;
        case '(':
            m_tokens.emplace_back(with_current_token(TokenType::LP));
            break;
        case ')':
            m_tokens.emplace_back(with_current_token(TokenType::RP));
            break;
        case '{':
            m_tokens.emplace_back(with_current_token(TokenType::LCP));
            break;
        case '}':
            m_tokens.emplace_back(with_current_token(TokenType::RCP));
            break;
        case '/':
            parse_slash();
            break;
        case '"':
            parse_string();
            break;
        default:
            if (isdigit(m_input[m_cursor]))
            {
                parse_number();
            }
            else if (isalpha(m_input[m_cursor]))
            {
                parse_identifier();
            }
            else
            {
                printf("Unknown char: %c\n", m_input[m_cursor]);
                m_cursor++;
            }
            break;
        }
    }
    m_tokens.emplace_back(make_token("EOF", TokenType::END_OF_FILE));
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
    m_cursor++; // Advance "
    size_t cursor_start = m_cursor;
    while (!is_eof() && m_input[m_cursor] != '"')
        m_cursor++;
    m_tokens.emplace_back(make_token(m_input.substr(cursor_start, m_cursor - cursor_start), TokenType::STRING));
    m_cursor++; // Advance "
}

void Tokenizer::parse_slash()
{
    if (m_input[m_cursor + 1] == '/') // Is a comment
    {
        while (!is_eof() && m_input[m_cursor] != '\n')
            m_cursor++;
    }
    else
    {
        m_tokens.emplace_back(make_token({m_input[m_cursor]}, TokenType::OPERATOR));
        m_cursor++;
    }
}

void Tokenizer::parse_identifier()
{
    uint64_t cursor_start = m_cursor;
    while (!is_eof() && isalpha(m_input[m_cursor]))
        m_cursor++;
    m_tokens.emplace_back(make_token(m_input.substr(cursor_start, m_cursor - cursor_start), TokenType::IDENTIFIER));
}

Token Tokenizer::make_token(std::string lex_value, TokenType type)
{
    return {lex_value, type};
}

Token Tokenizer::with_current_token(TokenType type)
{
    Token token = make_token({m_input[m_cursor]}, type);
    m_cursor++;
    return token;
}

void Tokenizer::print_token(Token token)
{
    printf("Token: Value[%s], Type[%s]\n", token.lex_value.c_str(), tokentype_to_token(token.type).c_str());
}

std::string Tokenizer::tokentype_to_token(TokenType type)
{
    switch (type)
    {
    case TokenType::NUMBER:
        return "TokenType::NUMBER";
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
    case TokenType::LCP:
        return "TokenType::LCP";
    case TokenType::RCP:
        return "TokenType::RCP";
    case TokenType::END_OF_FILE:
        return "TokenType::END_OF_FILE";
    default:
        printf("Unknown token.\n");
        exit(1);
    }
}

bool Tokenizer::is_eof()
{
    return m_cursor >= m_input.length();
}