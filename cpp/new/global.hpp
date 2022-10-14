#pragma once

#include <functional>

#include "nava.hpp"

struct Value
{
    String raw;
    String as_str()
    {
        return raw;
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

struct StackVar {
    String alias;
    size_t stack_offset;
};

struct MethodVar {
    bool is_final = false;
    VariableDef var_def;
    StackVar stack_var;
};

struct MethodDef
{
    bool is_public = true;
    bool is_static = false;
    bool is_abstract = false;
    String return_type;
    String method_name;
    Vec<ArgumentDef> args;
    Vec<MethodVar> stack_vars;
    size_t stack_offset;
};

struct ClassDef
{
    Vec<MethodDef> class_methods = {};
    Vec<VariableDef> class_variables = {};
    String in_file;
};

struct Project
{
    String main_class = "$GlobalVar";
    Map<String, ClassDef> project_classes = {};
};
