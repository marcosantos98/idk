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

void CodeGenerator::init(ClassDef class_def, MethodDef method_def)
{
    m_ctx.current_method = method_def;
    m_global_var_alias = {};
    m_stack_var_alias = {};

    for (auto var_def : class_def.class_variables)
        m_global_var_alias[var_def.arg_name] = var_def;

    for (auto expr : method_def.method_expressions)
    {
        if (expr.type == MethodExprType::VAR)
            m_stack_var_alias[expr.var_def.arg_name] = expr.var_def;

        // fixme 22/10/21: This isn't that good
        else if (expr.type == MethodExprType::WHILE)
        {
            for (auto while_expr : expr.while_def.body_expr)
                if (while_expr.type == MethodExprType::VAR)
                    m_stack_var_alias[while_expr.var_def.arg_name] = while_expr.var_def;
        }
    }
}

void CodeGenerator::generate()
{
    Vec<String> obj_files = {};

    for (auto def : m_project->project_classes)
    {
        m_ctx.current_class_name = def.first;
        m_ctx.current_class = def.second;

        generator->log_tdbg("Generating for: %s\n", def.second.in_file.c_str());

        String global_data_init;

        for (auto var : def.second.class_variables)
        {
            if (var.type == VariableTypeDef::ARRAY)
            {
                if (var.array.arr_size.type == ValueType::NUMBER)
                {
                    auto to_alloc = var.array.arr_size.as_int() * NAVA::primitive_byte_sizes[var.array.arr_type];
                    string_format(&m_ctx.bss, "\t%s_%s resb %ld\n", def.first.c_str(), var.arg_name.c_str(), to_alloc);
                }
            }
            else if (var.class_name == "int")
            {
                if (var.type == VariableTypeDef::BINOP)
                {
                    if (var.binop.left.type == ValueType::NUMBER && var.binop.rigth.type == ValueType::NUMBER)
                    {
                        if (var.binop.op == "+")
                        {
                            int val = var.binop.left.as_int() + var.binop.rigth.as_int();
                            string_format(&m_ctx.data, "\t%s_%s dd %d\n", def.first.c_str(), var.arg_name.c_str(), val);
                        }
                    }
                }
                else if (var.type == VariableTypeDef::VAL && var.val.type == ValueType::VAR_REF)
                {
                    // fixme 22/10/25: Check on type checker
                    string_format(&m_ctx.data, "\t%s_%s dd 0\n", def.first.c_str(), var.arg_name.c_str());
                    string_format(&global_data_init, "\tmov eax, [%s_%s]\n", def.first.c_str(), var.val.raw.c_str());
                    string_format(&global_data_init, "\tmov [%s_%s], eax\n", def.first.c_str(), var.arg_name.c_str());
                }
                else if (var.type == VariableTypeDef::VAL && var.val.type == ValueType::ARRAY_VAR_REF)
                {
                    // fixme 22/10/25: Check on type checker
                    string_format(&m_ctx.data, "\t%s_%s dd 0\n", def.first.c_str(), var.arg_name.c_str());

                    auto index = var.val.raw.substr(var.val.raw.find_first_of("[") + 1, var.val.raw.length() - var.val.raw.find_first_of("[") - 2);
                    auto alias = var.val.raw.substr(0, var.val.raw.find_first_of("["));

                    int i = stoi(index);
                    auto to_alloc = i * NAVA::primitive_byte_sizes[var.array.arr_type];

                    string_format(&global_data_init, "\tmov eax, [%s_%s-%ld]\n", def.first.c_str(), alias.c_str(), to_alloc);
                    string_format(&global_data_init, "\tmov [%s_%s], eax\n", def.first.c_str(), var.arg_name.c_str());
                }
                else if (var.type == VariableTypeDef::VAL)
                {
                    string_format(&m_ctx.data, "\t%s_%s dd %d\n", def.first.c_str(), var.arg_name.c_str(), var.val.as_int());
                }
            }
            else if (var.class_name == "String")
            {
                if (var.val.type == ValueType::STRING)
                {
                    // fixme 22/10/19: Write every byte instead of string literal
                    string_format(&m_ctx.data, "\t%s_%s db \"%s\", 10\n", def.first.c_str(), var.arg_name.c_str(), var.val.as_str().c_str());
                    string_format(&m_ctx.data, "\t%s_%s_sz equ $-%s_%s\n", def.first.c_str(), var.arg_name.c_str(), def.first.c_str(), var.arg_name.c_str());
                }
            }
            else if (var.class_name == "double")
            {
                string_format(&m_ctx.data, "\t%s dq %1f\n", var.arg_name.c_str(), var.val.as_double());
            }
            else if (var.class_name == "boolean")
            {
                string_format(&m_ctx.data, "\t%s_%s dd %d\n", def.first.c_str(), var.arg_name.c_str(), var.val.as_bool());
            }
        }

        if (!global_data_init.empty())
        {
            string_format(&m_ctx.text, "%s$_init_global_:\n", def.first.c_str());

            m_ctx.text.append(global_data_init);

            m_ctx.text += "\tret\n";
        }

        for (auto method : def.second.class_methods)
        {
            init(def.second, method);
            gen_method(method);
        }

        generator->log_tok("%s\n", m_project->main_class.c_str());

        if (def.first == m_project->main_class)
        {
            m_ctx.text += "global _start\n";
            m_ctx.text += "_start:\n";
            if (!global_data_init.empty())
                string_format(&m_ctx.text, "\tcall %s$_init_global_\n", def.first.c_str());

            // m_ctx.text += "\tpop rdi\n";
            string_format(&m_ctx.text, "\tcall %s$main\n", def.first.c_str());
        }

        if (m_project->main_class.empty())
            generator->log_twarn("WARN: Couldn't find main class in %s\n", def.first.c_str());

        auto output_asm = "" + m_ctx.global + m_ctx.ext + m_ctx.text + m_ctx.data + m_ctx.bss;
        auto path = m_project->root_path;
        auto build_path = std::filesystem::relative(path).append("build").string();

        if (std::filesystem::is_directory(path) && !std::filesystem::exists(build_path))
            std::filesystem::create_directory(build_path);
        else if (!std::filesystem::is_directory(path))
        {
            auto chopped = chop_delimiter(path, '/');
            // fixme 22/10/19: This is junky.
            std::ostringstream vts;
            std::copy(chopped.begin(), chopped.end() - 1, std::ostream_iterator<String>(vts, "/"));
            path = vts.str();
            build_path = std::filesystem::relative(path).append("build").string();
            if (!std::filesystem::exists(build_path))
                std::filesystem::create_directory(build_path);
        }

        // [folder_path]$[ClassName]
        // Replace all the '$' with '/' to construct a valid filepath
        auto classpath = def.first;
        if (!classpath.starts_with("$"))
            classpath = "$" + def.first;
        replace(classpath.begin(), classpath.end(), '$', '/');

        auto out_path = build_path + classpath;

        auto chopped = chop_delimiter(out_path, '/');
        // fixme 22/10/19: This is junky.
        std::ostringstream vts;
        std::copy(chopped.begin(), chopped.end() - 1, std::ostream_iterator<String>(vts, "/"));
        String tmp = vts.str();
        if (!std::filesystem::exists(tmp))
            std::filesystem::create_directories(tmp);

        path += classpath;

        auto final_path = out_path;
        final_path.append(".asm");
        std::ofstream out(final_path);
        out << output_asm;
        out.close();

        auto obj_path = out_path;
        obj_path.append(".o");
        obj_files.emplace_back(obj_path);

        auto compile_cmd = string_format("nasm -felf64 %s -o %s", final_path.c_str(), obj_path.c_str());
        nasm->log_tdbg("Compiling: %s\n", compile_cmd.c_str());
        auto stdout = exec_cmd(compile_cmd.c_str());

        if (stdout != "")
        {
            nasm->log_terr("NASM compile: %s\n", stdout.c_str());
            exit(1);
        }

        reset_ctx(&m_ctx);
    }

    String linker_obj_files;
    for (auto s : obj_files)
        linker_obj_files += s + " ";

    String exec_path;
    if (std::filesystem::is_directory(m_project->root_path))
    {
        exec_path = m_project->root_path;
        if (!exec_path.ends_with('/'))
            exec_path.append("/");
    }
    else if (!std::filesystem::is_directory(m_project->root_path))
    {
        auto chopped = chop_delimiter(m_project->root_path, '/');
        // fixme 22/10/19: This is junky.
        std::ostringstream vts;
        std::copy(chopped.begin(), chopped.end() - 1, std::ostream_iterator<String>(vts, "/"));
        exec_path = vts.str();
    }

    exec_path += "main";

    auto linker_cmd = string_format("ld -o %s %s", exec_path.c_str(), linker_obj_files.c_str());
    linker->log_tdbg("Linking obj files to %s: %s\n", exec_path.c_str(), linker_cmd.c_str());
    auto stdout = exec_cmd(linker_cmd.c_str());

    if (!stdout.empty())
        linker->log_terr("Linker: %s\n", stdout.c_str());
}

