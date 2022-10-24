#pragma once

#include "nava.hpp"
#include <json.hpp>

class Expression
{
public:
    virtual ~Expression() = default;

    virtual nlohmann::json to_json() = 0;
};

class ValueExpression : public Expression
{
public:
    String p_value;
    ValueType p_type;
    ValueExpression(String val, ValueType type)
        : p_value(val), p_type(type) {}

    virtual Json to_json() override;
};

class BinaryExpression : public Expression
{
public:
    String p_op;
    OwnPtr<Expression> p_lhs;
    OwnPtr<Expression> p_rhs;

    BinaryExpression(String op, OwnPtr<Expression> left, OwnPtr<Expression> right)
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

class WhileExpression : public Expression
{
public:
    OwnPtr<Expression> p_condition;
    OwnPtrVec<Expression> p_body;
    WhileExpression(OwnPtr<Expression> condition, OwnPtrVec<Expression> body)
        : p_condition(move(condition)), p_body(move(body)) {}

    virtual Json to_json() override;
};

class ClassExpression : public Expression
{
public:
    NAVA::Definition p_definition;
    OwnPtrVec<Expression> p_class_variables;
    OwnPtrVec<Expression> p_methods;
    OwnPtr<Expression> p_package;

    ClassExpression(NAVA::Definition def, OwnPtrVec<Expression> class_variables, OwnPtrVec<Expression> methods, OwnPtr<Expression> package)
        : p_definition(def), p_class_variables(move(class_variables)), p_methods(move(methods)), p_package(move(package)) {}

    virtual nlohmann::json to_json() override;
};

class PackageExpression : public Expression
{
public:
    String p_path;

    PackageExpression(String path)
        : p_path(path) {}

    virtual Json to_json() override;
};

class AssignExpression : public Expression
{
    public:
        String p_alias;
        OwnPtr<Expression> p_value;
    
    AssignExpression(String alias, OwnPtr<Expression> value)
        : p_alias(alias), p_value(move(value)) {}

    virtual Json to_json() override;
};

class AssignArrayExpression : public Expression
{
    public:
        String p_alias;
        OwnPtr<ValueExpression> p_ele_index;
        OwnPtr<Expression> p_value;
    
    AssignArrayExpression(String alias,  OwnPtr<ValueExpression> ele_index, OwnPtr<Expression> value)
        : p_alias(alias), p_ele_index(move(ele_index)), p_value(move(value)) {}

    virtual Json to_json() override;
};

class NewArrayExpression : public Expression {
    public:
        String p_array_type;
        OwnPtr<ValueExpression> p_array_size;
    
    NewArrayExpression(String array_type, OwnPtr<ValueExpression> array_size)
        : p_array_type(array_type), p_array_size(move(array_size)) {}

    virtual Json to_json() override;
};