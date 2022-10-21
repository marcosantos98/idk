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
    m_global_var_alias = {};
    m_stack_var_alias = {};

    for (auto var_def : class_def.class_variables)
        m_global_var_alias[var_def.arg_name] = var_def;

    for (auto expr : method_def.method_expressions)
    {
        if (expr.var_def.has_value())
            m_stack_var_alias[std::get<1>(expr.var_def.value()).alias] = expr.var_def.value();

        // fixme 22/10/21: This isn't that good
        if (expr.type == MethodExprType::WHILE)
            for (auto while_expr : expr.while_def.body_expr)
                if (while_expr.var_def.has_value())
                    m_stack_var_alias[std::get<1>(while_expr.var_def.value()).alias] = while_expr.var_def.value();
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

        for (auto var : def.second.class_variables)
        {
            if (var.class_name == "int")
            {
                string_format(&m_ctx.text, "\t%s_%s dd %d\n", def.first.c_str(), var.arg_name.c_str(), var.val.as_int());
            }
            else if (var.class_name == "double")
            {
                string_format(&m_ctx.data, "\t%s dq %1f\n", var.arg_name.c_str(), var.val.as_double());
            }
        }

        for (auto method : def.second.class_methods)
        {
            init(def.second, method);
            gen_method(method);
        }

        if (def.first == m_project->main_class)
        {
            m_ctx.text += "global _start\n";
            m_ctx.text += "_start:\n";
            string_format(&m_ctx.text, "\tcall %s$main\n", def.first.c_str());
        }

        if (m_project->main_class.empty())
            generator->log_twarn("WARN: Couldn't find main class in %s\n", def.first.c_str());

        auto output_asm = "" + m_ctx.global + m_ctx.ext + m_ctx.text + m_ctx.data;
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

    // fixme 22/10/14: pop args from stack

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
    if (std::get<0>(method_expr.var_def.value()).val.type == ValueType::NUMBER)
    {
        mov_mX_immX(&m_ctx.text,
                    std::get<0>(method_expr.var_def.value()).class_name,
                    std::get<1>(method_expr.var_def.value()).stack_offset,
                    std::get<0>(method_expr.var_def.value()).val);
    }
    else if (std::get<0>(method_expr.var_def.value()).val.type == ValueType::VAR_REF)
    {
        auto val = std::get<0>(method_expr.var_def.value()).val;
        if (m_global_var_alias.contains(val.raw))
            generator->log_terr("StackVar with reference to global variable not implemented yet!\n");
        // mov_mX_data(&m_ctx.text, arg_reg_32[m_ctx.current_arg_index++].c_str(), (m_ctx.current_class_name + "_" + val.raw));
        else if (m_stack_var_alias.contains(val.raw))
        {
            auto primitive = std::get<0>(m_stack_var_alias[val.raw]).class_name;
            auto offset = std::get<1>(m_stack_var_alias[val.raw]).stack_offset;
            mov_reg_mX(&m_ctx.text, primitive, offset, "eax");
            mov_mX_reg(&m_ctx.text, std::get<0>(method_expr.var_def.value()).class_name, std::get<1>(method_expr.var_def.value()).stack_offset, "eax");
        }
        else
            generator->log_terr("Variable not found! %s\n", val.raw.c_str());
    }
    else if (std::get<0>(method_expr.var_def.value()).val.type == ValueType::BOOL)
    {
        auto val = std::get<0>(method_expr.var_def.value()).val;
        mov_mX_immX(&m_ctx.text, "boolean", std::get<1>(method_expr.var_def.value()).stack_offset, val);
    }
    else
        generator->log_terr("ValueType not implemented: %d\n", static_cast<int>(std::get<0>(method_expr.var_def.value()).val.type));
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

    if (stack.var_def.has_value())
        mov_mX_reg(&m_ctx.text, std::get<0>(stack.var_def.value()).class_name, std::get<1>(stack.var_def.value()).stack_offset, "eax");
}

void CodeGenerator::gen_funcall(MethodExpr method_expr)
{

    if (method_expr.func_def.call_name == "asm")
    {
        // fixme 22/10/06: Check for only one arg or maybe parse this individually instead of working as a call expression.
        m_ctx.text += method_expr.func_def.args[0].as_str();
    }
    else
    {
        if (method_expr.func_def.args.size() > 0 && method_expr.func_def.args.size() < 6)
        {
            for (auto &arg : method_expr.func_def.args)
            {
                gen_value_type(arg);
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
                generator->log_terr("%s Not implemented yet\n", __FILE__);
            }
            else if (m_stack_var_alias.contains(method_expr.if_def.cond.val.raw))
            {
                auto primitive = std::get<0>(m_stack_var_alias[method_expr.if_def.cond.val.raw]).class_name;
                auto offset = std::get<1>(m_stack_var_alias[method_expr.if_def.cond.val.raw]).stack_offset;
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
    gen_type(method_expr.assign_def.val[0]);
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
        gen_assign(method_expr);
        break;
    default:
        generator->log_terr("gen_type: Not Implemented\n");
        break;
    }
}

void CodeGenerator::gen_var_ref(Value val)
{
    if (m_global_var_alias.contains(val.raw))
        mov_reg_data(&m_ctx.text, arg_reg_32[m_ctx.current_arg_index++].c_str(), (m_ctx.current_class_name + "_" + val.raw));
    else if (m_stack_var_alias.contains(val.raw))
    {
        auto primitive = std::get<0>(m_stack_var_alias[val.raw]).class_name;
        auto offset = std::get<1>(m_stack_var_alias[val.raw]).stack_offset;
        mov_reg_mX(&m_ctx.text, primitive, offset, arg_reg_32[m_ctx.current_arg_index++].c_str());
    }
    else
        generator->log_terr("Variable not found! %s\n", val.raw.c_str());
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
    case ValueType::VAR_REF:
        gen_var_ref(val);
        break;
    case ValueType::BOOL:
        mov_reg_immX(&m_ctx.text, "boolean", arg_reg_32[m_ctx.current_arg_index++].c_str(), val);
        break;
    default:
        generator->log_terr("fixme 22/10/19: 402 Not Implemented %d!\n", static_cast<int>(val.type));
        exit(1);
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
            return string_format("dword [rbp-%d]", std::get<1>(m_stack_var_alias[val.raw]).stack_offset);
        else
            generator->log_terr("as_val_or_mem: Var not found %s\n", val.raw.c_str());
    }
    generator->log_terr("AsValOrMem: Not implemented %d %s\n", static_cast<int>(val.type), val.raw.c_str());

    return "";
}