#include "project_parser.hpp"
#include "tokenizer.hpp"
#include "ast.hpp"

void ProjectParser::parse(String file_path, String source_file)
{
    Tokenizer tokenizer(source_file, file_path);
    tokenizer.run();

    AST ast(file_path, tokenizer.get_tokens());
    ast.parse();

    auto ast_root = ast.get_root_class();

    // fixme 22/10/25: Parse imports

    ClassDef class_def;

    Vec<VariableDef> class_vars = {};
    Vec<MethodDef> class_methods = {};

    for (auto &variable : ast_root.get()->p_class_variables)
    {
        auto variable_def = parse_variable_ast(as<VariableDeclarationExpression>(variable.get()));
        m_global_variables.emplace_back(variable_def);
        class_vars.emplace_back(variable_def);
    }

    for (auto &method : ast_root.get()->p_methods)
        class_methods.emplace_back(parse_method_ast(as<MethodExpression>(method.get())));

    class_def.class_variables = class_vars;
    class_def.class_methods = class_methods;
    class_def.imports = {};
    class_def.in_file = file_path;

    m_project->project_classes["$Test"] = class_def;
}

VariableDef ProjectParser::parse_variable_ast(VariableDeclarationExpression const *variable_declaration)
{
    VariableDef def;

    def.class_name = variable_declaration->p_definition.class_name;
    def.arg_name = variable_declaration->p_definition.arg_name;
    def.is_final = variable_declaration->p_definition.mod.is_final;
    def.is_static = variable_declaration->p_definition.mod.is_static;
    def.is_public = variable_declaration->p_definition.mod.is_public;
    def.is_protected = false; // fixme 22/10/25: Protected

    if (is<BinaryExpression>(variable_declaration->p_value.get()))
    {
        def.type = VariableTypeDef::BINOP;
        def.binop = parse_binop_ast(as<BinaryExpression>(variable_declaration->p_value.get()));
    }
    else if (is<ValueExpression>(variable_declaration->p_value.get()))
    {
        def.type = VariableTypeDef::VAL;
        def.val = parse_value_ast(as<ValueExpression>(variable_declaration->p_value.get()));
    }
    else if (is<NewArrayExpression>(variable_declaration->p_value.get()))
    {
        def.type = VariableTypeDef::ARRAY;
        def.array = parse_array_ast(as<NewArrayExpression>(variable_declaration->p_value.get()));
    }
    else
        logger->log_terr("Parse variable expression not implement yet!\n");

    return def;
}

MethodDef ProjectParser::parse_method_ast(MethodExpression const *method_expr)
{
    MethodDef def;
    def.is_public = method_expr->p_definition.mod.is_public;
    def.is_abstract = method_expr->p_definition.mod.is_abstract;
    def.is_static = method_expr->p_definition.mod.is_static;
    def.method_name = method_expr->p_definition.arg_name;
    def.return_type = method_expr->p_definition.class_name;

    Vec<MethodExpr> body_expressions = {};

    for (auto &body_expr : method_expr->p_body)
    {
        body_expressions.emplace_back(parse_as_method_expr(body_expr.get(), body_expr.get()->to_json()["type"].dump()));
    }

    def.method_expressions = body_expressions;

    return def;
}

BinopDef ProjectParser::parse_binop_ast(BinaryExpression const *binop)
{
    BinopDef def;

    auto left = static_cast<const ValueExpression *>(binop->p_lhs.get());

    Value left_val;
    left_val.raw = left->p_value;
    left_val.type = left->p_type;

    auto right = static_cast<const ValueExpression *>(binop->p_rhs.get());

    Value right_val;
    right_val.raw = right->p_value;
    right_val.type = right->p_type;

    def.left = left_val;
    def.rigth = right_val;

    def.op = binop->p_op;

    return def;
}

Value ProjectParser::parse_value_ast(ValueExpression const *value_expr)
{
    Value val;
    val.raw = value_expr->p_value;
    val.type = value_expr->p_type;
    return val;
}