void CodeGenerator::gen_method(MethodDef method)
{
    if (method.is_static || method.is_public)
        string_format(&m_ctx.global, "global %s$%s\n", m_ctx.current_class_name.c_str(), method.method_name.c_str());

    //[class_name]$[method_name]:
    string_format(&m_ctx.text, "%s$%s:\n", m_ctx.current_class_name.c_str(), method.method_name.c_str());

    m_ctx.text += "\tpush rbp\n";
    m_ctx.text += "\tmov rbp, rsp\n";

    // fixme 22/10/19: Always setting the offset + 10
    //                 This should be calculated with the args size
    string_format(&m_ctx.text, "\tsub rsp, %ld\n", method.stack_offset + 10);

    // for (auto arg : method.args)
    // {
    //     mov_mX_reg(&m_ctx.text, arg.class_name, 0, arg_reg_32[m_ctx.current_arg_index++].c_str());
    // }
    // m_ctx.current_arg_index = 0;

    for (auto method_expression : method.method_expressions)
    {
        gen_type(method_expression);
    }

    // fixme 22/10/16: Remove nop if return type not void
    m_ctx.text += "\tnop\n";
    m_ctx.text += "\tnop\n";
    m_ctx.text += "\tleave\n";
    m_ctx.text += "\tret\n";
}

