#include "ast.hpp"
#include "json.hpp"

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

std::unique_ptr<ClassDefinitionExpression> ASTBuilder::parse_class_definition()
{

    advance_token(); // eat class

    if (get_token().type != TokenType::IDENTIFIER)
        return nullptr;

    m_current_class_name = get_token().lex_value;
    advance_token();

    if (get_token().type != TokenType::LCP)
        return nullptr;

    advance_token(); // Eat {

    std::vector<std::unique_ptr<Expression>> vars = {};
    std::vector<std::unique_ptr<Expression>> methods = {};

    while (get_token().type != TokenType::RCP)
    {
        auto expression = parse_expression();

        if (dynamic_cast<const VariableDeclarationExpression *>(expression.get()) != nullptr)
            vars.emplace_back(std::move(expression));
        else if (dynamic_cast<const MethodDefinitionExpression *>(expression.get()) != nullptr)
            methods.emplace_back(std::move(expression));
    }

    if (get_token().type != TokenType::RCP)
        return nullptr;

    advance_token(); // Eat }

    bool s = has_static;
    has_static = false;

    return std::make_unique<ClassDefinitionExpression>(std::move(vars), std::move(methods), m_current_mod, s, m_current_class_name);
}

std::unique_ptr<CallExpression> ASTBuilder::parse_method_call()
{
    std::string method_name = get_token().lex_value;
    advance_token();
    advance_token(); // Eat (

    std::vector<std::string> args = {};

    while (get_token().type != TokenType::RP)
    {
        std::string arg_name = get_token().lex_value;
        advance_token();
        // fixme 22/10/03: hmm
        if (get_token().type == TokenType::COMMA)
            advance_token();

        args.emplace_back(arg_name);
    }

    advance_token(); // Eat )

    return std::make_unique<CallExpression>(method_name, args);
}

std::unique_ptr<Expression> ASTBuilder::try_parse_identifer()
{
    if (std::find(m_keywords.begin(), m_keywords.end(), get_token().lex_value) != m_keywords.end())
    {
        if (get_token().lex_value == "public")
        {
            m_current_mod = Modifiers::PUBLIC;
            advance_token();
        }
        if (get_token().lex_value == "private")
        {
            m_current_mod = Modifiers::PRIVATE;
            advance_token();
        }
        if (get_token().lex_value == "static")
        {
            has_static = true;
            advance_token();
        }
        if (get_token().lex_value == "class")
        {
            return std::move(parse_class_definition());
        }
    }

    if (m_tokens[m_current_token - 1].type == TokenType::OPERATOR && m_tokens[m_current_token - 1].lex_value == "=")
    {
        printf("Parsing varibale expression %s\n", get_token().lex_value.c_str());
        return std::move(parse_variable_expression());
    }
    else if (m_tokens[m_current_token + 1].type == TokenType::LP)
    {
        printf("Parsing method call %s\n", get_token().lex_value.c_str());
        return std::move(parse_method_call());
    }
    else if (m_tokens[m_current_token + 2].type == TokenType::LP)
    {
        printf("Parsing method %s %ld\n", get_token().lex_value.c_str(), m_current_token);
        return std::move(parse_method());
    }

    return std::move(parse_variable_declaration());
}

std::unique_ptr<VariableExpression> ASTBuilder::parse_variable_expression()
{
    auto result = get_token().lex_value;
    advance_token();
    return std::make_unique<VariableExpression>(result);
}

std::unique_ptr<MethodDefinitionExpression> ASTBuilder::parse_method()
{

    if (get_token().type != TokenType::IDENTIFIER)
        return nullptr;

    std::string ret_type = get_token().lex_value;
    advance_token();

    if (get_token().type != TokenType::IDENTIFIER)
        return nullptr;

    m_current_class_name = get_token().lex_value;
    advance_token();

    advance_token(); // Eat (

    std::map<std::string, std::string> args = {};

    while (get_token().type != TokenType::RP)
    {
        std::string class_name = get_token().lex_value;
        advance_token();
        std::string arg_name = get_token().lex_value;
        advance_token();

        args[class_name] = arg_name;

        // fixme 22/10/03: hmm
        if (get_token().type == TokenType::COMMA)
            advance_token();
    }

    advance_token(); // Eat )
    advance_token(); // Eat {

    std::vector<std::unique_ptr<Expression>> body = {};

    while (get_token().type != TokenType::RCP)
    {
        auto expression = parse_expression();
        body.emplace_back(std::move(expression));
    }

    advance_token(); // Eat }

    return std::make_unique<MethodDefinitionExpression>(m_current_class_name, ret_type, m_current_mod, has_static, std::move(body), args);
}

int i = 0;

std::unique_ptr<Expression> ASTBuilder::parse_primary()
{
    switch (get_token().type)
    {
    case TokenType::NUMBER:
        return parse_number_literal();
    case TokenType::IDENTIFIER:
        return try_parse_identifer();
    case TokenType::STRING:
        return parse_string_literal();
    default:
        i++;
        if (i > 10)
            exit(1);
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
        if (get_token().type == TokenType::END_OF_FILE)
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

std::string ASTBuilder::to_json_str()
{
    return m_expressions[0].get()->to_str().dump(4);
}