ArrayDef ProjectParser::parse_array_ast(NewArrayExpression const *array_expr)
{
    ArrayDef def;

    def.arr_type = array_expr->p_array_type;

    if (is<ValueExpression>(array_expr->p_array_size.get()))
    {
        auto value = parse_value_ast(as<ValueExpression>(array_expr->p_array_size.get()));

        if (value.type != ValueType::NUMBER && value.type != ValueType::VAR_REF && value.type != ValueType::ARRAY_VAR_REF)
            logger->log_terr("Array construction requires a NUMBER, VAR_REF/ARRAY_VAR_REF to a NUMBER. Got %s!\n", val_type_to_str(value.type).c_str());

        def.arr_size = value;

        if (value.type == ValueType::NUMBER)
        {
            def.alloc_sz = NAVA::primitive_byte_sizes[def.arr_type] * value.as_int();
        }
        else
            logger->log_terr("Alloc Size not implement to Value types other than NUMBER literal.\n");
    }
    else
        logger->log_terr("Probably an AST bug. array_type can only be a ValueExpression.\n");

    return def;
}

FuncallDef ProjectParser::parse_funcall_ast(CallExpression const *call_expr)
{
    FuncallDef def;
    def.call_name = call_expr->p_method_name;
    def.ret_type = "void"; // fixme 22/10/15: Implement this.

    Vec<FunArg> arguments = {};

    for (auto &arg : call_expr->p_args)
    {
        if (is<ValueExpression>(arg.get()))
        {
            FunArg fun_arg;
            fun_arg.type = FunArgType::VAL;
            auto value = parse_value_ast(as<ValueExpression>(arg.get()));
            fun_arg.val = value;
            arguments.emplace_back(fun_arg);
        }
        else
            logger->log_terr("Function arg %s not implemented yet!\n", arg.get()->to_json()["type"].dump().c_str());
    }

    def.args = arguments;

    return def;
}

AssignDef ProjectParser::parse_assign_ast(AssignExpression const *assign_expr)
{
    AssignDef def;

    def.alias = assign_expr->p_alias;

    if (is<BinaryExpression>(assign_expr->p_value.get()))
    {
        auto binop = parse_binop_ast(as<BinaryExpression>(assign_expr->p_value.get()));

        def.type = AssignDefType::BINOP;
        def.binop = binop;
    }
    else if (is<ValueExpression>(assign_expr->p_value.get()))
    {
        auto value = parse_value_ast(as<ValueExpression>(assign_expr->p_value.get()));

        def.type = AssignDefType::VAL;
        def.val = value;
    }
    else
        logger->log_terr("Assign value %s not implemented yet!\n", assign_expr->p_value.get()->to_json()["type"].dump().c_str());

    return def;
}

AssignArrayDef ProjectParser::parse_assign_array_ast(AssignArrayExpression const *assign_array_expr)
{
    AssignArrayDef def;
    def.alias = assign_array_expr->p_alias;

    auto element_index = parse_value_ast(as<ValueExpression>(assign_array_expr->p_ele_index.get()));

    if (element_index.type != ValueType::NUMBER && element_index.type != ValueType::VAR_REF && element_index.type != ValueType::ARRAY_VAR_REF)
        logger->log_terr("Array construction requires a NUMBER, VAR_REF/ARRAY_VAR_REF to a NUMBER. Got %s!\n", val_type_to_str(element_index.type).c_str());

    def.element_index = element_index;

    MethodExpr assign_value_expr;

    if (is<ValueExpression>(assign_array_expr->p_value.get()))
    {
        auto value = parse_value_ast(as<ValueExpression>(assign_array_expr->p_value.get()));
        def.type = AssignDefType::VAL;
        def.val = value;
    }
    else
        logger->log_terr("Assign value %s not implemented yet!\n", assign_array_expr->p_value.get()->to_json()["type"].dump().c_str());

    return def;
}

