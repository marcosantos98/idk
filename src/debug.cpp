#include "debug.hpp"

String tabs(int tabs)
{
    String tmp;
    int i = 0;
    while (i < tabs)
    {
        tmp += "    ";
        i++;
    }
    return tmp;
}

void print_val(Value val, int tabs_cnt)
{
    printf("%sType:%-15s Raw:%-10s\n",
           tabs(tabs_cnt).c_str(),
           val_type_to_str(val.type).c_str(),
           val.raw.c_str());
}

void print_stack(StackVar stack_var, int tabs_cnt)
{
    printf("%sAlias: %s Offset: %ld\n",
           tabs(tabs_cnt).c_str(),
           stack_var.alias.c_str(),
           stack_var.stack_offset);
}

void print_var_val(VariableDef var_def, int tabs_cnt)
{
    printf("%sType: %-10s Name:%-10s Class:%-10s ValueType:%-15s ValueRaw:%-10s\n",
           tabs(tabs_cnt).c_str(),
           "VAR_VAL",
           var_def.arg_name.c_str(),
           var_def.class_name.c_str(),
           val_type_to_str(var_def.val.type).c_str(),
           var_def.val.raw.c_str());
}

void print_binop(BinopDef binop, int tabs_cnt)
{
    printf("%sLeftRaw:%-5s Operand:%-5s RightRaw:%-5s\n",
           tabs(tabs_cnt).c_str(),
           binop.left.raw.c_str(),
           binop.op.c_str(),
           binop.rigth.raw.c_str());
}

void print_var_binop(VariableDef var_def, int tabs_cnt)
{
    printf("%sType: %-10s Name:%-10s Class:%-10s\n",
           tabs(tabs_cnt).c_str(),
           "VAR_BINOP",
           var_def.arg_name.c_str(),
           var_def.class_name.c_str());
    print_binop(var_def.binop, tabs_cnt + 1);
}

void print_var_array(VariableDef var_def, int tabs_cnt)
{
    printf("%sType: %-10s Name:%-10s Class:%-10s\n",
           tabs(tabs_cnt).c_str(),
           "VAR_ARRAY",
           var_def.arg_name.c_str(),
           var_def.class_name.c_str());
    printf("%sType:%-10s AllocSize:%ld Size:",
           tabs(tabs_cnt + 1).c_str(),
           var_def.array.arr_type.c_str(),
           var_def.array.alloc_sz);
    if (var_def.array.arr_size.type == ValueType::ARRAY_VAR_REF || var_def.array.arr_size.type == ValueType::VAR_REF)
    {
        printf("%s\n", var_def.array.arr_size.raw.c_str());
    }
    else if (var_def.array.arr_size.type == ValueType::NUMBER)
    {
        printf("%d\n", var_def.array.arr_size.as_int());
    }
    else
        printf("ERROR: Got invalid type %d\n", static_cast<int>(var_def.val.type));
}

void print_expr_var(MethodExpr var, int tabs_cnt)
{
    if (std::get<0>(var.var_def.value()).type == VariableTypeDef::VAL)
    {
        print_var_val(std::get<0>(var.var_def.value()), tabs_cnt);
        print_stack(std::get<1>(var.var_def.value()), tabs_cnt + 1);
    }
    else if (std::get<0>(var.var_def.value()).type == VariableTypeDef::BINOP)
    {
        print_var_binop(std::get<0>(var.var_def.value()), tabs_cnt);
        print_stack(std::get<1>(var.var_def.value()), tabs_cnt + 1);
    }
    else if (std::get<0>(var.var_def.value()).type == VariableTypeDef::ARRAY)
    {
        print_var_array(std::get<0>(var.var_def.value()), tabs_cnt);
        print_stack(std::get<1>(var.var_def.value()), tabs_cnt + 1);
    }
}

void print_expr_funcall(MethodExpr var, int tabs_cnt)
{
    printf("%sType: CALL_METHOD Name: %-10s\n", tabs(tabs_cnt).c_str(), var.func_def.call_name.c_str());
    for (auto val_arg : var.func_def.args)
    {

        if (var.func_def.call_name != "asm")
        {
            if (val_arg.type == FunArgType::BINOP)
            {
                print_binop(val_arg.binop, tabs_cnt + 1);
            }
            else if (val_arg.type == FunArgType::VAL)
            {
                print_val(val_arg.val, tabs_cnt + 1);
            }
        }
        else
            printf("%sArg: <assembly_code>\n", tabs(tabs_cnt + 1).c_str());
    }
}

