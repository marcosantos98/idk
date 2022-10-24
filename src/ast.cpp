#include "ast.hpp"

#include <stdarg.h>

void AST::parse()
{

    OwnPtr<Expression> package;

    while (get_token().type != TokenType::BASE_TYPE && get_token().type != TokenType::MODIFIER)
    {
        auto expr = parse_expression();

        if (dynamic_cast<const ImportExpression *>(expr.get()) != nullptr)
            m_imports.emplace_back(std::move(expr));
        else if (dynamic_cast<const PackageExpression *>(expr.get()) != nullptr)
            package = move(expr);
    }

    NAVA::Definition class_def = parse_class_definition(true);

    advanced_with_expected(TokenType::LCP);

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

    advanced_with_expected(TokenType::RCP);

    m_root_class = std::make_unique<ClassExpression>(class_def, std::move(class_expressions), std::move(method_expressions), std::move(package));

    // auto &val = m_root_class;

    // for(auto& import : m_imports)
    //     printf("%s\n", import.get()->to_json().dump(4).c_str());

    // printf("%s\n", val.get()->to_json().dump(4).c_str());
}

OwnPtr<Expression> AST::parse_primary()
{
    switch (get_token().type)
    {
    case TokenType::NUMBER:
    case TokenType::STRING:
        return parse_value_expression();
    case TokenType::IDENTIFIER:
    case TokenType::BASE_TYPE:
    case TokenType::MODIFIER:
        return try_parse_identifier_or_base_type();
    case TokenType::LP:
        return parse_parentisis_expression();
    default:
        log_error("Token not handled: %s\n", get_token().lex_value.c_str());
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

OwnPtr<ImportExpression> AST::parse_import_expression()
{
    advanced_if_identifier("import"); // Eat import

    String path = "";

    while (get_token().type != TokenType::SEMI_COLON)
    {
        path.append(get_token().lex_value).append(".");
        m_current_token++;
    }

    advanced_with_expected(TokenType::SEMI_COLON); // Eat ;

    path = path.substr(0, path.length() - 1);

    return std::make_unique<ImportExpression>(path);
}

OwnPtr<PackageExpression> AST::parse_package_expression()
{
    advanced_if_identifier("package"); // Eat package

    String path = "";

    while (get_token().type != TokenType::SEMI_COLON)
    {
        path.append(get_token().lex_value).append(".");
        m_current_token++;
    }

    advanced_with_expected(TokenType::SEMI_COLON); // Eat ;

    path = path.substr(0, path.length() - 1);

    return std::make_unique<PackageExpression>(path);
}

OwnPtr<ValueExpression> AST::parse_value_expression()
{
    ValueType type;
    if (get_token().type == TokenType::NUMBER)
        type = ValueType::NUMBER;
    else if (get_token().type == TokenType::STRING)
        type = ValueType::STRING;
    else if (get_token().type == TokenType::IDENTIFIER)
        type = ValueType::VAR_REF;
    else
        log_error("Invalid token for a value expression.\n");
    String val = get_token().lex_value;
    m_current_token++;

    if(get_token().lex_value == "[")
    {
        val += get_token().lex_value;
        advanced_with_expected(TokenType::LB);
        val += get_token().lex_value;
        m_current_token++; //Value
        val += get_token().lex_value;
        advanced_with_expected(TokenType::RB);
        type = ValueType::ARRAY_VAR_REF;
    }

    return std::make_unique<ValueExpression>(val, type);
}

OwnPtr<NewArrayExpression> AST::parse_new_array_expression()
{
    advanced_with_expected(TokenType::IDENTIFIER);
    auto array_type = get_token().lex_value;
    advanced_with_expected(TokenType::BASE_TYPE);
    advanced_with_expected(TokenType::LB);
    auto array_size = parse_value_expression();
    advanced_with_expected(TokenType::RB);
    return std::make_unique<NewArrayExpression>(array_type, move(array_size));
}

OwnPtr<ValueExpression> AST::parse_bool_expression()
{
    String val = get_token().lex_value;
    m_current_token++;
    return std::make_unique<ValueExpression>(val, ValueType::BOOL);
}

OwnPtr<AssignExpression> AST::parse_asign_expression()
{
    String val = get_token().lex_value;
    advanced_with_expected(TokenType::IDENTIFIER);
    advanced_with_expected(TokenType::OPERATOR); // fixme 22/10/23: check operator
    auto value = parse_expression();
    advanced_with_expected(TokenType::SEMI_COLON);
    return std::make_unique<AssignExpression>(val, move(value));
}

OwnPtr<AssignArrayExpression> AST::parse_assign_array_expression()
{
    String identifier = get_token().lex_value;
    advanced_with_expected(TokenType::IDENTIFIER);
    advanced_with_expected(TokenType::LB);
    auto index = parse_value_expression();
    advanced_with_expected(TokenType::RB);
    advanced_with_expected(TokenType::OPERATOR); // fixme 22/10/24: check operator =
    auto value = parse_expression();
    advanced_with_expected(TokenType::SEMI_COLON);
    return std::make_unique<AssignArrayExpression>(identifier, move(index), move(value));
}

OwnPtr<ValueExpression> AST::parse_variable_expression()
{
    String val = get_token().lex_value;
    m_current_token++;
    return std::make_unique<ValueExpression>(val, ValueType::VAR_REF);
}

OwnPtr<Expression> AST::parse_parentisis_expression()
{
    advanced_with_expected(TokenType::LP);
    auto expr = parse_expression();
    advanced_with_expected(TokenType::RP);
    return expr;
}

OwnPtr<Expression> AST::parse_binary_right_side(int precedence, OwnPtr<Expression> lhs)
{
    while (true)
    {
        if (get_token().type == TokenType::END_OF_FILE)
            return lhs;

        auto op = get_token().lex_value;

        int current_precedence = get_token_precedence();
        if (current_precedence < precedence)
            return lhs;

        advanced_with_expected(TokenType::OPERATOR); // fixme 22/10/23: check Operator

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
    advanced_with_expected(TokenType::OPERATOR); // Check operator
    OwnPtr<Expression> value = parse_expression();
    advanced_with_expected(TokenType::SEMI_COLON, __FUNCTION__);
    return std::make_unique<VariableDeclarationExpression>(def, std::move(value));
}

OwnPtr<IfExpression> AST::parse_if_expression()
{
    advanced_if_identifier("if");
    advanced_with_expected(TokenType::LP);
    auto parse = parse_expression();
    advanced_with_expected(TokenType::RP);

    OwnPtrVec<Expression> body = {};
    if (get_token().type == TokenType::LCP)
    {
        advanced_with_expected(TokenType::LCP);
        while (get_token().type != TokenType::RCP)
        {
            auto expression = parse_expression();
            body.emplace_back(std::move(expression));
        }

        advanced_with_expected(TokenType::RCP);
    }
    else
    {
        auto b = parse_expression();
        body.emplace_back(move(b));
    }

    return std::make_unique<IfExpression>(move(parse), move(body));
}

OwnPtr<WhileExpression> AST::parse_while_expression()
{
    advanced_if_identifier("while");
    advanced_with_expected(TokenType::LP);

    auto parse = parse_expression();

    advanced_with_expected(TokenType::RP);

    OwnPtrVec<Expression> body = {};
    if (get_token().type == TokenType::LCP)
    {
        advanced_with_expected(TokenType::LCP);
        while (get_token().type != TokenType::RCP)
        {
            auto expression = parse_expression();
            body.emplace_back(std::move(expression));
        }

        advanced_with_expected(TokenType::RCP);
    }
    else
    {
        auto b = parse_expression();
        body.emplace_back(move(b));
    }

    return std::make_unique<WhileExpression>(move(parse), move(body));
}

OwnPtr<MethodExpression> AST::parse_method_expression(NAVA::Definition def)
{

    advanced_with_expected(TokenType::LP);

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

        if (get_token().type != TokenType::COMMA && get_token().type != TokenType::RP)
        {
            log_error("Expected [, or )] after a argument name.\n");
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
    advanced_with_expected(TokenType::IDENTIFIER);
    advanced_with_expected(TokenType::LP);

    OwnPtrVec<Expression> args = {};

    while (get_token().type != TokenType::RP)
    {
        auto val = parse_expression();
        args.emplace_back(move(val));

        if (get_token().type != TokenType::COMMA && get_token().type != TokenType::RP)
        {
            log_error("Expected [, or )] after a argument name.");
        }
        else if (get_token().type == TokenType::COMMA)
        {
            m_current_token++;
        }
    }

    advanced_with_expected(TokenType::RP);
    advanced_with_expected(TokenType::SEMI_COLON);

    return std::make_unique<CallExpression>(name, move(args));
}

OwnPtr<Expression> AST::try_parse_identifier_or_base_type()
{
    // fixme 22/10/07: Add keywords to remove them from identifiers.
    if (get_token().type == TokenType::IDENTIFIER && get_token().lex_value == "if")
        return parse_if_expression();

    if (get_token().type == TokenType::IDENTIFIER && get_token().lex_value == "new")
        return parse_new_array_expression();

    if (get_token().type == TokenType::IDENTIFIER && m_tokens[m_current_token + 1].lex_value == "=")
        return parse_asign_expression();

    if (get_token().type == TokenType::IDENTIFIER && m_tokens[m_current_token + 1].lex_value == "[")
    {
        if (m_tokens[m_current_token + 4].type == TokenType::OPERATOR)
            return parse_assign_array_expression();
        else return parse_value_expression();
    }

    if (get_token().type == TokenType::IDENTIFIER && get_token().lex_value == "while")
        return parse_while_expression();

    if (get_token().type == TokenType::IDENTIFIER && (get_token().lex_value == "true" || get_token().lex_value == "false"))
        return parse_bool_expression();

    // fixme 22/10/06: I dont like this approach but it works.
    if (get_token().type == TokenType::IDENTIFIER && m_tokens[m_current_token + 1].type == TokenType::LP)
        return parse_call_expression();

    if (get_token().type == TokenType::IDENTIFIER && get_token().lex_value == "import")
        return parse_import_expression();
    else if (get_token().type == TokenType::IDENTIFIER && get_token().lex_value == "package")
        return parse_package_expression();

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
        if (m_tokens[m_current_token + 1].type == TokenType::OPERATOR)
        {
            return parse_variable_math_expression();
        }
        else
        {
            return parse_variable_expression();
        }
    }
}

OwnPtr<Expression> AST::parse_variable_math_expression()
{
    auto left = std::make_unique<ValueExpression>(get_token().lex_value, ValueType::VAR_REF);
    m_current_token++;
    if (get_token().lex_value == "++")
    {
        m_current_token++;
        m_current_token++;
        auto right = std::make_unique<ValueExpression>(std::to_string(1), ValueType::NUMBER);
        return std::make_unique<BinaryExpression>("++", move(left), move(right));
    }
    else if (get_token().lex_value == "--")
    {
        m_current_token++;
        m_current_token++;
        auto right = std::make_unique<ValueExpression>(std::to_string(1), ValueType::NUMBER);
        return std::make_unique<BinaryExpression>("--", move(left), move(right));
    }
    else
    {
        return left;
    }

    log_error("This is a AST bug.\n");
    return nullptr;
}

NAVA::Definition AST::parse_class_definition(bool is_class_root)
{
    NAVA::Definition def;

    while (get_token().type != TokenType::BASE_TYPE && get_token().type != TokenType::IDENTIFIER)
    {
        auto val = get_token().lex_value;

        if (val == "public")
        {
            def.mod.is_public = true;
            m_current_token++;
        }
        else if (val == "static")
        {
            if (is_class_root)
                log_error("Root class can't be marked static.\n");
            def.mod.is_static = true;
            m_current_token++;
        }
        else if (val == "abstract")
        {
            if (def.mod.is_final)
                log_error("Class marked as final can't be marked abstract.\n");
            def.mod.is_abstract = true;
            m_current_token++;
        }
        else if (val == "final")
        {
            if (def.mod.is_abstract)
                log_error("Class marked as abstract can't be marked final.\n");
            def.mod.is_final = true;
            m_current_token++;
        }
    }

    if (get_token().type != TokenType::BASE_TYPE)
        log_error("Expected base type. [class, enum, record].\n");

    m_current_token++;

    if (get_token().type != TokenType::IDENTIFIER)
        log_error("Expected identifier for the class name.\n");

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

    if (m_tokens[tmp].type == TokenType::LB && m_tokens[tmp + 1].type == TokenType::RB)
    {
        def.class_name += "[]";
        tmp += 2;
    }

    def.arg_name = m_tokens[tmp].lex_value;
    tmp++;

    def.end = tmp;

    return def;
}

void AST::advanced_with_expected(TokenType type, const char *fun)
{
    if (get_token().type == type)
        m_current_token++;
    else
        log_error("Expected %s but found %s with lexed [%s] %s\n",
                  Tokenizer::tokentype_to_token(type).c_str(),
                  Tokenizer::tokentype_to_token(get_token().type).c_str(),
                  get_token().lex_value.c_str(),
                  fun);
}

void AST::advanced_if_identifier(String const &identifider, const char *fun)
{
    if (get_token().type == TokenType::IDENTIFIER && get_token().lex_value == identifider)
        m_current_token++;
    else
        log_error("Expected IDENTIFIER with %s but found %s with lexed [%s] %s\n",
                  identifider.c_str(),
                  Tokenizer::tokentype_to_token(get_token().type).c_str(),
                  fun);
}

int AST::get_token_precedence()
{
    if (m_current_token >= m_tokens.size())
        return -1;

    if (!isascii(get_token().lex_value[0]))
        return -1;

    if (!NAVA::op_precedence.contains(get_token().lex_value))
        return -1;

    return NAVA::op_precedence[get_token().lex_value];
}

void AST::do_if_token_lex_pred_and_advance(String lex, std::function<void()> lambda)
{
    if (get_token().lex_value == lex)
    {
        lambda();
        m_current_token++;
    }
}

void AST::log_error(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    printf("\u001b[1m\u001b[31m[AST:%s]%ld:%ld:\u001b[0m ", m_file_path.c_str(), get_token().row, get_token().col);
    printf("%s", buffer);
    exit(1);
}