void CodeGenerator::gen_stack_var(MethodExpr method_expr)
{
    if (method_expr.var_def.val.type == ValueType::NUMBER)
    {
        mov_mX_immX(&m_ctx.text,
                    method_expr.var_def.class_name,
                    method_expr.var_def.stack_info.stack_offset,
                    method_expr.var_def.val);
    }
    else if (method_expr.var_def.type == VariableTypeDef::BINOP)
    {
        MethodExpr expr;
        expr.type = MethodExprType::VAR;
        expr.binop_def = method_expr.var_def.binop;
        expr.var_def.stack_info.stack_offset = method_expr.var_def.stack_info.stack_offset;
        expr.var_def.class_name = method_expr.var_def.class_name;
        gen_binop(expr);
    }
    else if (method_expr.var_def.val.type == ValueType::VAR_REF)
    {
        auto val = method_expr.var_def.val;
        if (m_global_var_alias.contains(val.raw))
            generator->log_terr("StackVar with reference to global variable not implemented yet!\n");
        // mov_mX_data(&m_ctx.text, arg_reg_32[m_ctx.current_arg_index++].c_str(), (m_ctx.current_class_name + "_" + val.raw));
        else if (m_stack_var_alias.contains(val.raw))
        {
            auto primitive = m_stack_var_alias[val.raw].class_name;
            auto offset = m_stack_var_alias[val.raw].stack_info.stack_offset;

            mov_reg_mX(&m_ctx.text, primitive, offset, "eax");
            mov_mX_reg(&m_ctx.text, method_expr.var_def.class_name, method_expr.var_def.stack_info.stack_offset, "eax");
        }
        else
            generator->log_terr("Variable not found! %s\n", val.raw.c_str());
    }
    else if (method_expr.var_def.val.type == ValueType::BOOL)
    {
        auto val = method_expr.var_def.val;
        mov_mX_immX(&m_ctx.text, "boolean", method_expr.var_def.stack_info.stack_offset, val);
    }
    else
        generator->log_tdbg("ValueType not implemented: %d\n", static_cast<int>(method_expr.var_def.val.type));
}

