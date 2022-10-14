#pragma once

#include "global.hpp"

class CodeGenerator
{
public:
    CodeGenerator(Project *proj)
        : m_project(proj) {}

    void generate();

private:
    Project *m_project;
};