IfDef ProjectParser::parse_if_ast(IfExpression const *if_expr)
{
    IfDef def;

    if (is<ValueExpression>(if_expr->p_condition.get()))
    {
        auto value = parse_value_ast(as<ValueExpression>(if_expr->p_condition.get()));

        CondDef cond_def;
        cond_def.val = value;

        def.cond = cond_def;
        def.type = CondType::VAL;
    }
    else if (is<BinaryExpression>(if_expr->p_condition.get()))
    {
        auto binop = parse_binop_ast(as<BinaryExpression>(if_expr->p_condition.get()));

        CondDef cond_def;
        cond_def.binop = binop;

        def.cond = cond_def;
        def.type = CondType::BINOP;
    }
    else
        logger->log_terr("IF condition type %s not implemented yet!\n", if_expr->p_condition.get()->to_json()["type"].dump().c_str());

    Vec<MethodExpr> body = {};

    for (auto &exp : if_expr->p_body)
    {
        body.emplace_back(parse_as_method_expr(exp.get(), exp.get()->to_json()["type"].dump()));
    }

    def.body_expr = body;

    return def;
}

WhileDef ProjectParser::parse_while_ast(WhileExpression const *while_expr)
{
    WhileDef def;

    if (is<ValueExpression>(while_expr->p_condition.get()))
    {
        auto value = parse_value_ast(as<ValueExpression>(while_expr->p_condition.get()));

        CondDef cond_def;
        cond_def.val = value;

        def.cond = cond_def;
        def.type = CondType::VAL;
    }
    else if (is<BinaryExpression>(while_expr->p_condition.get()))
    {
        auto binop = parse_binop_ast(as<BinaryExpression>(while_expr->p_condition.get()));

        CondDef cond_def;
        cond_def.binop = binop;

        def.cond = cond_def;
        def.type = CondType::BINOP;
    }
    else
        logger->log_terr("WHILE condition type %s not implemented yet!\n", while_expr->p_condition.get()->to_json()["type"].dump().c_str());

    Vec<MethodExpr> body = {};

    for (auto &exp : while_expr->p_body)
    {
        body.emplace_back(parse_as_method_expr(exp.get(), exp.get()->to_json()["type"].dump()));
    }

    def.body_expr = body;

    return def;
}

MethodExpr ProjectParser::parse_as_method_expr(Expression const *expr, String type)
{
    MethodExpr method_expr;

    if (is<VariableDeclarationExpression>(expr))
    {
        auto var_def = parse_variable_ast(as<VariableDeclarationExpression>(expr));
        method_expr.type = MethodExprType::VAR;

        StackVar stack_var;
        stack_var.alias = var_def.arg_name;
        stack_var.stack_offset = 0;

        method_expr.var_def = std::make_tuple(var_def, stack_var);
    }
    else if (is<CallExpression>(expr))
    {
        auto call_def = parse_funcall_ast(as<CallExpression>(expr));
        method_expr.type = MethodExprType::FUNCALL;

        method_expr.func_def = call_def;
    }
    else if (is<AssignExpression>(expr))
    {
        auto assign_def = parse_assign_ast(as<AssignExpression>(expr));
        method_expr.type = MethodExprType::ASSIGN;

        method_expr.assign_def = assign_def;
    }
    else if (is<AssignArrayExpression>(expr))
    {
        auto assign_array_def = parse_assign_array_ast(as<AssignArrayExpression>(expr));
        method_expr.type = MethodExprType::ASSIGN_ARRAY;

        method_expr.assign_array_def = assign_array_def;
    }
    else if (is<IfExpression>(expr))
    {
        auto if_def = parse_if_ast(as<IfExpression>(expr));
        method_expr.type = MethodExprType::IF;

        method_expr.if_def = if_def;
    }
    else if (is<WhileExpression>(expr))
    {
        auto while_def = parse_while_ast(as<WhileExpression>(expr));
        method_expr.type = MethodExprType::WHILE;

        method_expr.while_def = while_def;
    }
    else if (is<BinaryExpression>(expr))
    {
        auto binop_def = parse_binop_ast(as<BinaryExpression>(expr));
        method_expr.type = MethodExprType::BINOP;

        method_expr.binop_def = binop_def;
    }
    else
        logger->log_terr("MethodExpr: expression %s not implemented yet!\n", type.c_str());

    return method_expr;
}