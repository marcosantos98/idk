#include "x86_64_nasm_linux.hpp"

#include <memory>
#include <string>
#include <stdexcept>
#include <stdarg.h>

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

void CodeGenerator::generate()
{
    String final_out = "";
    String text_section = "section .text\n";
    String data_section = "section .data\n";

    for (auto def : m_project->project_classes)
    {
        printf("Generating NASM x86_64 Linux for: %s\n", def.second.in_file.c_str());
        for (auto var : def.second.class_variables)
        {
            if (var.class_name == "int")
            {
                data_section.append(string_format("\t%s dd %d\n", var.arg_name.c_str(), var.val.as_int()));
            }
            else if (var.class_name == "double")
            {
                data_section.append(string_format("\t%s dq %1f\n", var.arg_name.c_str(), var.val.as_double()));
            }
        }

        for(auto method : def.second.class_methods)
        {
            if(method.is_static || method.is_public)
                text_section.append(string_format("extern %s$%s\n", def.first.c_str(), method.method_name.c_str()));
        
            text_section.append(string_format("%s$%s:\n", def.first.c_str(), method.method_name.c_str()));

            text_section.append("\tpush rbp\n");
            text_section.append("\tmov rbp, rsp\n");

            //fixme 22/10/14: pop args from stack

            for(auto stack : method.stack_vars)
            {
                text_section.append(string_format("\tmov dword [rbp-%d], %d\n", stack.stack_var.stack_offset, stack.var_def.val.as_int()));
            }
        }
        
        if(def.first == m_project->main_class)
        {
            text_section.append("global _start\n");
            text_section.append("_start:\n");
            text_section.append(string_format("\tcall %s$main\n", def.first.c_str()));
        }
    }

    final_out.append(text_section).append(data_section);

    printf("%s\n", final_out.c_str());
}
