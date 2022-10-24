#include "tokenizer.hpp"

#include <stdarg.h>

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
            parse_plus();
            break;
        case '-':
            parse_minus();
            break;
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
        case '[':
            m_tokens.emplace_back(with_current_token(TokenType::LB));
            break;
        case ']':
            m_tokens.emplace_back(with_current_token(TokenType::RB));
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
        case '!':
            parse_exclamation_mark();
            break;
        case '<':
            parse_less_sign();
            break;
        case '>':
            parse_greater_sign();
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
                printf("Unknown char: %c at %ld:%ld cursor pos %ld\n", m_input[m_cursor], m_row, m_col, m_cursor);
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
    while (!is_eof() && (isdigit(m_input[m_cursor]) || m_input[m_cursor] == '.'))
    {
        m_cursor++;
        m_col++;
    }
    
    auto val = m_input.substr(cursor_start, m_cursor - cursor_start);

    if(std::count(val.begin(), val.end(), '.') > 1)
        log_error("Only allowed one dot per number.\n");

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

void Tokenizer::parse_exclamation_mark()
{
    if (m_input[m_cursor + 1] == '=')
    {
        m_cursor += 2;
        m_col += 2;
        m_tokens.emplace_back(make_token("!=", TokenType::OPERATOR));
    }
    else
    {
        exit(1);
    }
}

void Tokenizer::parse_plus()
{
    if (m_input[m_cursor + 1] == '=')
    {
        m_cursor += 2;
        m_col += 2;
        m_tokens.emplace_back(make_token("+=", TokenType::OPERATOR));
    }
    else if (m_input[m_cursor + 1] == '+')
    {
        m_cursor += 2;
        m_col += 2;
        m_tokens.emplace_back(make_token("++", TokenType::OPERATOR));
    }
    else
    {
        m_tokens.emplace_back(with_current_token(TokenType::OPERATOR));
    }
}

void Tokenizer::parse_minus()
{
    if (m_input[m_cursor + 1] == '=')
    {
        m_cursor += 2;
        m_col += 2;
        m_tokens.emplace_back(make_token("-=", TokenType::OPERATOR));
    }
    else if (m_input[m_cursor + 1] == '-')
    {
        m_cursor += 2;
        m_col += 2;
        m_tokens.emplace_back(make_token("--", TokenType::OPERATOR));
    }
    else
    {
        m_tokens.emplace_back(with_current_token(TokenType::OPERATOR));
    }
}

void Tokenizer::parse_less_sign()
{
    if (m_input[m_cursor + 1] == '=')
    {
        m_cursor += 2;
        m_col += 2;
        m_tokens.emplace_back(make_token("<=", TokenType::OPERATOR));
    }
    else
    {
        m_tokens.emplace_back(with_current_token(TokenType::OPERATOR));
    }
}

void Tokenizer::parse_greater_sign()
{
    if (m_input[m_cursor + 1] == '=')
    {
        m_cursor += 2;
        m_col += 2;
        m_tokens.emplace_back(make_token(">=", TokenType::OPERATOR));
    }
    else
    {
        m_tokens.emplace_back(with_current_token(TokenType::OPERATOR));
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

bool Tokenizer::is_eof()
{
    return m_cursor >= m_input.length();
}

void Tokenizer::log_error(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    printf("\u001b[1m\u001b[31m[Tokenizer:%s]%ld:%ld:\u001b[0m ", m_path.c_str(), m_row, m_col);
    printf("%s", buffer);
    exit(1);
}