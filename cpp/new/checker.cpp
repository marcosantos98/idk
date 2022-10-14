#include "checker.hpp"

#include <stdarg.h>

#include "nava.hpp"

void Checker::start_checking()
{
    for (auto classs : m_project->project_classes)
    {
        m_current_path = classs.second.in_file;
        printf("Checking: %s\n", m_current_path.c_str());
        for (auto variables : classs.second.class_variables)
        {
            if (variables.class_name == "int" || variables.class_name == "double" || variables.class_name == "long")
            {
                check_numbers(variables);
            }
        }
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
            log_error("Invalid argument: Expected int value got %s\n", def.val.raw.c_str());
        }
        catch (std::out_of_range const &e)
        {
            log_error("Value is out of range for %s. Value: %s\n", def.class_name.c_str(), def.val.raw.c_str());
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
            log_error("Invalid argument: Expected int value got %s\n", def.val.raw.c_str());
        }
        catch (std::out_of_range const &e)
        {
            log_error("Value is out of range for %s. Value: %s\n", def.class_name.c_str(), def.val.raw.c_str());
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
            log_error("Invalid argument: Expected int value got %s\n", def.val.raw.c_str());
        }
        catch (std::out_of_range const &e)
        {
            log_error("Value is out of range for %s. Value: %s\n", def.class_name.c_str(), def.val.raw.c_str());
        }
    }
    else
    {
        printf("%s\n", def.class_name.c_str());
    }
}

void Checker::log_error(const char *msg, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, msg);
    (void)vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);
    printf("\u001b[1m\u001b[31m[Checker:%s]\u001b[0m ", m_current_path.c_str());
    printf(buffer);
    exit(1);
}