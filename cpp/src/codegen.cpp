#include "codegen.hpp"

#include <algorithm>
#include <algorithm>

struct StackInfo
{
    String class_name;
    size_t offset;
};

void CodeGenerator::run()
{
    bool has_main = false;

    for (auto &method : m_root_class.get()->p_methods)
    {
        if (auto method_ = dynamic_cast<const MethodExpression *>(method.get()))
        {
            if (method_->p_definition.arg_name == "main" && method_->p_definition.class_name == "void" && method_->p_definition.mod.is_static && method_->p_definition.mod.is_public)
                has_main = true;
        }
    }

    // for (auto &importt : m_imports)
    // {
    //     if (auto import = dynamic_cast<const ImportExpression *>(importt.get()))
    //     {
    //         m_final_out.append("%include \"").append(import->p_path).append(".asm").append("\"\n");
    //     }
    // }

    if (has_main)
        m_text_section.append("global _start\n");

    auto *class_def = m_root_class.get();

    parse_class_variables();
    parse_class_methods();

    if (has_main)
    {
        m_text_section.append("_start:\n");

        m_text_section.append("\tcall ").append(class_def->p_definition.class_name).append("$main\n");

        m_text_section.append("\tmov rax, 60\n");
        m_text_section.append("\tmov rdi, 0\n");
        m_text_section.append("\tsyscall\n");
    }

    m_final_out.append(m_externs);
    m_final_out.append(m_text_section).append("\n");
    m_final_out.append(m_data_section).append("\n");
}

void CodeGenerator::parse_class_variables()
{
    auto *class_def = m_root_class.get();

    size_t j = 0;
    while (j < class_def->p_class_variables.size())
    {
        auto current_variable = static_cast<const VariableDeclarationExpression *>(class_def->p_class_variables[j].get());
        std::string var_name = class_def->p_definition.class_name + "_" + current_variable->p_definition.arg_name;
        // global_variables.emplace_back(current_variable->p_definition.arg_name);

        if (auto str_literal = dynamic_cast<const StringLiteralExpression *>(current_variable->p_value.get()))
        {
            m_data_section.append("\t").append(var_name).append(" db ").append("\"").append(str_literal->p_value).append("\"").append(", 0\n");
            m_data_section.append("\tsz_").append(var_name).append(" equ $-").append(var_name).append("\n");
        }
        else if (auto binop = dynamic_cast<const BinaryExpression *>(current_variable->p_value.get()))
        {
            if (binop->p_op == '+')
            {
                auto lhs = static_cast<const NumberLiteralExpression *>(binop->p_lhs.get());
                auto rhs = static_cast<const NumberLiteralExpression *>(binop->p_rhs.get());

                if (lhs && rhs)
                {
                    double val = lhs->p_value + rhs->p_value;
                    if (current_variable->p_definition.class_name == "int")
                    {
                        int int_val = (int)val;
                        m_data_section.append("\t").append(var_name).append(" dd ").append(std::to_string(int_val)).append("\n");
                    }
                    else
                    {
                        m_data_section.append("\t").append(var_name).append(" dd ").append(std::to_string(val)).append("\n");
                    }
                }
            }
        }
        else if (auto number_literal = dynamic_cast<const NumberLiteralExpression *>(current_variable->p_value.get()))
        {
            auto val = number_literal->p_value;
            if (current_variable->p_definition.class_name == "int")
            {
                m_data_section.append("\t").append(var_name).append(" dd ").append(std::to_string((int)val)).append("\n");
                m_class_vars[current_variable->p_definition.arg_name] = var_name;
            }
        }
        j++;
    }
}