void CodeGenerator::gen_binop(MethodExpr stack)
{

    String left = as_val_or_mem(stack.binop_def.left);
    String right = as_val_or_mem(stack.binop_def.rigth);

    if (stack.binop_def.op == "+")
    {
        string_format(&m_ctx.text, "\tmov eax, %s\n", left.c_str());
        string_format(&m_ctx.text, "\tadd eax, %s\n", right.c_str());
    }
    else if (stack.binop_def.op == "-")
    {
        string_format(&m_ctx.text, "\tmov eax, %s\n", left.c_str());
        string_format(&m_ctx.text, "\tsub eax, %s\n", right.c_str());
    }
    else if (stack.binop_def.op == "*")
    {
        string_format(&m_ctx.text, "\tmov eax, %s\n", left.c_str());
        string_format(&m_ctx.text, "\timul eax, %s\n", right.c_str());
    }
    else if (stack.binop_def.op == "<")
        string_format(&m_ctx.text, "\tcmp %s, %s\n", left.c_str(), right.c_str());
    else if (stack.binop_def.op == ">")
        string_format(&m_ctx.text, "\tcmp %s, %s\n", left.c_str(), right.c_str());
    else if (stack.binop_def.op == "++")
        string_format(&m_ctx.text, "\tinc %s\n", left.c_str());
    else if (stack.binop_def.op == "--")
        string_format(&m_ctx.text, "\tdec %s\n", left.c_str());

    else if (stack.binop_def.op == "/")
    {
        string_format(&m_ctx.text, "\tmov eax, %s\n", left.c_str());
        m_ctx.text += "\txor edx, edx\n";
        string_format(&m_ctx.text, "\tmov ecx, %s\n", right.c_str());
        m_ctx.text += "\tcdq\n";
        m_ctx.text += "\tidiv ecx\n";
    }
    else if (stack.binop_def.op == "%")
    {
        string_format(&m_ctx.text, "\tmov eax, %s\n", left.c_str());
        m_ctx.text += "\txor edx, edx\n";
        string_format(&m_ctx.text, "\tmov ecx, %s\n", right.c_str());
        m_ctx.text += "\tcdq\n";
        m_ctx.text += "\tidiv ecx\n";
        m_ctx.text += "\tmov eax, edx\n";
    }

    if (stack.type == MethodExprType::VAR)
        mov_mX_reg(&m_ctx.text, stack.var_def.class_name, stack.var_def.stack_info.stack_offset, "eax");
}

