#pragma once

#include "nava.hpp"
#include "tokenizer.hpp"
#include "expressions.hpp"

class AST
{
public:
    AST(Vec<Token> tokens)
        : m_tokens(tokens) {}

    void parse();

private:
    size_t m_current_token = 0;

    Vec<Token> m_tokens;

    OwnPtr<ClassExpression> m_root_class;

    OwnPtr<Expression> parse_primary();
    OwnPtr<Expression> parse_expression();
    OwnPtr<NumberLiteralExpression> parse_number_literal_expression();
    OwnPtr<StringLiteralExpression> parse_string_literal_expression();
    OwnPtr<VariableExpression> parse_variable_expression();
    OwnPtr<Expression> parse_parentisis_expression();
    OwnPtr<Expression> parse_binary_right_side(int, OwnPtr<Expression>);
    OwnPtr<VariableDeclarationExpression> parse_variable_declaration_expression(NAVA::Definition);
    OwnPtr<MethodExpression> parse_method_expression(NAVA::Definition);
    OwnPtr<CallExpression> parse_call_expression();
    OwnPtr<Expression> try_parse_identifier_or_base_type();
    NAVA::Definition parse_class_definition(bool);
    NAVA::Definition parse_definition();
    NAVA::Definition parse_temp_definition();

    Token get_token() const
    {
        return m_current_token >= m_tokens.size() ? m_tokens[m_tokens.size() - 1] : m_tokens[m_current_token];
    }

    int get_token_precedence();
    void do_if_token_lex_pred_and_advance(String, std::function<void()>);
};