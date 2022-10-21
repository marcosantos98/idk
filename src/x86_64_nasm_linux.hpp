#pragma once

#include "global.hpp"
#include "log.hpp"

class CodeGenerator
{
public:
    CodeGenerator(Project *proj)
        : m_project(proj) {}

    ~CodeGenerator()
    {
        delete generator;
        delete nasm;
        delete linker;
    }

    void generate();

private:
    Project *m_project;
    size_t jmp_cnt = 0;
    size_t str_cnt = 0;

    Log *generator = new Log("Generator");
    Log *nasm = new Log("NASM");
    Log *linker = new Log("LD");

    Map<String, VariableDef> m_global_var_alias = {};
    Map<String, std::tuple<VariableDef, StackVar>> m_stack_var_alias = {};

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

    struct GenContext
    {
        String global = "";
        String ext = "";
        String text = "section .text\n";
        String data = "section .data\n";
        String current_class_name = "";
        ClassDef current_class;
        size_t current_arg_index = 0;
        Vec<String> externs = {};
    };

    GenContext m_ctx;

    void init(ClassDef, MethodDef);

    void gen_method(MethodDef);
    void gen_stack_var(MethodExpr);
    void gen_binop(MethodExpr);
    void gen_funcall(MethodExpr);
    void gen_if(MethodExpr);
    void gen_while(MethodExpr);
    void gen_assign(MethodExpr);
    void gen_type(MethodExpr);
    void gen_var_ref(Value);
    void gen_value_type(Value);

    String as_val_or_mem(Value);

    static void reset_ctx(GenContext *ctx)
    {
        ctx->global = "";
        ctx->ext = "";
        ctx->text = "section .text\n";
        ctx->data = "section .data\n";
        ctx->externs = {};
    }
};