#pragma once

#include "global.hpp"
#include "expressions.hpp"
#include "log.hpp"

class ProjectParser
{
public:
    ProjectParser(Project *project)
        : m_project(project) {}

    ~ProjectParser() { delete logger; }

    void parse(String file_path, String source_file);

private:
    Project *m_project;

    Vec<VariableDef> m_global_variables = {};
    Vec<VariableDef> m_stack_variables = {};

    Log *logger = new Log("Project");

    VariableDef parse_variable_ast(VariableDeclarationExpression const *, bool);
    MethodDef parse_method_ast(MethodExpression const *);
    BinopDef parse_binop_ast(BinaryExpression const *);
    Value parse_value_ast(ValueExpression const *);
    ArrayDef parse_array_ast(NewArrayExpression const *);
    FuncallDef parse_funcall_ast(CallExpression const *);
    AssignDef parse_assign_ast(AssignExpression const *);
    AssignArrayDef parse_assign_array_ast(AssignArrayExpression const *);
    IfDef parse_if_ast(IfExpression const*);
    WhileDef parse_while_ast(WhileExpression const*);

    MethodExpr parse_as_method_expr(Expression const*, String);

    struct Context {
        MethodDef* current_method;
    };
    Context m_ctx;

    template <typename T, typename V>
    inline bool is(V const *val)
    {
        return dynamic_cast<const T *>(val) != nullptr;
    }

    template <typename T, typename V>
    inline T const *as(V const *val)
    {
        return dynamic_cast<const T *>(val);
    }
};