#include "expressions.hpp"

Json NumberLiteralExpression::to_json()
{
    Json json;
    json["type"] = "NumberLiteralExpression";
    // fixme 22/10/07: This is done because only supported this two.
    json["number_type"] = p_type == NAVA::NumberType::INT ? "int" : "double";
    json["value"] = p_value;
    return json;
}

String NumberLiteralExpression::code_gen(NAVA::GlobalContext *ctx)
{
    char buf[1024];
    if (p_type == NAVA::NumberType::INT)
    {
        if (ctx->var_ctx == NAVA::VarContext::DATA)
        {
            snprintf(buf, 1024, "\t%s_%s$int dd %d\n", ctx->class_name.c_str(), ctx->current_def.arg_name.c_str(), (int)p_value);
            ctx->data_section.append(buf);
            ctx->data_vars.emplace_back(ctx->class_name + "_" + ctx->current_def.arg_name + "$int");
        }
        else if (ctx->var_ctx == NAVA::VarContext::STACK)
        {
            ctx->current_stack_offset += 4;
            snprintf(buf, 1024, "\tmov dword [rbp-%ld], %d\n", ctx->current_stack_offset, (int)p_value);
            ctx->stack_vars[ctx->current_def.arg_name] = {ctx->current_stack_offset, "int"};
            ctx->text_section.append(buf);
        }
    }
    else
    {
        if (ctx->var_ctx == NAVA::VarContext::DATA)
        {
            snprintf(buf, 1024, "\t%s_%s$double dq %f\n", ctx->class_name.c_str(), ctx->current_def.arg_name.c_str(), p_value);
            ctx->data_section.append(buf);
            ctx->data_vars.emplace_back(ctx->class_name + "_" + ctx->current_def.arg_name + "$double");
        }
        else if (ctx->var_ctx == NAVA::VarContext::STACK)
        {
            ctx->current_stack_offset += 8;
            snprintf(buf, 1024, "\tmov qword [rbp-%ld], %f\n", ctx->current_stack_offset, p_value);
            ctx->stack_vars[ctx->current_def.arg_name] = {ctx->current_stack_offset, "double"};
            ctx->text_section.append(buf);
        }
    }

    return "";
}

Json StringLiteralExpression::to_json()
{
    Json json;
    json["type"] = "StringLiteralExpression";
    json["value"] = p_value;
    return json;
}

String StringLiteralExpression::code_gen(NAVA::GlobalContext *ctx)
{
    char buf[1024];
    snprintf(buf, 1024, "\t%s_%s db \"%s\", 10\n", ctx->current_def.class_name.c_str(), ctx->current_def.arg_name.c_str(), p_value.c_str());
    ctx->data_section.append(buf);
    return "";
}

Json BinaryExpression::to_json()
{
    Json json;
    json["type"] = "BinaryExpression";
    String op = {p_op};
    json["operator"] = op;
    json["lhs"] = p_lhs.get()->to_json();
    json["rhs"] = p_rhs.get()->to_json();
    return json;
}

