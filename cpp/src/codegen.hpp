#pragma once

#include "nava.hpp"
#include "ast.hpp"

class CodeGenerator
{
public:
    CodeGenerator(OwnPtr<ClassExpression> root_class, OwnPtrVec<Expression> imports) 
        : m_root_class(std::move(root_class)), m_imports(move(imports)) {}

    void run();
   
    String get_final_out() const
    {
        return m_final_out;
    }

private:
    std::string m_text_section = "segment .text\n";
    std::string m_data_section = "segment .data\n";
    std::string m_final_out;
    std::string m_externs;
    std::string m_globals;

    Map<String, String> m_class_vars = {};

    OwnPtr<ClassExpression> m_root_class;
    OwnPtrVec<Expression> m_imports;

    void parse_class_variables();
    void parse_class_methods();
};