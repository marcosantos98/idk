#pragma once

#include "global.hpp"
#include "log.hpp"

class Checker
{
public:
    Checker(Project *project)
        : m_project(project) {}

    ~Checker() { delete m_logger; }

    void start_checking();

private:
    Project *m_project;
    String m_current_path;
    Log *m_logger = new Log("Checker");

    void find_and_set_main();
    void check_numbers(VariableDef const &);
};