void print_expr_assign(MethodExpr var, int tabs_cnt)
{
    printf("%sType: ASSIGN Alias:%s\n", tabs(tabs_cnt).c_str(), var.assign_def.alias.c_str());
    if (var.assign_def.type == AssignDefType::BINOP)
    {
        print_binop(var.assign_def.binop, tabs_cnt + 1);
    }
    else if (var.assign_def.type == AssignDefType::VAL)
    {
        print_val(var.assign_def.val, tabs_cnt + 1);
    }
}

void print_expr_assign_array(MethodExpr var, int tabs_cnt)
{
    printf("%sType: ASSIGN_ARRAY Alias:%s[%s] to:\n", tabs(tabs_cnt).c_str(), var.assign_array_def.alias.c_str(), var.assign_array_def.element_index.raw.c_str());
    if (var.assign_array_def.type == AssignDefType::BINOP)
    {
        print_binop(var.assign_array_def.binop, tabs_cnt + 1);
    }
    else if (var.assign_array_def.type == AssignDefType::VAL)
    {
        print_val(var.assign_array_def.val, tabs_cnt + 1);
    }
}

void print_expr_if(MethodExpr var, int tabs_cnt)
{
    if (var.if_def.type == CondType::VAL)
    {
        printf("%sType: IF\n", tabs(tabs_cnt).c_str());
        print_val(var.if_def.cond.val, tabs_cnt + 1);
    }
    else
    {
        printf("%sType: IF\n", tabs(tabs_cnt).c_str());
        print_binop(var.if_def.cond.binop, tabs_cnt + 1);
    }
    printf("%sBody:\n", tabs(tabs_cnt + 1).c_str());
    for (auto body_expr : var.if_def.body_expr)
    {
        print_expr_type(body_expr, tabs_cnt + 2);
    }
}

void print_expr_while(MethodExpr var, int tabs_cnt)
{
    if (var.while_def.type == CondType::VAL)
    {
        printf("%sType: WHILE\n", tabs(tabs_cnt).c_str());
        print_val(var.while_def.cond.val, tabs_cnt + 1);
    }
    else
    {
        printf("%sType: WHILE\n", tabs(tabs_cnt).c_str());
        print_binop(var.while_def.cond.binop, tabs_cnt + 1);
    }
    printf("%sBody:\n", tabs(tabs_cnt + 1).c_str());
    for (auto body_expr : var.while_def.body_expr)
    {
        print_expr_type(body_expr, tabs_cnt + 2);
    }
}

void print_expr_type(MethodExpr expr, int tabs_cnt)
{
    switch (expr.type)
    {
    case MethodExprType::VAR:
        print_expr_var(expr, tabs_cnt);
        break;
    case MethodExprType::FUNCALL:
        print_expr_funcall(expr, tabs_cnt);
        break;
    case MethodExprType::ASSIGN:
        print_expr_assign(expr, tabs_cnt);
        break;
    case MethodExprType::ASSIGN_ARRAY:
        print_expr_assign_array(expr, tabs_cnt);
        break;
    case MethodExprType::IF:
        print_expr_if(expr, tabs_cnt);
        break;
    case MethodExprType::WHILE:
        print_expr_while(expr, tabs_cnt);
        break;
    case MethodExprType::BINOP:
        print_binop(expr.binop_def, tabs_cnt);
        break;
    default:
        printf("Not implemented!\n");
        break;
    }
}

void print_project(Project const &project)
{
    for (auto clazz : project.project_classes)
    {
        printf("Class %s:\n", clazz.first.c_str());
        for (auto imp : clazz.second.imports)
        {
            printf("    Import:%s\n", imp.c_str());
        }
        printf("    ====> Variables <====\n\n");
        for (auto var_def : clazz.second.class_variables)
        {
            if (var_def.type == VariableTypeDef::VAL)
            {
                print_var_val(var_def, 1);
            }
            else if (var_def.type == VariableTypeDef::BINOP)
            {
                print_var_binop(var_def, 1);
            }
            else if (var_def.type == VariableTypeDef::ARRAY)
            {
                print_var_array(var_def, 1);
            }
        }
        printf("\n    ====> Methods <====\n\n");
        for (auto method_def : clazz.second.class_methods)
        {
            printf("    MethodName:%s, RetType:%s ArgCount:%ld\n",
                   method_def.method_name.c_str(),
                   method_def.return_type.c_str(),
                   method_def.args.size());
            for (auto var : method_def.method_expressions)
            {
                print_expr_type(var, 2);
            }
        }
    }
}