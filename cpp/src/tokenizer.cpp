#include "tokenizer.hpp"

#include "nava.hpp"

void Tokenizer::run()
{
    while (!is_eof())
    {
        switch (m_input[m_cursor])
        {
        case '\n':
        case '\r':
            m_cursor++;
            m_row++;
            m_col = 0;
            break;
        case '\t':
        case ' ':
            m_cursor++;
            m_col++;
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
        case ',':
            m_tokens.emplace_back(with_current_token(TokenType::COMMA));
            break;
        case ';':
            m_tokens.emplace_back(with_current_token(TokenType::SEMI_COLON));
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
                m_col++;
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
    {
        m_cursor++;
        m_col++;
    }
    m_tokens.emplace_back(make_token(m_input.substr(cursor_start, m_cursor - cursor_start), TokenType::NUMBER));
}

void Tokenizer::parse_string()
{
    m_cursor++; // Advance "
    m_col++;
    size_t cursor_start = m_cursor;
    while (!is_eof() && m_input[m_cursor] != '"')
    {
        m_cursor++;
        m_col++;
    }
    m_tokens.emplace_back(make_token(m_input.substr(cursor_start, m_cursor - cursor_start), TokenType::STRING));
    m_cursor++; // Advance "
    m_col++;
}

void Tokenizer::parse_slash()
{
    if (m_input[m_cursor + 1] == '/') // Is a comment
    {
        while (!is_eof() && m_input[m_cursor] != '\n')
        {
            m_cursor++;
            m_col++;
        }
    }
    else
    {
        m_tokens.emplace_back(make_token({m_input[m_cursor]}, TokenType::OPERATOR));
        m_cursor++;
        m_col++;
    }
}

void Tokenizer::parse_identifier()
{
    uint64_t cursor_start = m_cursor;
    while (!is_eof() && (isalpha(m_input[m_cursor]) || m_input[m_cursor] == '.'))
    {
        m_cursor++;
        m_col++;
    }

    auto val = m_input.substr(cursor_start, m_cursor - cursor_start);

    if (VEC_HAS(NAVA::modifiers, val))
        m_tokens.emplace_back(make_token(val, TokenType::MODIFIER));
    else if (VEC_HAS(NAVA::base_types, val))
        m_tokens.emplace_back(make_token(val, TokenType::BASE_TYPE));
    else
        m_tokens.emplace_back(make_token(val, TokenType::IDENTIFIER));
}

Token Tokenizer::make_token(std::string lex_value, TokenType type)
{
    return {lex_value, type, m_row, m_col - lex_value.length() + 1};
}

Token Tokenizer::with_current_token(TokenType type)
{
    Token token = make_token({m_input[m_cursor]}, type);
    m_cursor++;
    m_col++;
    return token;
}

void Tokenizer::print_token(Token token)
{
    printf("Token: Value[%s], Type[%s], Row[%ld], Col[%ld]\n", token.lex_value.c_str(), tokentype_to_token(token.type).c_str(), token.row, token.col);
}

std::string Tokenizer::tokentype_to_token(TokenType type)
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

bool Tokenizer::is_eof()
{
    return m_cursor >= m_input.length();
}