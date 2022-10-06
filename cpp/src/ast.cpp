#include "ast.hpp"

void AST::parse()
{
    NAVA::Definition class_def = parse_class_definition(true);

    m_current_token++; // advance {

    OwnPtrVec<Expression> class_expressions = {};
    OwnPtrVec<Expression> method_expressions = {};

    while (get_token().type != TokenType::RCP)
    {
        auto expression = parse_expression();

        if (dynamic_cast<const VariableDeclarationExpression *>(expression.get()) != nullptr)
            class_expressions.emplace_back(std::move(expression));
        else if (dynamic_cast<const MethodExpression *>(expression.get()) != nullptr)
            method_expressions.emplace_back(std::move(expression));
    }

    m_current_token++;

    m_root_class = std::make_unique<ClassExpression>(class_def, std::move(class_expressions), std::move(method_expressions));

    auto &val = m_root_class;

    printf("%s\n", val.get()->to_json().dump(4).c_str());
}

OwnPtr<Expression> AST::parse_primary()
{
    switch (get_token().type)
    {
    case TokenType::NUMBER:
        return parse_number_literal_expression();
    case TokenType::IDENTIFIER:
    case TokenType::BASE_TYPE:
    case TokenType::MODIFIER:
        return try_parse_identifier_or_base_type();
    case TokenType::STRING:
        return parse_string_literal_expression();
    case TokenType::LP:
        return parse_parentisis_expression();
    default:
        printf("Token not handled: %s\n", get_token().lex_value.c_str());
        exit(1);
        return nullptr;
    }
}

OwnPtr<Expression> AST::parse_expression()
{
    auto lhs = parse_primary();
    if (!lhs)
        return nullptr;
    return parse_binary_right_side(0, move(lhs));
}

OwnPtr<NumberLiteralExpression> AST::parse_number_literal_expression()
{
    double val = std::stod(get_token().lex_value);
    m_current_token++;
    return std::make_unique<NumberLiteralExpression>(val);
}

OwnPtr<StringLiteralExpression> AST::parse_string_literal_expression()
{
    String val = get_token().lex_value;
    m_current_token++;
    return std::make_unique<StringLiteralExpression>(val);
}

OwnPtr<VariableExpression> AST::parse_variable_expression()
{
    String val = get_token().lex_value;
    m_current_token++;
    return std::make_unique<VariableExpression>(val);
}

OwnPtr<Expression> AST::parse_parentisis_expression()
{
    m_current_token++;
    auto expr = parse_expression();
    m_current_token++;
    return std::move(expr);
}

OwnPtr<Expression> AST::parse_binary_right_side(int precedence, OwnPtr<Expression> lhs)
{
    while (true)
    {
        if (get_token().type == TokenType::END_OF_FILE)
            return lhs;

        char op = get_token().lex_value[0];

        int current_precedence = get_token_precedence();
        if (current_precedence < precedence)
            return lhs;

        m_current_token++;

        auto rhs = parse_primary();
        if (!rhs)
            return nullptr;

        int next_precedence = get_token_precedence();
        if (current_precedence < next_precedence)
        {
            rhs = parse_binary_right_side(current_precedence + 1, std::move(rhs));
            if (!rhs)
                return nullptr;
        }

        lhs = std::make_unique<BinaryExpression>(op, std::move(lhs), std::move(rhs));
    }
}

OwnPtr<VariableDeclarationExpression> AST::parse_variable_declaration_expression(NAVA::Definition def)
{
    m_current_token++; // Advance =

    OwnPtr<Expression> value = nullptr;

    // fixme 22/10/06: Maybe check if the variable was declared before
    if (get_token().type == TokenType::IDENTIFIER)
        value = parse_variable_expression();
    else
        value = parse_expression();

    // fixme 22/10/05: This is not a good option. Maybe remove semicolon?
    m_current_token++; // Advance ;

    return std::make_unique<VariableDeclarationExpression>(def, std::move(value));
}

OwnPtr<MethodExpression> AST::parse_method_expression(NAVA::Definition def)
{

    m_current_token++; // Eat (

    Vec<NAVA::Definition> args = {};

    while (get_token().type != TokenType::RP)
    {
        NAVA::Definition def;

        while (get_token().type != TokenType::BASE_TYPE && get_token().type != TokenType::IDENTIFIER)
        {
            do_if_token_lex_pred_and_advance("final", [&]()
                                             { def.mod.is_final = true; });
        }

        def.class_name = get_token().lex_value;
        m_current_token++;
        def.arg_name = get_token().lex_value;
        m_current_token++;

        args.emplace_back(def);

        // fixme 22/10/05: Implement compiler logger;
        if (get_token().type != TokenType::COMMA && get_token().type != TokenType::RP)
        {
            printf("Expected [, or )] after a argument name.");
            exit(1);
        }
        else if (get_token().type == TokenType::COMMA)
            m_current_token++;
    }

    m_current_token++; // Eat ()
    m_current_token++; // Eat {

    OwnPtrVec<Expression> body = {};

    while (get_token().type != TokenType::RCP)
    {
        auto expression = parse_expression();
        body.emplace_back(std::move(expression));
    }

    m_current_token++; // Eat }

    return std::make_unique<MethodExpression>(def, args, std::move(body));
}

