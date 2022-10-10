#pragma once

#include "nava.hpp"
#include "tokenizer.hpp"
#include "expressions.hpp"

class CodeGenerator
{

public:
    CodeGenerator(OwnPtr<ClassExpression> root)
        : m_root_class(move(root)) {}

    String generate();
private:
    OwnPtr<ClassExpression> m_root_class;

    bool has_main();
};