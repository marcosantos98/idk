#include "ast.hpp"

void ASTBuilder::run()
{
    while (get_token().type != TokenType::END_OF_FILE)
    {
        auto bla = parse_expression();
        if (bla)
            m_expressions.emplace_back(std::move(bla));
    }
}

void ASTBuilder::advance_token()
{
    if (m_current_token < m_tokens.size())
        m_current_token++;
    else
        printf("exceeded max value: %ld\n", m_tokens.size() - 1);
}

int ASTBuilder::get_token_precedence()
{
    if (m_current_token >= m_tokens.size())
        return -1;

    if (!isascii(get_token().lex_value[0]))
        return -1;

    if (!m_op_precedence.contains(get_token().lex_value[0]))
        return -1;

    return m_op_precedence[get_token().lex_value[0]];
}

std::unique_ptr<Expression> ASTBuilder::parse_primary()
{
    switch (get_token().type)
    {
    case TokenType::NUMBER:
        return parse_number_literal();
    case TokenType::IDENTIFIER:
        return parse_variable_declaration();
    case TokenType::STRING:
        return parse_string_literal();
    default:
        printf("Token not handled: %s\n", get_token().lex_value.c_str());
        return nullptr;
    }
}

std::unique_ptr<Expression> ASTBuilder::parse_expression()
{
    auto lhs = parse_primary();
    if (!lhs)
        return nullptr;
    return parse_binary_right(0, std::move(lhs));
}

std::unique_ptr<NumberLiteral> ASTBuilder::parse_number_literal()
{
    double val = std::stod(get_token().lex_value);
    advance_token();
    return std::make_unique<NumberLiteral>(val);
}

std::unique_ptr<StringLiteral> ASTBuilder::parse_string_literal()
{
    std::string val = get_token().lex_value;
    advance_token();
    return std::make_unique<StringLiteral>(val);
}

std::unique_ptr<VariableDeclarationExpression> ASTBuilder::parse_variable_declaration()
{
    std::string class_name = get_token().lex_value;
    advance_token();
    if (get_token().type != TokenType::IDENTIFIER)
        return nullptr;
    std::string var_name = get_token().lex_value;
    advance_token();
    if (get_token().type != TokenType::OPERATOR && get_token().lex_value != "=")
        return nullptr;
    advance_token();
    auto value = parse_expression();
    return std::make_unique<VariableDeclarationExpression>(class_name, var_name, std::move(value));
}

std::unique_ptr<Expression> ASTBuilder::parse_binary_right(int precedence, std::unique_ptr<Expression> lhs)
{
    while (true)
    {
        if(get_token().type == TokenType::END_OF_FILE)
            return lhs;

        char op = get_token().lex_value[0];

        int current_precedence = get_token_precedence();
        if (current_precedence < precedence)
            return lhs;

        advance_token();

        auto rhs = parse_primary();
        if (!rhs)
            return nullptr;

        int next_precedence = get_token_precedence();
        if (current_precedence < next_precedence)
        {
            rhs = parse_binary_right(current_precedence + 1, std::move(rhs));
            if (!rhs)
                return nullptr;
        }

        lhs = std::make_unique<BinaryExpression>(op, std::move(lhs), std::move(rhs));
    }
}

void ASTBuilder::print_ast()
{
    for (auto &expression : m_expressions)
    {
        if (expression)
            printf("%s\n", expression.get()->to_str().c_str());
    }
}