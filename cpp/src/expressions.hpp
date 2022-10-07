#pragma once

#include "nava.hpp"
#include <json.hpp>

class Expression
{
public:
    virtual ~Expression() = default;

    virtual nlohmann::json to_json() = 0;
};

class NumberLiteralExpression : public Expression
{
public:
    double p_value;
    NAVA::NumberType p_type;
    NumberLiteralExpression(double val, NAVA::NumberType type)
        : p_value(val), p_type(type) {}

    virtual Json to_json() override;
};

class StringLiteralExpression : public Expression
{
public:
    String p_value;
    StringLiteralExpression(String val)
        : p_value(val) {}

    virtual Json to_json() override;
};

class BinaryExpression : public Expression
{
public:
    char p_op;
    OwnPtr<Expression> p_lhs;
    OwnPtr<Expression> p_rhs;

    BinaryExpression(char op, OwnPtr<Expression> left, OwnPtr<Expression> right)
        : p_op(op), p_lhs(move(left)), p_rhs(move(right)) {}

    virtual Json to_json() override;
};

class VariableExpression : public Expression
{
public:
    String p_var_name;
    VariableExpression(String var)
        : p_var_name(var) {}

    virtual Json to_json() override;
};

class VariableDeclarationExpression : public Expression
{
public:
    NAVA::Definition p_definition;
    OwnPtr<Expression> p_value;

    VariableDeclarationExpression(NAVA::Definition def, OwnPtr<Expression> val)
        : p_definition(def), p_value(move(val)){};

    virtual nlohmann::json to_json() override;
};

class MethodExpression : public Expression
{
public:
    NAVA::Definition p_definition;
    Vec<NAVA::Definition> p_args;
    OwnPtrVec<Expression> p_body;

    MethodExpression(NAVA::Definition def, Vec<NAVA::Definition> args, OwnPtrVec<Expression> body)
        : p_definition(def), p_args(args), p_body(move(body)) {}

    virtual nlohmann::json to_json() override;
};

class CallExpression : public Expression
{
public:
    String p_method_name;
    OwnPtrVec<Expression> p_args;

    CallExpression(String method_name, OwnPtrVec<Expression> args)
        : p_method_name(method_name), p_args(move(args)) {}

    virtual Json to_json() override;
};

class ImportExpression : public Expression
{
public:
    String p_path;

    ImportExpression(String path)
        : p_path(path) {}

    virtual Json to_json() override;
};

class IfExpression : public Expression
{
public:
    OwnPtr<Expression> p_condition;
    OwnPtrVec<Expression> p_body;
    IfExpression(OwnPtr<Expression> condition, OwnPtrVec<Expression> body)
        : p_condition(move(condition)), p_body(move(body)) {}

    virtual Json to_json() override;
};

class ClassExpression : public Expression
{
public:
    NAVA::Definition p_definition;
    OwnPtrVec<Expression> p_class_variables;
    OwnPtrVec<Expression> p_methods;

    ClassExpression(NAVA::Definition def, OwnPtrVec<Expression> class_variables, OwnPtrVec<Expression> methods)
        : p_definition(def), p_class_variables(move(class_variables)), p_methods(move(methods)) {}

    virtual nlohmann::json to_json() override;
};