String BinaryExpression::code_gen(NAVA::GlobalContext *ctx)
{

    if (ctx->current_def.class_name == "int")
    {
        if (auto left_number = dynamic_cast<const NumberLiteralExpression *>(p_lhs.get()))
        {
            char buf[1024];
            snprintf(buf, 1024, "\tmov eax, %d\n", (int)left_number->p_value);
            ctx->text_section.append(buf);
        }
        else if (auto left_var = dynamic_cast<const VariableExpression *>(p_lhs.get()))
        {
            char buf[1024];
            snprintf(buf, 1024, "\tmov eax, dword [rbp-%ld]\n", ctx->stack_vars[left_var->p_var_name].offset);
            ctx->text_section.append(buf);
        }

        if (auto right_number = dynamic_cast<const NumberLiteralExpression *>(p_rhs.get()))
        {
            char buf[1024];
            snprintf(buf, 1024, "\tmov edx, %d\n", (int)right_number->p_value);
            ctx->text_section.append(buf);
        }
        else if (auto right_var = dynamic_cast<const VariableExpression *>(p_rhs.get()))
        {
            char buf[1024];
            snprintf(buf, 1024, "\tmov edx, dword [rbp-%ld]\n", ctx->stack_vars[right_var->p_var_name].offset);
            ctx->text_section.append(buf);
        }

        if (p_op == '+')
        {
            ctx->text_section.append("\tadd eax, edx\n");
            char buf[1024];
            ctx->current_stack_offset += 4;
            snprintf(buf, 1024, "\tmov dword [rbp-%ld], eax\n", ctx->current_stack_offset);
            ctx->text_section.append(buf);
            ctx->stack_vars[ctx->current_def.arg_name] = {ctx->current_stack_offset, "int"};
        }
        else if (p_op == '-')
        {
            ctx->text_section.append("\tsub eax, edx\n");
            char buf[1024];
            ctx->current_stack_offset += 4;
            snprintf(buf, 1024, "\tmov dword [rbp-%ld], eax\n", ctx->current_stack_offset);
            ctx->text_section.append(buf);
            ctx->stack_vars[ctx->current_def.arg_name] = {ctx->current_stack_offset, "int"};
        }
        else if (p_op == '*')
        {
            ctx->text_section.append("\tmul edx\n");
            char buf[1024];
            ctx->current_stack_offset += 4;
            snprintf(buf, 1024, "\tmov dword [rbp-%ld], eax\n", ctx->current_stack_offset);
            ctx->text_section.append(buf);
            ctx->stack_vars[ctx->current_def.arg_name] = {ctx->current_stack_offset, "int"};
        }
        else if (p_op == '/')
        {
            ctx->text_section.append("\tmov ecx, edx\n");
            ctx->text_section.append("\tmov edx, 0\n");
            ctx->text_section.append("\tdiv ecx\n");
            char buf[1024];
            ctx->current_stack_offset += 4;
            snprintf(buf, 1024, "\tmov dword [rbp-%ld], eax\n", ctx->current_stack_offset);
            ctx->text_section.append(buf);
            ctx->stack_vars[ctx->current_def.arg_name] = {ctx->current_stack_offset, "int"};
        }
        else if (p_op == '%')
        {
            ctx->text_section.append("\tmov ecx, edx\n");
            ctx->text_section.append("\tmov edx, 0\n");
            ctx->text_section.append("\tdiv ecx\n");
            ctx->text_section.append("\tmov eax, edx\n");
            char buf[1024];
            ctx->current_stack_offset += 4;
            snprintf(buf, 1024, "\tmov dword [rbp-%ld], eax\n", ctx->current_stack_offset);
            ctx->text_section.append(buf);
            ctx->stack_vars[ctx->current_def.arg_name] = {ctx->current_stack_offset, "int"};
        }
        else if (p_op == '!=')
        {
            ctx->text_section.append("\tsetne al\n");
            ctx->text_section.append("\tmovsx eax, al\n");
            char buf[1024];
            ctx->current_stack_offset += 4;
            snprintf(buf, 1024, "\tmov dword [rbp-%ld], eax\n", ctx->current_stack_offset);
            ctx->text_section.append(buf);
            ctx->stack_vars[ctx->current_def.arg_name] = {ctx->current_stack_offset, "int"};
        }
    }

    return "";
}

Json VariableExpression::to_json()
{
    Json json;

    json["type"] = "VariableExpression";
    json["var_name"] = p_var_name;

    return json;
}

String VariableExpression::code_gen(NAVA::GlobalContext *ctx)
{
    char buf[1024];
    snprintf(buf, 1024, "mov eax, %d", 0);
    return {buf};
}

Json VariableDeclarationExpression::to_json()
{
    Json json = def_to_json(p_definition);

    json["type"] = "VariableDeclarationExpression";
    json["value"] = p_value.get()->to_json();

    return json;
}

