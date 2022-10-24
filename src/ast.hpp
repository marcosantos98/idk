#pragma once

#include "nava.hpp"
#include "tokenizer.hpp"
#include "expressions.hpp"

class AST
{
public:
    AST(String file_path, Vec<Token> tokens)
        : m_file_path(file_path), m_tokens(tokens) {}

    void parse();

    OwnPtr<ClassExpression> get_root_class()
    {
        return move(m_root_class);
    }

    OwnPtrVec<Expression> get_imports()
    {
        return move(m_imports);
    }

private:
    size_t m_current_token = 0;
    String m_file_path;

    Vec<Token> m_tokens;

    OwnPtr<ClassExpression> m_root_class;
    OwnPtrVec<Expression> m_imports = {};

    OwnPtr<Expression> parse_primary();
    OwnPtr<Expression> parse_expression();
    OwnPtr<ImportExpression> parse_import_expression();
    OwnPtr<PackageExpression> parse_package_expression();
    OwnPtr<ValueExpression> parse_value_expression();
    OwnPtr<ValueExpression> parse_variable_expression();
    OwnPtr<ValueExpression> parse_bool_expression();
    OwnPtr<NewArrayExpression> parse_new_array_expression();
    OwnPtr<AssignExpression> parse_asign_expression();
    OwnPtr<AssignArrayExpression> parse_assign_array_expression();
    OwnPtr<Expression> parse_parentisis_expression();
    OwnPtr<Expression> parse_binary_right_side(int, OwnPtr<Expression>);
    OwnPtr<VariableDeclarationExpression> parse_variable_declaration_expression(NAVA::Definition);
    OwnPtr<IfExpression> parse_if_expression();
    OwnPtr<WhileExpression> parse_while_expression();
    OwnPtr<MethodExpression> parse_method_expression(NAVA::Definition);
    OwnPtr<CallExpression> parse_call_expression();
    OwnPtr<Expression> try_parse_identifier_or_base_type();
    OwnPtr<Expression> parse_variable_math_expression();
    NAVA::Definition parse_class_definition(bool);
    NAVA::Definition parse_definition();
    NAVA::Definition parse_temp_definition();

    Token get_token() const
    {
        return m_current_token >= m_tokens.size() ? m_tokens[m_tokens.size() - 1] : m_tokens[m_current_token];
    }

    void advanced_with_expected(TokenType, const char* fun = "");
    void advanced_if_identifier(String const&, const char* fun = "");

    int get_token_precedence();
    void do_if_token_lex_pred_and_advance(String, std::function<void()>);
    void log_error(const char *, ...);
};