#include "debug.hpp"

void print_project(Project const &project)
{
    for (auto clazz : project.project_classes)
    {
        printf("Class %s:\n", clazz.first.c_str());
        for (auto imp : clazz.second.imports)
        {
            printf("\tImport:%s\n", imp.c_str());
        }
        for (auto var_def : clazz.second.class_variables)
        {
            printf("\tVarName:%s, VarClass:%s VarValueRaw:%s, VarValueType:%d\n",
                   var_def.arg_name.c_str(),
                   var_def.class_name.c_str(),
                   var_def.val.raw.c_str(),
                   static_cast<int>(var_def.val.type));
        }
        for (auto method_def : clazz.second.class_methods)
        {
            printf("\tMethodName:%s, RetType:%s ArgCount:%ld\n",
                   method_def.method_name.c_str(),
                   method_def.return_type.c_str(),
                   method_def.args.size());
            for (auto var : method_def.method_expressions)
            {
                if (var.type == MethodExprType::VAR)
                {
                    printf("\t\tVarName:%s, VarClass:%s VarValueRaw:%s VarValueType:%d\n",
                           std::get<0>(var.var_def.value()).arg_name.c_str(),
                           std::get<0>(var.var_def.value()).class_name.c_str(),
                           std::get<0>(var.var_def.value()).val.raw.c_str(),
                           static_cast<int>(std::get<0>(var.var_def.value()).val.type));
                    printf("\t\t\tStackVar:%s Offset:%ld\n",
                           std::get<1>(var.var_def.value()).alias.c_str(),
                           std::get<1>(var.var_def.value()).stack_offset);
                }
                else if (var.type == MethodExprType::BINOP)
                {
                    printf("\t\tVarName:%s, VarClass:%s, VarRaw:%s\n",
                           std::get<0>(var.var_def.value()).arg_name.c_str(),
                           std::get<0>(var.var_def.value()).class_name.c_str(),
                           std::get<0>(var.var_def.value()).val.raw.c_str());
                    printf("\t\t\tLeftValueRaw:%s OP:%s RightValueRaw:%s\n",
                           var.binop_def.left.raw.c_str(),
                           var.binop_def.op.c_str(),
                           var.binop_def.rigth.raw.c_str());
                    printf("\t\t\tStackVar:%s Offset:%ld\n",
                           std::get<1>(var.var_def.value()).alias.c_str(),
                           std::get<1>(var.var_def.value()).stack_offset);
                }
                else if (var.type == MethodExprType::FUNCALL)
                {
                    printf("\t\tMethodCall:%s\n", var.func_def.call_name.c_str());
                    for (auto val_arg : var.func_def.args)
                    {
                        if (var.func_def.call_name != "asm")
                            printf("\t\t\tArg:%s Type:%d\n", val_arg.raw.c_str(), static_cast<int>(val_arg.type));
                        else
                            printf("\t\t\tArg: <assembly_code>\n");
                    }
                }
                else if (var.type == MethodExprType::IF)
                {
                    if (var.if_def.type == CondType::VAL)
                        printf("\t\tIfExpressionCond:%s\n", var.if_def.cond.val.raw.c_str());
                    else
                        printf("\t\t\tIfExpressionCond: LeftValueRaw:%s OP:%s RightValueRaw:%s\n",
                               var.if_def.cond.binop.left.raw.c_str(),
                               var.if_def.cond.binop.op.c_str(),
                               var.if_def.cond.binop.rigth.raw.c_str());
                }
                else if (var.type == MethodExprType::WHILE)
                {
                    if (var.while_def.type == CondType::VAL)
                        printf("\tWhileExpressionCond:%s\n", var.while_def.cond.val.raw.c_str());
                    else
                        printf("\t\tWhileExpressionCond: LeftValueRaw:%s OP:%s RightValueRaw:%s\n",
                               var.while_def.cond.binop.left.raw.c_str(),
                               var.while_def.cond.binop.op.c_str(),
                               var.while_def.cond.binop.rigth.raw.c_str());
                    for (auto e : var.while_def.body_expr)
                    {
                        if (e.type == MethodExprType::FUNCALL)
                        {
                            printf("\t\t\tMethodCall:%s\n", e.func_def.call_name.c_str());
                            for (auto val_arg : e.func_def.args)
                            {
                                if (e.func_def.call_name != "asm")
                                    printf("\t\t\t\tArg:%s Type:%d\n", val_arg.raw.c_str(), static_cast<int>(val_arg.type));
                                else
                                    printf("\t\t\t\tArg: <assembly_code>\n");
                            }
                        }
                        else if (e.type == MethodExprType::BINOP)
                        {
                            printf("\t\t\tVarName:%s, VarClass:%s VarRaw:%s\n",
                                   std::get<0>(e.var_def.value()).arg_name.c_str(),
                                   std::get<0>(e.var_def.value()).class_name.c_str(),
                                   std::get<0>(e.var_def.value()).val.raw.c_str());
                            printf("\t\t\t\tLeftValueRaw:%s OP:%s RightValueRaw:%s\n",
                                   e.binop_def.left.raw.c_str(), e.binop_def.op.c_str(),
                                   e.binop_def.rigth.raw.c_str());
                            printf("\t\t\t\tStackVar:%s Offset:%ld\n",
                                   std::get<1>(e.var_def.value()).alias.c_str(),
                                   std::get<1>(e.var_def.value()).stack_offset);
                        }
                        else if (e.type == MethodExprType::VAR)
                        {
                            printf("\t\tVarName:%s, VarClass:%s VarValueRaw:%s VarValueType:%d\n",
                                   std::get<0>(e.var_def.value()).arg_name.c_str(),
                                   std::get<0>(e.var_def.value()).class_name.c_str(),
                                   std::get<0>(e.var_def.value()).val.raw.c_str(),
                                   static_cast<int>(std::get<0>(e.var_def.value()).val.type));
                            printf("\t\t\tStackVar:%s Offset:%ld\n",
                                   std::get<1>(e.var_def.value()).alias.c_str(),
                                   std::get<1>(e.var_def.value()).stack_offset);
                        }
                        else if (e.type == MethodExprType::ASSIGN)
                        {
                            printf("\t\t\tAssign:%s from ", e.assign_def.alias.c_str());
                            if (e.assign_def.val[0].type == MethodExprType::BINOP)
                            {
                                printf("LeftValueRaw:%s OP:%s RightValueRaw:%s\n",
                                       e.assign_def.val[0].binop_def.left.raw.c_str(), e.assign_def.val[0].binop_def.op.c_str(),
                                       e.assign_def.val[0].binop_def.rigth.raw.c_str());
                            }
                            else if (e.assign_def.val[0].type == MethodExprType::VAR)
                            {
                                printf("ValueRaw:%s VarOffset:%ld\n", std::get<0>(e.assign_def.val[0].var_def.value()).val.raw.c_str(), std::get<1>(e.assign_def.val[0].var_def.value()).stack_offset);
                            }
                        }
                    }
                }
                else if (var.type == MethodExprType::ARRAY)
                {
                    printf("\t\tVarName:%s, VarClass:%s VarValueRaw:%s VarValueType:%d\n",
                           std::get<0>(var.var_def.value()).arg_name.c_str(),
                           std::get<0>(var.var_def.value()).class_name.c_str(),
                           std::get<0>(var.var_def.value()).val.raw.c_str(),
                           static_cast<int>(std::get<0>(var.var_def.value()).val.type));
                    printf("\t\t\tStackVar:%s Offset:%ld\n",
                           std::get<1>(var.var_def.value()).alias.c_str(),
                           std::get<1>(var.var_def.value()).stack_offset);
                }
                else if (var.type == MethodExprType::ASSIGN_ARRAY)
                {
                    printf("\t\t\tAssign:%s with index %s to ", var.assign_array_def.alias.c_str(), var.assign_array_def.element_index.raw.c_str());
                    if (var.assign_array_def.val[0].type == MethodExprType::BINOP)
                    {
                        printf("LeftValueRaw:%s OP:%s RightValueRaw:%s\n",
                               var.assign_array_def.val[0].binop_def.left.raw.c_str(), var.assign_array_def.val[0].binop_def.op.c_str(),
                               var.assign_array_def.val[0].binop_def.rigth.raw.c_str());
                    }
                    else if (var.assign_array_def.val[0].type == MethodExprType::VAR)
                    {
                        printf("ValueRaw:%s VarOffset:%ld\n", std::get<0>(var.assign_array_def.val[0].var_def.value()).val.raw.c_str(), std::get<1>(var.assign_array_def.val[0].var_def.value()).stack_offset);
                    }
                }
            }
        }
    }
}