void CodeGenerator::parse_class_methods()
{
    size_t j = 0;

    auto *class_def = m_root_class.get();

    while (j < class_def->p_methods.size())
    {

        size_t current_offset = 0;

        auto current_method = static_cast<const MethodExpression *>(class_def->p_methods[j].get());

        m_text_section.append("global ")
            .append(class_def->p_definition.class_name)
            .append("$")
            .append(current_method->p_definition.arg_name).append("\n");

        m_text_section.append(class_def->p_definition.class_name).append("$").append(current_method->p_definition.arg_name).append(":\n");

        m_text_section.append("\tpush rbp\n");
        m_text_section.append("\tmov rbp, rsp\n");

        size_t k = 0;
        std::map<String, StackInfo> stack = {};

        bool has_calls = false;

        String body_text = "";

        while (k < current_method->p_body.size())
        {

            if (auto var = dynamic_cast<const VariableDeclarationExpression *>(current_method->p_body[k].get()))
            {
                if (auto number_literal = dynamic_cast<const NumberLiteralExpression *>(var->p_value.get()))
                {
                    if (var->p_definition.class_name == "int")
                    {
                        current_offset += 4;
                        body_text.append("\tmov dword [rbp-")
                            .append(std::to_string(current_offset))
                            .append("], ")
                            .append(std::to_string((int)number_literal->p_value))
                            .append("\n");
                        stack[var->p_definition.arg_name] = {var->p_definition.class_name, current_offset};
                    }
                }
                else if (auto binop = dynamic_cast<const BinaryExpression *>(var->p_value.get()))
                {
                    if (binop->p_op == '+')
                    {
                        if (auto lhs_number = dynamic_cast<const NumberLiteralExpression *>(binop->p_lhs.get()))
                        {
                            current_offset += 4;
                            body_text.append("\tmov eax, ")
                                .append(std::to_string((int)lhs_number->p_value))
                                .append("\n");
                        }
                        else if (auto lhs_var = dynamic_cast<const VariableExpression *>(binop->p_lhs.get()))
                        {
                            if (stack.contains(lhs_var->p_var_name))
                            {
                                body_text.append("\tmov eax, dword [rbp-")
                                    .append(std::to_string(stack[lhs_var->p_var_name].offset))
                                    .append("]\n");
                            }
                        }

                        if (auto rhs_number = dynamic_cast<const NumberLiteralExpression *>(binop->p_rhs.get()))
                        {
                            current_offset += 4;
                            body_text.append("\tmov edx, ")
                                .append(std::to_string((int)rhs_number->p_value))
                                .append("\n");
                        }
                        else if (auto rhs_var = dynamic_cast<const VariableExpression *>(binop->p_rhs.get()))
                        {
                            if (stack.contains(rhs_var->p_var_name))
                            {
                                body_text.append("\tmov edx, dword [rbp-")
                                    .append(std::to_string(stack[rhs_var->p_var_name].offset))
                                    .append("]\n");
                            }
                        }

                        body_text.append("\tadd eax, edx\n");
                        current_offset += 4;
                        body_text.append("\tmov dword [rbp-")
                            .append(std::to_string(current_offset))
                            .append("], eax\n");
                        stack[var->p_definition.arg_name] = {var->p_definition.class_name, current_offset};
                    }
                }
                else if (auto varibale = dynamic_cast<const VariableExpression *>(var->p_value.get()))
                {
                    // if (std::find(global_variables.begin(), global_variables.end(), varibale->p_var_name) != global_variables.end())
                    // {
                    //     m_text_section.append("\t").append("push qword [").append(class_def->p_definition.class_name + "_" + varibale->p_var_name).append("]\n");
                    // }
                }
            }
            else if (auto call = dynamic_cast<const CallExpression *>(current_method->p_body[k].get()))
            {
                has_calls = true;
                if (call->p_method_name == "asm")
                {
                    // fixme 22/10/06: Check for only one arg or maybe parse this individually instead of working as a call expression.
                    if (auto str_val = dynamic_cast<const StringLiteralExpression *>(call->p_args[0].get()))
                    {
                        body_text.append(str_val->p_value);
                    }
                }
                else
                {   
                   
                    if (call->p_args.size() > 0 && call->p_args.size() < 6)
                    {
                        for (auto &arg : call->p_args)
                        {
                            // fixme 22/10/07: Expressions
                            if (auto number = dynamic_cast<const NumberLiteralExpression *>(arg.get()))
                            {
                                // fixme 22/10/07: Provide number type in expression
                                body_text.append("\tmov rdi, ")
                                    .append(std::to_string((int)number->p_value))
                                    .append("\n");
                            }
                            else if (auto varibleaa = dynamic_cast<const VariableExpression *>(arg.get()))
                            {
                                if (stack.find(varibleaa->p_var_name) != stack.end())
                                {
                                    auto reg = stack[varibleaa->p_var_name].class_name == "int" ? "edi" : "rdi";
                                    body_text.append("\tmov ")
                                        .append(reg)
                                        .append(", [rbp-")
                                        .append(std::to_string(stack[varibleaa->p_var_name].offset))
                                        .append("]\n");
                                }
                            }
                        }
                    }
                    String s = call->p_method_name;
                    replace(s.begin(), s.end(), '.', '$');
                    m_externs.append("extern ").append(s).append("\n");
                    body_text.append("\tcall ")
                        .append(s)
                        .append("\n");
                }
            }
            k++;
        }
        if (has_calls)
        {
            m_text_section.append("\tsub rsp, 16\n");
            m_text_section.append(body_text).append("\n");
            m_text_section.append("\tleave\n");
        }
        else
        {
            m_text_section.append(body_text).append("\n");
            m_text_section.append("\tpop rbp\n");
        }

        m_text_section.append("\tret\n");
        j++;
    }
}