void CodeGenerator::gen_funcall(MethodExpr method_expr)
{

    if (method_expr.func_def.call_name == "asm")
    {
        // fixme 22/10/06: Check for only one arg or maybe parse this individually instead of working as a call expression.
        m_ctx.text += method_expr.func_def.args[0].val.as_str();
    }
    else
    {
        if (method_expr.func_def.args.size() > 0 && method_expr.func_def.args.size() < 6)
        {
            for (auto &arg : method_expr.func_def.args)
            {
                gen_value_type(arg.val);
            }

            m_ctx.current_arg_index = 0;
        }

        String s;

        for (auto ss : m_ctx.current_class.class_methods)
        {
            if (ss.method_name == method_expr.func_def.call_name)
            {
                s.append(m_ctx.current_class_name + "$" + method_expr.func_def.call_name);
                break;
            }
        }

        if (s.empty())
            s = "$" + method_expr.func_def.call_name;

        replace(s.begin(), s.end(), '.', '$');

        if (!VEC_HAS(m_ctx.externs, s))
        {
            m_ctx.externs.emplace_back(s);
            string_format(&m_ctx.ext, "extern %s\n", s.c_str());
        }

        string_format(&m_ctx.text, "\tcall %s\n", s.c_str());
    }
}

void CodeGenerator::gen_if(MethodExpr method_expr)
{
    if (method_expr.if_def.type == CondType::VAL)
    {
        if (method_expr.if_def.cond.val.type == ValueType::NUMBER)
        {
            string_format(&m_ctx.text, "\tmov eax, %d\n", method_expr.if_def.cond.val.as_int());
            m_ctx.text += "\tcmp eax, 0\n";
            string_format(&m_ctx.text, "\tje .JPL_%ld\n", jmp_cnt);
        }
        else if (method_expr.if_def.cond.val.type == ValueType::BOOL)
        {
            string_format(&m_ctx.text, "\tmov eax, %d\n", method_expr.if_def.cond.val.as_bool());
            m_ctx.text += "\tcmp eax, 1\n";
            string_format(&m_ctx.text, "\tjne .JPL_%ld\n", jmp_cnt);
        }
        else if (method_expr.if_def.cond.val.type == ValueType::VAR_REF)
        {
            if (m_global_var_alias.contains(method_expr.if_def.cond.val.raw))
            {
                generator->log_tdbg("%s Not implemented yet\n", __FILE__);
            }
            else if (m_stack_var_alias.contains(method_expr.if_def.cond.val.raw))
            {
                auto primitive = m_stack_var_alias[method_expr.if_def.cond.val.raw].class_name;
                auto offset = m_stack_var_alias[method_expr.if_def.cond.val.raw].stack_info.stack_offset;
                mov_reg_mX(&m_ctx.text, primitive, offset, "eax");
                m_ctx.text += "\tcmp eax, 1\n";
                string_format(&m_ctx.text, "\tjne .JPL_%ld\n", jmp_cnt);
            }
            else
                generator->log_terr("%s Variable not found: %s\n", __FILE__, method_expr.if_def.cond.val.raw.c_str());
        }
        else
            generator->log_terr("If_Expr: Not implemented %d\n", static_cast<int>(method_expr.if_def.cond.val.type));
    }

    for (auto method_expr : method_expr.if_def.body_expr)
        if (method_expr.type == MethodExprType::FUNCALL)
            gen_funcall(method_expr);

    string_format(&m_ctx.text, ".JPL_%ld:\n", jmp_cnt++);
}

void CodeGenerator::gen_while(MethodExpr method_expr)
{

    String temp = "";

    string_format(&m_ctx.text, "\tjmp .JPL_%ld\n", jmp_cnt);
    string_format(&m_ctx.text, ".JPL_%ld:\n", jmp_cnt + 1);

    for (auto body_expr : method_expr.while_def.body_expr)
        gen_type(body_expr);

    string_format(&m_ctx.text, ".JPL_%ld:\n", jmp_cnt);

    if (method_expr.while_def.type == CondType::BINOP)
    {
        MethodExpr expr;
        expr.binop_def = method_expr.while_def.cond.binop;

        gen_binop(expr);
    }

    if (method_expr.while_def.cond.binop.op == "<")
    {
        string_format(&m_ctx.text, "\tjl .JPL_%ld\n", jmp_cnt + 1);
    }
    else if (method_expr.while_def.cond.binop.op == ">")
    {
        string_format(&m_ctx.text, "\tjg .JPL_%ld\n", jmp_cnt + 1);
    }
    else
    {
        generator->log_terr("Not supported: %s\n", method_expr.while_def.cond.binop.op.c_str());
    }

    jmp_cnt += 2;
}

