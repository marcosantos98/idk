#include "x86_64_nasm_linux.hpp"

#include <memory>
#include <string>
#include <stdexcept>
#include <stdarg.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "x86_64_helper.hpp"

std::string exec_cmd(const char *cmd)
{
    char buffer[128];
    std::string result = "";
    auto *pipe = popen(cmd, "r");
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    try
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL)
        {
            result += buffer;
        }
    }
    catch (...)
    {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

template <typename... Args>
String string_format(const std::string &format, Args... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    if (size_s <= 0)
    {
        throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

size_t str_cnt = 0;
Vec<String> externs = {};

void call_expression(String classs, ClassDef def, MethodDef method, MethodExpr stack, String *text_section, String *data_section, String *extern_section, Vec<String> arg_reg_32, Vec<String> arg_reg_64)
{
    if (stack.func_def.call_name == "asm")
    {
        // fixme 22/10/06: Check for only one arg or maybe parse this individually instead of working as a call expression.
        text_section->append(stack.func_def.args[0].as_str());
    }
    else
    {
        if (stack.func_def.args.size() > 0 && stack.func_def.args.size() < 6)
        {
            size_t index = 0;
            for (auto &arg : stack.func_def.args)
            {
                if (arg.type == ValueType::NUMBER)
                {
                    // fixme 22/10/16: Include primitive type instead of hardcoded int
                    mov_reg_immX(text_section, "int", arg_reg_32[index].c_str(), arg);
                }
                else if (arg.type == ValueType::STRING)
                {
                    string_format(data_section, "\ttmp_str_%ld db \"%s\", 10\n", str_cnt, arg.as_str().c_str());
                    string_format(text_section, "\tmov %s, tmp_str_%ld\n", arg_reg_64[index].c_str(), str_cnt++);
                }
                else if (arg.type == ValueType::VAR_REF)
                {
                    // Check in class_vars
                    for (auto var_def : def.class_variables)
                    {
                        if (var_def.arg_name == arg.raw)
                        {
                            // fixme 22/10/15: Check size of operand
                            mov_reg_data(text_section, arg_reg_32[index].c_str(), (classs + "_" + arg.raw));
                            break;
                        }
                    }
                    for (auto expr : method.method_expressions)
                    {
                        if (expr.type == MethodExprType::VAR || expr.type == MethodExprType::BINOP)
                        {
                            if (std::get<1>(expr.var_def).alias == arg.raw)
                            {
                                mov_reg_mX(text_section, std::get<0>(expr.var_def).class_name, std::get<1>(expr.var_def).stack_offset, arg_reg_32[index].c_str());
                                break;
                            }
                        }
                    }
                }
                index++;
            }
            index = 0;
        }

        String s = "$" + stack.func_def.call_name;
        replace(s.begin(), s.end(), '.', '$');

        if (VEC_HAS(externs, s))
            printf("");
        else
        {
            externs.emplace_back(s);
            string_format(extern_section, "extern %s\n", s.c_str());
        }

        string_format(text_section, "\tcall %s\n", s.c_str());
    }
}

String as_val_or_mem(String classpath, ClassDef def, MethodDef method, Value val)
{
    if (val.type == ValueType::NUMBER)
    {
        return string_format("%d", val.as_int());
    }
    else if (val.type == ValueType::VAR_REF)
    {
        for (auto var_def : def.class_variables)
        {
            if (var_def.arg_name == val.raw)
            {
                return string_format("%s_%s", classpath.c_str(), val.raw.c_str());
            }
        }
        for (auto expr : method.method_expressions)
        {
            if (expr.type == MethodExprType::VAR || expr.type == MethodExprType::BINOP)
            {
                if (std::get<1>(expr.var_def).alias == val.raw)
                {
                    // fixme 22/10/16: Check operand size
                    return string_format("dword [rbp-%d]", std::get<1>(expr.var_def).stack_offset);
                }
            }
        }
    }
    printf("AsValOrMem: Not implemented %d %s\n", val.type, val.raw.c_str());
    exit(1);
}

void parse_binop(String classpath, ClassDef classDef, MethodDef def, MethodExpr stack, String *text_section, bool to_stack)
{

    String left = as_val_or_mem(classpath, classDef, def, stack.binop_def.left);
    String right = as_val_or_mem(classpath, classDef, def, stack.binop_def.rigth);

    string_format(text_section, "\tmov eax, %s\n", left.c_str());

    if (stack.binop_def.op == "+")
    {
        string_format(text_section, "\tadd eax, %s\n", right.c_str());
    }
    else if (stack.binop_def.op == "-")
    {
        string_format(text_section, "\tsub eax, %s\n", right.c_str());
    }
    else if (stack.binop_def.op == "*")
    {
        string_format(text_section, "\timul eax, %s\n", right.c_str());
    }
    else if (stack.binop_def.op == "/")
    {
        text_section->append("\txor edx, edx\n");
        string_format(text_section, "\tmov ecx, %s\n", right.c_str());
        text_section->append("\tcdq\n");
        text_section->append("\tidiv ecx\n");
    }
    else if (stack.binop_def.op == "%")
    {
        text_section->append("\txor edx, edx\n");
        string_format(text_section, "\tmov ecx, %s\n", right.c_str());
        text_section->append("\tcdq\n");
        text_section->append("\tidiv ecx\n");
        text_section->append("\tmov eax, edx\n");
    }
    else if (stack.binop_def.op == "<")
    {
        if (std::get<0>(stack.var_def).class_name == "boolean")
        {
            string_format(text_section, "\tcmp eax, %s\n", right.c_str());
            text_section->append("\tsetle al\n");
            text_section->append("\tmovzx eax, al\n");
        }
        else
        {
            string_format(text_section, "\tcmp eax, %s\n", right.c_str());
        }
    }
    else if (stack.binop_def.op == ">")
    {
        if (std::get<0>(stack.var_def).class_name == "boolean")
        {
            string_format(text_section, "\tcmp eax, %s\n", right.c_str());
            text_section->append("\tsetg al\n");
            text_section->append("\tmovzx eax, al\n");
        }
        else
        {
            string_format(text_section, "\tcmp eax, %s\n", right.c_str());
        }
    }
    else if (stack.binop_def.op == "++")
        string_format(text_section, "\tinc %s\n", left.c_str());
    else if (stack.binop_def.op == "++")
        string_format(text_section, "\tdec %s\n", left.c_str());

    if (to_stack)
        mov_mX_reg(text_section, std::get<0>(stack.var_def).class_name, std::get<1>(stack.var_def).stack_offset, "eax");
}

void CodeGenerator::generate()
{
    String final_out = "";
    String global_section;
    String extern_section;
    String text_section = "section .text\n";
    String data_section = "section .data\n";

    size_t jmp_cnt = 0;

    Vec<String> obj_files = {};

    for (auto def : m_project->project_classes)
    {
        printf("Generating NASM x86_64 Linux for: %s\n", def.second.in_file.c_str());
        for (auto var : def.second.class_variables)
        {
            if (var.class_name == "int")
            {
                data_section.append(string_format("\t%s_%s dd %d\n", def.first.c_str(), var.arg_name.c_str(), var.val.as_int()));
            }
            else if (var.class_name == "double")
            {
                data_section.append(string_format("\t%s dq %1f\n", var.arg_name.c_str(), var.val.as_double()));
            }
        }

        for (auto method : def.second.class_methods)
        {
            if (method.is_static || method.is_public)
                global_section.append(string_format("global %s$%s\n", def.first.c_str(), method.method_name.c_str()));

            string_format(&text_section, "%s$%s:\n", def.first.c_str(), method.method_name.c_str());

            text_section.append("\tpush rbp\n");
            text_section.append("\tmov rbp, rsp\n");
            string_format(&text_section, "\tsub rsp, %ld\n", method.stack_offset + 10);

            // fixme 22/10/14: pop args from stack

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

            for (auto stack : method.method_expressions)
            {
                if (stack.type == MethodExprType::VAR)
                {
                    string_format(&text_section, "\tmov dword [rbp-%d], %d\n", std::get<1>(stack.var_def).stack_offset, std::get<0>(stack.var_def).val.as_int());
                }
                else if (stack.type == MethodExprType::BINOP)
                {
                    if (&std::get<0>(stack.var_def) != 0)
                        parse_binop(def.first, def.second, method, stack, &text_section, true);
                }
                else if (stack.type == MethodExprType::FUNCALL)
                {
                    call_expression(def.first, def.second, method, stack, &text_section, &data_section, &extern_section, arg_reg_32, arg_reg_64);
                }
                else if (stack.type == MethodExprType::IF)
                {
                    if (stack.if_def.type == CondType::VAL)
                    {
                        if (stack.if_def.cond.val.type == ValueType::NUMBER)
                        {
                            string_format(&text_section, "\tmov eax, %d\n", stack.if_def.cond.val.as_int());
                            text_section.append("\tcmp eax, 0\n");
                            string_format(&text_section, "\tje .JPL_%ld\n", jmp_cnt);
                        }
                    }

                    for (auto method_expr : stack.if_def.body_expr)
                    {
                        if (method_expr.type == MethodExprType::FUNCALL)
                        {
                            call_expression(def.first, def.second, method, method_expr, &text_section, &data_section, &extern_section, arg_reg_32, arg_reg_64);
                        }
                    }

                    string_format(&text_section, ".JPL_%ld:\n", jmp_cnt++);
                }
                else if (stack.type == MethodExprType::WHILE)
                {
                    String temp = "";

                    if (stack.while_def.type == CondType::BINOP)
                    {
                        MethodExpr expr;
                        expr.binop_def = stack.while_def.cond.binop;

                        parse_binop(def.first, def.second, method, expr, &temp, false);

                        string_format(&text_section, "\tjmp .JPL_%ld\n", jmp_cnt);
                        string_format(&text_section, ".JPL_%ld:\n", jmp_cnt + 1);

                        // if (stack.while_def.cond.val.type == ValueType::NUMBER)
                        // {
                        //     string_format(&text_section, "\tmov eax, %d\n", stack.if_def.cond.val.as_int()));
                        //     text_section.append("\tcmp eax, 0\n");
                        //     string_format(&text_section, "\tje .JPL_%ld\n", jmp_cnt));
                        // }
                    }

                    for (auto method_expr : stack.while_def.body_expr)
                    {
                        if (method_expr.type == MethodExprType::FUNCALL)
                        {
                            call_expression(def.first, def.second, method, method_expr, &text_section, &data_section, &extern_section, arg_reg_32, arg_reg_64);
                        }
                        else if (method_expr.type == MethodExprType::BINOP)
                        {
                            parse_binop(def.first, def.second, method, method_expr, &text_section, false);
                        }
                        else
                        {
                            printf("Wtaht? %d\n", method_expr.type);
                        }
                    }

                    string_format(&text_section, ".JPL_%ld:\n", jmp_cnt);
                    text_section.append(temp);

                    if (stack.while_def.cond.binop.op == "<")
                    {
                        string_format(&text_section, "\tjle .JPL_%ld\n", jmp_cnt + 1);
                    }
                    else if (stack.while_def.cond.binop.op == ">")
                    {
                        string_format(&text_section, "\tjge .JPL_%ld\n", jmp_cnt + 1);
                    }
                    else
                    {
                        printf("Not supported: %s\n", stack.while_def.cond.binop.op.c_str());
                        exit(1);
                    }

                    jmp_cnt += 2;
                }
            }

            // fixme 22/10/16: Remove nop if return type not void
            text_section.append("\tnop\n");
            text_section.append("\tnop\n");
            text_section.append("\tleave\n");
            text_section.append("\tret\n");
        }

        if (def.first == m_project->main_class)
        {
            text_section.append("global _start\n");
            text_section.append("_start:\n");
            string_format(&text_section, "\tcall %s$main\n", def.first.c_str());
        }
        if (m_project->main_class.empty())
            printf("WARN: Couldn't find main class in %s\n", def.first.c_str());

        String final_ = final_out.append(global_section).append(extern_section).append(text_section).append(data_section);

        String path = m_project->root_path;

        if (!std::filesystem::exists(path + "build/"))
            std::filesystem::create_directory(path + "build/");

        if (path[path.length() - 1] == '/')
            path.erase(path.length() - 1, path.length());

        String classpath = def.first;
        replace(classpath.begin(), classpath.end(), '$', '/');

        String out_path = path + "/build" + classpath;

        path += classpath;

        std::string final_path = out_path;
        final_path.append(".asm");
        std::ofstream out(final_path);
        out << final_;
        out.close();

        String obj_path = out_path;
        obj_path.append(".o");
        obj_files.emplace_back(obj_path);

        String compile_cmd = string_format("nasm -felf64 %s -o %s", final_path.c_str(), obj_path.c_str());
        printf("Compiling with NASM: %s\n", compile_cmd.c_str());
        String stdout = exec_cmd(compile_cmd.c_str());

        if (!stdout.empty())
        {
            printf("NASM compile: %s\n", stdout.c_str());
            exit(1);
        }
        // Reset
        final_out = "";
        extern_section = "";
        global_section = "";
        text_section = "section .text\n";
        data_section = "section .data\n";
        externs = {};
    }

    // printf("%s\n", final_out.c_str());

    String linker_obj_files;
    for (auto s : obj_files)
        linker_obj_files += s + " ";

    String exec_path = m_project->root_path;
    exec_path += "main";

    String linker_cmd = string_format("ld -o %s %s", exec_path.c_str(), linker_obj_files.c_str());
    printf("\nLinking obj files into to %s: %s\n", exec_path.c_str(), linker_cmd.c_str());
    String stdout = exec_cmd(linker_cmd.c_str()).c_str();

    if (!stdout.empty())
        printf("Linker: %s\n", stdout.c_str());
}
