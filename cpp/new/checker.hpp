#pragma once

#include "global.hpp"

class Checker
{
public:
    Checker(Project *project)
        : m_project(project) {}

    void start_checking();

private:
    Project *m_project;
    String m_current_path;

    void find_and_set_main();
    void check_numbers(VariableDef const&);
    void log_error(const char*, ...);
};