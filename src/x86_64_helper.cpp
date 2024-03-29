#include "x86_64_helper.hpp"

void mov_mX_reg(String *out, String const &primitive, size_t offset, const char *reg)
{
    if (primitive == "boolean")
    {
        string_format(out, "\tmov byte [rbp-%ld], %s\n", offset, reg);
    }
    else if (primitive == "short")
    {
        string_format(out, "\tmov word [rbp-%ld], %s\n", offset, reg);
    }
    else if (primitive == "int")
    {
        string_format(out, "\tmov dword [rbp-%ld], %s\n", offset, reg);
    }
}

void mov_mX_mX(String *out, String const &primitive, size_t off, size_t set)
{
    if (primitive == "boolean")
    {
        string_format(out, "\tmov byte [rbp-%ld], byte [rbp-%ld]\n", off, set);
    }
    else if (primitive == "short")
    {
        string_format(out, "\tmov word [rbp-%ld], word [rbp-%ld]\n", off, set);
    }
    else if (primitive == "int")
    {
        string_format(out, "\tmov dword [rbp-%ld], dword [rbp-%ld]\n", off, set);
    }
}

void mov_reg_mX(String *out, String const &primitive, size_t offset, const char *reg)
{
    if (primitive == "boolean")
    {
        string_format(out, "\tmovzx %s, byte [rbp-%ld]\n", reg, offset);
    }
    else if (primitive == "short")
    {
        string_format(out, "\tmov %s, word [rbp-%ld]\n", reg, offset);
    }
    else if (primitive == "int")
    {
        string_format(out, "\tmov %s, dword [rbp-%ld]\n", reg, offset);
    }
    else
    {
        printf("Unknown primitive! %s\n", primitive.c_str());
        exit(1);
    }
}

void mov_reg_immX(String *out, String const &primitive, const char *reg, Value val)
{
    if (primitive == "boolean")
    {
        string_format(out, "\tmov %s, %d\n", reg, val.as_bool());
    }
    else if (primitive == "short")
    {
        string_format(out, "\tmov %s, %d\n", reg, val.as_int());
    }
    else if (primitive == "int")
    {
        string_format(out, "\tmov %s, %d\n", reg, val.as_int());
    }
}

void mov_mX_immX(String *out, String const &primitive, size_t stack_offset, Value val)
{
    if (primitive == "boolean")
    {
        string_format(out, "\tmov byte [rbp-%ld], %d\n", stack_offset, val.as_bool());
    }
    else if (primitive == "short")
    {
        string_format(out, "\tmov word [rbp-%ld], %d\n", stack_offset, val.as_int());
    }
    else if (primitive == "int")
    {
        string_format(out, "\tmov dword [rbp-%ld], %d\n", stack_offset, val.as_int());
    }
    else {
        printf("mov_mX_immX: Not valid!\n");
        exit(1);
    }
}

void mov_reg_data(String *out, const char *reg, String const &val)
{
    string_format(out, "\tmov %s, [%s]\n", reg, val.c_str());
}