String VariableDeclarationExpression::code_gen(NAVA::GlobalContext *ctx)
{

    ctx->current_def = p_definition;
    p_value.get()->code_gen(ctx);
    return "";
}

Json MethodExpression::to_json()
{
    Json json = def_to_json(p_definition);

    json["type"] = "MethodExpression";

    Json args_arr = nlohmann::json::array();

    for (auto arg : p_args)
        args_arr.emplace_back(def_to_json(arg));

    json["args"] = args_arr;

    Json body_arr = nlohmann::json::array();

    for (auto &expr : p_body)
        body_arr.emplace_back(expr->to_json());

    json["body"] = body_arr;

    return json;
}

String MethodExpression::code_gen(NAVA::GlobalContext *ctx)
{
    ctx->text_section.append("global ").append(ctx->class_name).append("$").append(p_definition.arg_name).append("\n");
    ctx->text_section.append(ctx->class_name).append("$").append(p_definition.arg_name).append(":\n");
    ctx->text_section.append("\tpush rbp\n");
    ctx->text_section.append("\tmov rbp, rsp\n");

    bool has_calls = false;
    for (auto &body : p_body)
    {
        if (dynamic_cast<const CallExpression *>(body.get()) != nullptr)
        {
            has_calls = true;
            break;
        }
    }

    if (has_calls)
        ctx->text_section.append("\tsub rsp, 16\n");

    for (auto &body : p_body)
    {
        body.get()->code_gen(ctx);
    }

    if (has_calls)
    {
        ctx->text_section.append("\tleave\n");
    }
    else
    {
        ctx->text_section.append("\tpop rbp\n");
        ctx->text_section.append("\tret\n");
    }

    ctx->text_section.append("\tret\n");

    return "";
}

Json CallExpression::to_json()
{
    Json json;

    json["type"] = "CallExpression";

    Json args = nlohmann::json::array();

    for (auto &arg : p_args)
    {
        args.emplace_back(arg.get()->to_json());
    }

    json["args"] = args;

    json["method_name"] = p_method_name;

    return json;
}

String CallExpression::code_gen(NAVA::GlobalContext *ctx)
{
    Vec<String> arg_reg_64 = {
        "rdi",
        "rsi",
        "rdx",
        "r10",
        "r8",
        "r9",
    };

    Vec<String> arg_reg_32 = {
        "edi",
        "esi",
        "edx",
        "r10d",
        "r8d",
        "r9d",
    };

    int index = 0;

    if (p_method_name == "asm")
    {
        // fixme 22/10/06: Check for only one arg or maybe parse this individually instead of working as a call expression.
        if (auto str_val = dynamic_cast<const StringLiteralExpression *>(p_args[0].get()))
        {
            ctx->text_section.append(str_val->p_value);
        }
    }
    else
    {

        if (p_args.size() > 0 && p_args.size() < 6)
        {
            for (auto &arg : p_args)
            {
                if (auto number = dynamic_cast<const NumberLiteralExpression *>(arg.get()))
                {
                    if (number->p_type == NAVA::NumberType::INT)
                    {
                        char buf[1024];
                        snprintf(buf, 1024, "\tmov %s, %d\n", arg_reg_32[index].c_str(), (int)number->p_value);
                        ctx->text_section.append(buf);
                    }
                    else if (number->p_type == NAVA::NumberType::DOUBLE)
                    {
                        char buf[1024];
                        snprintf(buf, 1024, "\tmov %s, %d\n", arg_reg_64[index].c_str(), (int)number->p_value);
                        ctx->text_section.append(buf);
                    }
                }
                else if (auto string = dynamic_cast<const StringLiteralExpression *>(arg.get()))
                {
                    NAVA::Definition tmp = {
                        .arg_name = ""+std::to_string(ctx->tmp_str_cnt),
                        .class_name = "tmp_str",
                    };
                    ctx->current_def = tmp;
                    ctx->tmp_str_cnt++;
                    arg.get()->code_gen(ctx);
                    char buf[1024];
                    snprintf(buf, 1024, "\tmov %s, %s\n", arg_reg_64[index].c_str(), (tmp.class_name + "_" + tmp.arg_name).c_str());
                    ctx->text_section.append(buf);
                }
                else if (auto variable = dynamic_cast<const VariableExpression *>(arg.get()))
                {
                    if (ctx->stack_vars.contains(variable->p_var_name))
                    {
                        if (ctx->stack_vars[variable->p_var_name].class_name == "int")
                        {
                            char buf[1024];
                            snprintf(buf, 1024, "\tmov %s, [rbp-%ld]\n", arg_reg_32[index].c_str(), ctx->stack_vars[variable->p_var_name].offset);
                            ctx->text_section.append(buf);
                        }
                        else
                        {
                            char buf[1024];
                            snprintf(buf, 1024, "\tmov %s, [rbp-%ld]\n", arg_reg_64[index].c_str(), ctx->stack_vars[variable->p_var_name].offset);
                            ctx->text_section.append(buf);
                        }
                    }
                    else
                    {
                        // fixme 22/10/09: Report errror
                    }
                }
                index++;
            }
            index = 0;
        }
        String s = p_method_name;
        replace(s.begin(), s.end(), '.', '$');
        if (std::find(ctx->externs.begin(), ctx->externs.end(), s) == ctx->externs.end())
        {
            ctx->extern_section.append("extern ").append(s).append("\n");
            ctx->externs.emplace_back(s);
        }
        ctx->text_section.append("\tcall ").append(s).append("\n");
    }

    return "";
}

