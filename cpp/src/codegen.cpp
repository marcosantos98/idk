#include "codegen.hpp"

String CodeGenerator::generate()
{

    NAVA::GlobalContext ctx;

    ctx.data_section.append("section .data\n");
    ctx.text_section.append("section .text\n");

    ctx.class_name = m_root_class.get()->p_definition.class_name;

    bool main = has_main();

    if (main)
        ctx.text_section.append("global _start\n");

    ctx.var_ctx = NAVA::VarContext::DATA;
    for (auto &var : m_root_class.get()->p_class_variables)
    {
        var.get()->code_gen(&ctx);
    }

    ctx.var_ctx = NAVA::VarContext::STACK;
    for (auto &method : m_root_class.get()->p_methods)
    {
        method.get()->code_gen(&ctx);
    }

    if (main)
    {
        ctx.text_section.append("_start:\n");

        ctx.text_section.append("\t;; Call entry point\n");
        ctx.text_section.append("\tcall ").append(m_root_class.get()->p_definition.class_name).append("$main\n");
        ctx.text_section.append("\t;; Exit syscall\n");
        ctx.text_section.append("\tmov rax, 60\n");
        ctx.text_section.append("\tmov rdi, 0\n");
        ctx.text_section.append("\tsyscall\n");
    }

    String final_out;
    final_out.append(ctx.extern_section);
    final_out.append(ctx.text_section);
    final_out.append(ctx.data_section);

    //printf("%s\n", final_out.c_str());

    return final_out;
}

bool CodeGenerator::has_main()
{

    for (auto &method : m_root_class.get()->p_methods)
        if (auto method_ = dynamic_cast<const MethodExpression *>(method.get()))
            if (method_->p_definition.arg_name == "main" && method_->p_definition.class_name == "void" && method_->p_definition.mod.is_static && method_->p_definition.mod.is_public)
                return true;
    return false;
}