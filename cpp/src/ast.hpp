#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "tokenizer.hpp"

class Expression
{
public:
    virtual ~Expression() = default;

    virtual std::string to_str() = 0;
};

class NumberLiteral : public Expression
{
public:
    double value;
    NumberLiteral(double val) : value(val) {}

    std::string to_str() override
    {
        return "Number Literal: " + std::to_string(value);
    }
};

class StringLiteral : public Expression
{
public:
    std::string value;
    StringLiteral(std::string val) : value(val) {}

    std::string to_str() override
    {
        return "String Literal: " + value;
    }
};

class VariableDeclarationExpression : public Expression
{
public:
    std::string class_name;
    std::string variable_name;
    std::unique_ptr<Expression> value;
    VariableDeclarationExpression(std::string name, std::string var_name, std::unique_ptr<Expression> val)
        : class_name(name), variable_name(var_name), value(std::move(val)) {}

    std::string to_str() override
    {
        return "Variable Declaration: Class[" + class_name + "], VarName[" + variable_name + "], Value[" + value.get()->to_str() + "]";
    }
};

class BinaryExpression : public Expression
{
public:
    char op;
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;

    BinaryExpression(char op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : op(op), lhs(std::move(left)), rhs(std::move(right)) {}

    std::string to_str() override
    {
        return "BinaryExpression: LHS[" + lhs.get()->to_str() + "], Operand[" + op + "], RHS[" + rhs.get()->to_str() + "]";
    }
};

class ASTBuilder
{
public:
    ASTBuilder(std::vector<Token> tokens) : m_tokens(tokens) {}

    void run();

    void print_ast();

private:
    std::map<char, int> m_op_precedence = {
        {'+', 20},
        {'-', 20},
        {'%', 30},
        {'*', 30},
        {'/', 30},
    };

    std::vector<Token> m_tokens;
    std::vector<std::unique_ptr<Expression>> m_expressions = {};
    u_int64_t m_current_token = 0;

    void advance_token();
    int get_token_precedence();

    std::unique_ptr<Expression> parse_primary();
    std::unique_ptr<Expression> parse_expression();
    std::unique_ptr<NumberLiteral> parse_number_literal();
    std::unique_ptr<StringLiteral> parse_string_literal();
    std::unique_ptr<VariableDeclarationExpression> parse_variable_declaration();
    std::unique_ptr<Expression> parse_binary_right(int, std::unique_ptr<Expression>);

    Token get_token() const
    {
        return m_current_token >= m_tokens.size() ? m_tokens[m_tokens.size() - 1] : m_tokens[m_current_token];
    }
};