void CodeGenerator::gen_assign(MethodExpr method_expr)
{
    if (method_expr.type == MethodExprType::ASSIGN)
    {
        if (method_expr.assign_def.type == AssignDefType::BINOP)
        {
            MethodExpr expr;
            // fixme 22/10/27: Find array dont assume that is in current stack.
            expr.var_def = m_stack_var_alias[method_expr.assign_def.alias];
            expr.type = MethodExprType::VAR;
            expr.binop_def = method_expr.assign_def.binop;
            gen_binop(expr);
        }
        else if (method_expr.assign_def.type == AssignDefType::VAL)
        {
            MethodExpr expr;
            expr.var_def.val.type = ValueType::VAR_REF;
            expr.var_def.val = method_expr.assign_def.val;
            expr.var_def.class_name = m_stack_var_alias[method_expr.assign_def.alias].class_name;
            expr.var_def.stack_info.stack_offset = m_stack_var_alias[method_expr.assign_def.alias].stack_info.stack_offset;
            gen_stack_var(expr);
        }
        // fixme 22/10/25: This is probably broken now.
        // gen_type(method_expr);
    }
    else if (method_expr.type == MethodExprType::ASSIGN_ARRAY)
    {
        auto offset = m_stack_var_alias[method_expr.assign_array_def.alias].stack_info.stack_offset;
        printf("%ld\n", offset);

        auto primitive = m_stack_var_alias[method_expr.assign_array_def.alias].class_name;
        printf("%s\n", primitive.substr(0, primitive.find_first_of("[")).c_str());

        if (method_expr.assign_array_def.type == AssignDefType::VAL)
        {
            MethodExpr expr;
            expr.var_def.val.type = ValueType::NUMBER;
            expr.var_def.val = method_expr.assign_array_def.val;
            expr.var_def.class_name = m_stack_var_alias[method_expr.assign_array_def.alias].class_name;
            expr.var_def.stack_info.stack_offset = m_stack_var_alias[method_expr.assign_def.alias].stack_info.stack_offset;

            if (method_expr.assign_array_def.element_index.type == ValueType::NUMBER)
            {
                auto off = offset - method_expr.assign_array_def.element_index.as_int() * NAVA::primitive_byte_sizes[expr.var_def.class_name];
printf("%ld", off);
                expr.var_def.stack_info.stack_offset = off;

                gen_stack_var(expr);
            }
        }

        // gen_type(method_expr);
    }
}

void CodeGenerator::gen_array(MethodExpr method_expr)
{
    Value val;
    val.raw = "0";
    val.type = ValueType::NUMBER;

    mov_mX_immX(&m_ctx.text, method_expr.var_def.class_name, method_expr.var_def.stack_info.stack_offset, val);
}

void CodeGenerator::gen_type(MethodExpr method_expr)
{
    switch (method_expr.type)
    {
    case MethodExprType::VAR:
        gen_stack_var(method_expr);
        break;
    case MethodExprType::BINOP:
        gen_binop(method_expr);
        break;
    case MethodExprType::FUNCALL:
        gen_funcall(method_expr);
        break;
    case MethodExprType::IF:
        gen_if(method_expr);
        break;
    case MethodExprType::WHILE:
        gen_while(method_expr);
        break;
    case MethodExprType::ASSIGN:
    case MethodExprType::ASSIGN_ARRAY:
        gen_assign(method_expr);
        break;
    case MethodExprType::ARRAY:
        gen_array(method_expr);
        break;
    default:
        generator->log_terr("gen_type: Not Implemented\n");
        break;
    }
}

