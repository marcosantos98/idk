#include "checker.hpp"

#include <stdarg.h>
#include <string_view>
#include <ranges>

#include "nava.hpp"

void Checker::find_and_set_main()
{
    for(auto classs : m_project->project_classes)
    {
        for(auto method : classs.second.class_methods)
        {
            if(method.is_static && method.is_public && method.return_type == "void" && method.method_name == "main")
            {
                m_project->main_class = classs.first;
                break;
            }
        }
    }
}

void Checker::start_checking()
{
    find_and_set_main();
    for (auto classs : m_project->project_classes)
    {
        m_current_path = classs.second.in_file;
        m_logger->log_tdbg("Checking: %s ", m_current_path.c_str());
        for (auto variables : classs.second.class_variables)
        {
            if (variables.class_name == "int" || variables.class_name == "double" || variables.class_name == "long")
            {
                check_numbers(variables);
            }
        }

        // for (auto method : classs.second.class_methods)
        // {
        //     for (auto method_expr : method.method_expressions)
        //     {
        //         if (method_expr.type == MethodExprType::FUNCALL && method_expr.func_def.call_name != "asm")
        //         {
        //             if (VEC_HAS(classs.second.imports, string_split(method_expr.func_def.call_name, '.')[0]))
        //                 continue;
        //             else
        //                 m_logger->log_err("Class doesn't import %s\n", string_split(method_expr.func_def.call_name, '.')[0].c_str());
        //         }
        //     }
        // }

        m_logger->log_ok("OK!\n");
    }
}

void Checker::check_numbers(VariableDef const &def)
{
    if (def.class_name == "int")
    {
        try
        {
            Value val = def.val;
            val.as_int();
        }
        catch (std::invalid_argument const &e)
        {
            m_logger->log_err("Invalid argument: Expected int value got %s\n", def.val.raw.c_str());
        }
        catch (std::out_of_range const &e)
        {
            m_logger->log_err("Value is out of range for %s. Value: %s\n", def.class_name.c_str(), def.val.raw.c_str());
        }
    }
    else if (def.class_name == "double")
    {
        try
        {
            Value val = def.val;
        }
        catch (std::invalid_argument const &e)
        {
            m_logger->log_err("Invalid argument: Expected int value got %s\n", def.val.raw.c_str());
        }
        catch (std::out_of_range const &e)
        {
            m_logger->log_err("Value is out of range for %s. Value: %s\n", def.class_name.c_str(), def.val.raw.c_str());
        }
    }
    else if (def.class_name == "long")
    {
        try
        {
            Value val = def.val;
            printf("%ld\n", val.as_long());
        }
        catch (std::invalid_argument const &e)
        {
            m_logger->log_err("Invalid argument: Expected int value got %s\n", def.val.raw.c_str());
        }
        catch (std::out_of_range const &e)
        {
            m_logger->log_err("Value is out of range for %s. Value: %s\n", def.class_name.c_str(), def.val.raw.c_str());
        }
    }
    else
    {
        printf("%s\n", def.class_name.c_str());
    }
}