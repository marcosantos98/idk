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
    ASSIGN = 5,
    ARRAY = 6,
    ASSIGN_ARRAY = 7,
};

struct MethodExpr;

struct ArrayDef
{
    String arr_type;
    Value arr_size;
    size_t alloc_sz = 0;
};

enum class AssignDefType
{
    VAL,
    BINOP,
};

struct BinopDef
{
    Value left;
    Value rigth;
    String op;
};

struct AssignArrayDef
{
    String alias;
    Value element_index;
    Value val;
    BinopDef binop;
    AssignDefType type;
};

struct AssignDef
{
    AssignDefType type;
    String alias;
    Value val;
    BinopDef binop;
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

enum class FunArgType
{
    VAL,
    BINOP,
};

struct FunArg
{
    Value val;
    BinopDef binop;
    FunArgType type;
};

struct FuncallDef
{
    Vec<FunArg> args;
    String ret_type;
    String call_name;
};

enum class VariableTypeDef
{
    VAL,
    BINOP,
    ARRAY,
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
    BinopDef binop;
    ArrayDef array;
    VariableTypeDef type;
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
    AssignDef assign_def;
    ArrayDef array_def;
    AssignArrayDef assign_array_def;
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
    String root_path = ".";
    bool single_file = false;
    Map<String, ClassDef> project_classes = {};
};

static inline String val_type_to_str(ValueType type)
{
    switch (type)
    {
    case ValueType::NUMBER:
        return "NUMBER";
    case ValueType::STRING:
        return "STRING";
    case ValueType::BOOL:
        return "BOOL";
    case ValueType::VAR_REF:
        return "VAR_REF";
    case ValueType::ARRAY_VAR_REF:
        return "ARRAY_VAR_REF";
    default:
        printf("Not implemented! %s\n", __FUNCTION__);
    }
}