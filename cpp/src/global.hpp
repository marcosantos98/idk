#pragma once

#include <optional>
#include <tuple>

#include "nava.hpp"

struct Value
{
    String raw;
    ValueType type;
    String as_str()
    {
        return raw;
    }

    uint8_t as_bool()
    {
        return raw == "true" ? 1 : 0;
    }

    double as_double()
    {
        return std::stod(raw);
    }

    int as_int()
    {
        return std::stoi(raw);
    }

    long as_long()
    {
        return std::stol(raw);
    }
};

struct VariableDef
{
    bool is_public = true;
    bool is_static = false;
    bool is_final = false;
    bool is_protected = false;
    String class_name;
    String arg_name;
    Value val;
};

struct ArgumentDef
{
    bool is_final = false;
    String class_name;
    String arg_name;
};

struct StackVar
{
    String alias;
    size_t stack_offset;
};

enum class MethodExprType : int
{
    VAR = 0,
    FUNCALL = 1,
    BINOP = 2,
    IF = 3,
    WHILE = 4,
};

struct MethodExpr;

struct BinopDef
{
    Value left;
    Value rigth;
    String op;
};

enum class CondType
{
    VAL,
    BINOP,
};

struct CondDef
{
    Value val;
    BinopDef binop;
};

struct WhileDef
{
    CondType type;
    CondDef cond;
    Vec<MethodExpr> body_expr;
};

struct IfDef
{
    CondType type;
    CondDef cond;
    Vec<MethodExpr> body_expr;
};

struct FuncallDef
{
    Vec<Value> args;
    String ret_type;
    String call_name;
};

struct MethodExpr
{
    bool is_final = false;
    MethodExprType type;
    std::optional<std::tuple<VariableDef, StackVar>> var_def;
    FuncallDef func_def;
    BinopDef binop_def;
    IfDef if_def;
    WhileDef while_def;
};

struct MethodDef
{
    bool is_public = true;
    bool is_static = false;
    bool is_abstract = false;
    String return_type;
    String method_name;
    Vec<ArgumentDef> args;
    Vec<MethodExpr> method_expressions;
    size_t stack_offset;
};

struct ClassDef
{
    Vec<MethodDef> class_methods = {};
    Vec<VariableDef> class_variables = {};
    Vec<String> imports = {};
    String in_file;
};

struct Project
{
    String main_class = "";
    String root_path;
    bool single_file = false;
    Map<String, ClassDef> project_classes = {};
};