Json ImportExpression::to_json()
{
    Json json;

    json["type"] = "ImportExpression";
    json["path"] = p_path;

    return json;
}

String ImportExpression::code_gen(NAVA::GlobalContext *ctx)
{
    char buf[1024];
    snprintf(buf, 1024, "mov eax, %d", 0);
    return {buf};
}

Json IfExpression::to_json()
{
    Json json;

    json["type"] = "IfExpression";
    json["condition"] = p_condition.get()->to_json();

    Json body_arr = nlohmann::json::array();

    for (auto &expr : p_body)
        body_arr.emplace_back(expr->to_json());

    json["body"] = body_arr;

    return json;
}

String IfExpression::code_gen(NAVA::GlobalContext *ctx)
{
    if(auto number = dynamic_cast<const NumberLiteralExpression*>(p_condition.get()))
    {
        char buf[1024];
        snprintf(buf, 1024, "\tmov eax, %d\n\tcmp eax, 1\n", (int)number->p_value);
        ctx->text_section.append(buf);
    } 
    else if(auto binop = dynamic_cast<const BinaryExpression*>(p_condition.get()))
    {
        p_condition.get()->code_gen(ctx);
    }

    ctx->text_section.append("\tjne ").append(".JL_").append(std::to_string(ctx->label_cnt)).append("\n");

    for(auto &body : p_body)
    {
        body.get()->code_gen(ctx);
    }

    ctx->text_section.append(".JL_").append(std::to_string(ctx->label_cnt++)).append(":\n");

    return "";
}

Json ClassExpression::to_json()
{
    Json json = def_to_json(p_definition);

    json["type"] = "ClassExpression";

    Json variables = nlohmann::json::array();

    for (auto &variable : p_class_variables)
        variables.emplace_back(variable->to_json());

    json["variables"] = variables;

    Json methods = nlohmann::json::array();

    for (auto &method : p_methods)
        methods.emplace_back(method->to_json());

    json["methods"] = methods;

    return json;
}

String ClassExpression::code_gen(NAVA::GlobalContext *ctx)
{
    char buf[1024];
    snprintf(buf, 1024, "mov eax, %d", 0);
    return {buf};
}