OwnPtr<CallExpression> AST::parse_call_expression()
{
    String name = get_token().lex_value;
    m_current_token++;
    m_current_token++; // Eat (

    OwnPtrVec<Expression> args = {};

    while (get_token().type != TokenType::RP)
    {
        auto val = parse_expression();
        args.emplace_back(std::move(val));

        // fixme 22/10/05: Implement compiler logger;
        if (get_token().type != TokenType::COMMA && get_token().type != TokenType::RP)
        {
            printf("Expected [, or )] after a argument name.");
            exit(1);
        }
        else if (get_token().type == TokenType::COMMA)
        {
            m_current_token++;
        }
    }

    m_current_token++; // Eat )
    m_current_token++; // Eat ;

    return std::make_unique<CallExpression>(name, std::move(args));
}

OwnPtr<Expression> AST::try_parse_identifier_or_base_type()
{
    // fixme 22/10/06: I dont like this approach but it works.
    if (get_token().type == TokenType::IDENTIFIER && m_tokens[m_current_token + 1].type == TokenType::LP)
        return parse_call_expression();

    NAVA::Definition def = parse_temp_definition();

    if (m_tokens[def.end].type == TokenType::OPERATOR && m_tokens[def.end].lex_value == "=")
    {
        m_current_token = def.end;
        return parse_variable_declaration_expression(def);
    }
    else if (m_tokens[def.end].type == TokenType::LP)
    {
        m_current_token = def.end;
        return parse_method_expression(def);
    }
    else
    {
        return parse_variable_expression();
    }
}

NAVA::Definition AST::parse_class_definition(bool is_class_root)
{
    // fixme 22/10/05: Don't allow static for class root
    (void)is_class_root;

    NAVA::Definition def;

    while (get_token().type != TokenType::BASE_TYPE)
    {
        do_if_token_lex_pred_and_advance("public", [&]()
                                         { def.mod.is_public = true; });
        do_if_token_lex_pred_and_advance("final", [&]()
                                         { def.mod.is_final = true; });
        do_if_token_lex_pred_and_advance("static", [&]()
                                         { def.mod.is_static = true; });
        do_if_token_lex_pred_and_advance("abstract", [&]()
                                         { def.mod.is_abstract = true; });
    }

    // fixme 22/10/05: Fix this for enum and record
    m_current_token++;

    def.class_name = get_token().lex_value;
    m_current_token++;

    return def;
}

NAVA::Definition AST::parse_definition()
{
    NAVA::Definition def;

    while (get_token().type != TokenType::BASE_TYPE && get_token().type != TokenType::IDENTIFIER)
    {
        do_if_token_lex_pred_and_advance("public", [&]()
                                         { def.mod.is_public = true; });
        do_if_token_lex_pred_and_advance("private", [&]()
                                         { def.mod.is_public = false; });
        do_if_token_lex_pred_and_advance("final", [&]()
                                         { def.mod.is_final = true; });
        do_if_token_lex_pred_and_advance("static", [&]()
                                         { def.mod.is_static = true; });
    }

    def.class_name = get_token().lex_value;
    m_current_token++;
    def.arg_name = get_token().lex_value;
    m_current_token++;

    return def;
}

NAVA::Definition AST::parse_temp_definition()
{
    NAVA::Definition def;

    def.start = m_current_token;

    size_t tmp = m_current_token;

    while (m_tokens[tmp].type != TokenType::BASE_TYPE && m_tokens[tmp].type != TokenType::IDENTIFIER)
    {

        if (m_tokens[tmp].type == TokenType::MODIFIER && m_tokens[tmp].lex_value == "public")
        {
            def.mod.is_public = true;
            tmp++;
        }
        else if (m_tokens[tmp].type == TokenType::MODIFIER && m_tokens[tmp].lex_value == "private")
        {
            def.mod.is_public = false;
            tmp++;
        }
        else if (m_tokens[tmp].type == TokenType::MODIFIER && m_tokens[tmp].lex_value == "final")
        {
            def.mod.is_final = true;
            tmp++;
        }
        else if (m_tokens[tmp].type == TokenType::MODIFIER && m_tokens[tmp].lex_value == "static")
        {
            def.mod.is_static = true;
            tmp++;
        }
    }

    def.class_name = m_tokens[tmp].lex_value;
    tmp++;
    def.arg_name = m_tokens[tmp].lex_value;
    tmp++;

    def.end = tmp;

    return def;
}

int AST::get_token_precedence()
{
    if (m_current_token >= m_tokens.size())
        return -1;

    if (!isascii(get_token().lex_value[0]))
        return -1;

    if (!NAVA::op_precedence.contains(get_token().lex_value[0]))
        return -1;

    return NAVA::op_precedence[get_token().lex_value[0]];
}

void AST::do_if_token_lex_pred_and_advance(String lex, std::function<void()> lambda)
{
    if (get_token().lex_value == lex)
    {
        lambda();
        m_current_token++;
    }
}