void CodeGenerator::gen_var_ref(Value val)
{
    if (val.type == ValueType::VAR_REF)
    {
        if (m_global_var_alias.contains(val.raw))
            mov_reg_data(&m_ctx.text, arg_reg_32[m_ctx.current_arg_index++].c_str(), (m_ctx.current_class_name + "_" + val.raw));
        else if (m_stack_var_alias.contains(val.raw))
        {
            auto primitive = m_stack_var_alias[val.raw].class_name;
            auto offset = m_stack_var_alias[val.raw].stack_info.stack_offset;
            mov_reg_mX(&m_ctx.text, primitive, offset, arg_reg_32[m_ctx.current_arg_index++].c_str());
        }
        else
        {
            bool found;
            ArgumentDef argument;
            for (auto arg : m_ctx.current_method.args)
            {
                if (arg.arg_name == val.raw)
                {
                    argument = arg;
                    found = true;
                    break;
                }
            }

            if (!found)
                generator->log_terr("%s, Variable not found! %s\n", val.raw.c_str(), __FUNCTION__);

            // fixme 22/10/27: Stack offset
            mov_reg_mX(&m_ctx.text, argument.class_name, 0, arg_reg_32[m_ctx.current_arg_index++].c_str());
        }
    }
    else
    {
        auto name = val.raw.substr(0, val.raw.find_first_of("["));
        auto el = val.raw.substr(val.raw.find_first_of("[") + 1, 1);

        if (m_global_var_alias.contains(name))
            mov_reg_data(&m_ctx.text, arg_reg_32[m_ctx.current_arg_index++].c_str(), (m_ctx.current_class_name + "_" + name));
        else if (m_stack_var_alias.contains(name))
        {
            auto primitive = m_stack_var_alias[name].class_name;
            auto offset = m_stack_var_alias[name].stack_info.stack_offset - (stoi(el) * NAVA::primitive_byte_sizes[primitive]);
            mov_reg_mX(&m_ctx.text, primitive, offset, arg_reg_32[m_ctx.current_arg_index++].c_str());
        }
        else
            generator->log_terr("Variable not found! %s\n", name.c_str());
    }
}

void CodeGenerator::gen_value_type(Value val)
{
    switch (val.type)
    {
    case ValueType::NUMBER:
        mov_reg_immX(&m_ctx.text, "int", arg_reg_32[m_ctx.current_arg_index++].c_str(), val);
        break;
    case ValueType::STRING:
        // fixme 22/10/19: Write every byte instead of string literal
        string_format(&m_ctx.data, "\ttmp_str_%ld db \"%s\", 10\n", str_cnt, val.as_str().c_str());
        string_format(&m_ctx.text, "\tmov %s, tmp_str_%ld\n", arg_reg_32[m_ctx.current_arg_index++].c_str(), str_cnt++);
        break;
    case ValueType::ARRAY_VAR_REF:
    case ValueType::VAR_REF:
        gen_var_ref(val);
        break;
    case ValueType::BOOL:
        mov_reg_immX(&m_ctx.text, "boolean", arg_reg_32[m_ctx.current_arg_index++].c_str(), val);
        break;
    default:
        generator->log_tdbg("fixme 22/10/19: 402 Not Implemented %d!\n", static_cast<int>(val.type));
        // exit(1);
        break;
    }
}

String CodeGenerator::as_val_or_mem(Value val)
{
    if (val.type == ValueType::NUMBER)
    {
        return string_format("%d", val.as_int());
    }
    else if (val.type == ValueType::VAR_REF)
    {
        if (m_global_var_alias.contains(val.raw))
            return string_format("%s_%s", m_ctx.current_class_name.c_str(), val.raw.c_str());
        else if (m_stack_var_alias.contains(val.raw))
            return string_format("dword [rbp-%d]", m_stack_var_alias[val.raw].stack_info.stack_offset);
        else
            generator->log_terr("as_val_or_mem: Var not found %s\n", val.raw.c_str());
    }
    generator->log_terr("AsValOrMem: Not implemented %d %s\n", static_cast<int>(val.type), val.raw.c_str());

    return "";
}