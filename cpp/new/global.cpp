#include "global.hpp"

#include <cstdio>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "tokenizer.hpp"
#include "ast.hpp"
#include "checker.hpp"
#include "x86_64_nasm_linux.hpp"

void log_error(const char *msg)
{
    printf("\u001b[1m\u001b[31m[Parser]\u001b[0m %s", msg);
    exit(1);
}

String read_file_source(const char *file_path)
{

    std::ifstream inFile;
    inFile.open(file_path); // open the input file

    if (inFile.fail())
    {
        printf("Files doesn't exist! %s\n", file_path);
        exit(1);
    }

    std::stringstream strStream;
    strStream << inFile.rdbuf();       // read the file
    std::string str = strStream.str(); // str holds the content of the file

    return str;
}

Vec<String> glob_with_ext(const std::string &path, const char *ext)
{
    Vec<String> paths;
    for (const auto &p : std::filesystem::recursive_directory_iterator(path))
        if (!std::filesystem::is_directory(p))
            if (p.path().filename().extension() == ext || strcmp(ext, "*") == 0)
                paths.emplace_back(p.path());
    return paths;
}

void parse_input(Project *project, String file_path, String input)
{
    printf("Parsing file: %s\n", file_path.c_str());
    Tokenizer tokenizer(input, file_path);
    tokenizer.run();

    // for (auto token : tokenizer.get_tokens())
    //     tokenizer.print_token(token);

    AST ast(file_path, tokenizer.get_tokens());
    ast.parse();

    auto root = move(ast.get_root_class());
    // printf("%s\n", root.get()->to_json().dump(4).c_str());

    String package = "";
    if (root.get()->p_package.get() != nullptr)
        package = static_cast<const PackageExpression *>(root.get()->p_package.get())->p_path;

    String classpath = package + "$" + root.get()->p_definition.class_name;

    printf("Parsing: %s in %s. Classpath: %s\n", root.get()->p_definition.class_name.c_str(), package.length() == 0 ? "root" : package.c_str(), classpath.c_str());

    if (project->project_classes.contains(classpath))
        log_error("Classpath already defined!\n");

    Vec<VariableDef> class_variables_def = {};
    Vec<MethodDef> class_methods_def = {};

    for (auto &expr : root.get()->p_class_variables)
    {
        auto variable = static_cast<const VariableDeclarationExpression *>(expr.get());
        VariableDef def;
        def.class_name = variable->p_definition.class_name;
        def.arg_name = variable->p_definition.arg_name;
        def.is_final = variable->p_definition.mod.is_final;
        def.is_static = variable->p_definition.mod.is_static;
        def.is_public = variable->p_definition.mod.is_public;
        def.is_protected = false; // Implement is_protected

        auto value = static_cast<const ValueExpression *>(variable->p_value.get());

        Value val;
        val.raw = value->p_value;
        def.val = val;

        class_variables_def.emplace_back(def);
    }

    for (auto &expr : root.get()->p_methods)
    {
        auto method = static_cast<const MethodExpression *>(expr.get());

        MethodDef def;
        def.stack_offset = 0; // fixme 22/10/11: Do it
        def.is_public = method->p_definition.mod.is_public;
        def.is_abstract = method->p_definition.mod.is_abstract;
        def.is_static = method->p_definition.mod.is_static;
        def.return_type = method->p_definition.class_name;
        def.method_name = method->p_definition.arg_name;

        Vec<ArgumentDef> method_args = {};

        for (auto &arg : method->p_args)
        {
            ArgumentDef arg_def;
            arg_def.arg_name = arg.arg_name;
            arg_def.class_name = arg.class_name;
            arg_def.is_final = arg.mod.is_final;

            method_args.emplace_back(arg_def);
        }

        Vec<MethodVar> stack_vars = {};

        for (auto &body_expr : method->p_body)
        {
            if (auto var_expr = dynamic_cast<const VariableDeclarationExpression *>(body_expr.get()))
            {
                VariableDef var_def;
                var_def.arg_name = var_expr->p_definition.arg_name;
                var_def.class_name = var_expr->p_definition.class_name;
                auto value = static_cast<const ValueExpression *>(var_expr->p_value.get());

                Value val;
                val.raw = value->p_value;
                var_def.val = val;

                StackVar var;
                var.alias = var_expr->p_definition.arg_name;
                var.stack_offset = def.stack_offset;


                MethodVar method_var_def;
                method_var_def.is_final = false; //fixme 22/10/14: This should be handled.
                method_var_def.stack_var = var;
                method_var_def.var_def = var_def;

                stack_vars.emplace_back(method_var_def);
            }
        }

        def.stack_vars = stack_vars;
        def.args = method_args;

        for (auto method_def : class_methods_def)
        {
            // fixme 22/10/11: Check args as well
            if (method_def.method_name == def.method_name)
                log_error("Method name already exits.");
        }
        class_methods_def.emplace_back(def);
    }

    ClassDef def = {
        .class_methods = class_methods_def,
        .class_variables = class_variables_def,
        .in_file = file_path,
    };

    project->project_classes[classpath] = def;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)*argv++; // advance program name
    const char *path = *argv;

    Project project;

    // for (auto globpath : glob_with_ext(path, ".nava"))
    // {
    //     parse_input(&project, globpath, read_file_source(globpath.c_str()));
    // }

    parse_input(&project, "../examples/global_var.nava", read_file_source("../examples/global_var.nava"));

    for (auto clazz : project.project_classes)
    {
        printf("Class %s:\n", clazz.first.c_str());
        for (auto var_def : clazz.second.class_variables)
        {
            printf("\tVarName:%s, VarClass:%s VarValueRaw:%s\n",
                   var_def.arg_name.c_str(), var_def.class_name.c_str(), var_def.val.raw.c_str());
        }
        for (auto method_def : clazz.second.class_methods)
        {
            printf("\tMethodName:%s, RetType:%s ArgCount:%ld\n",
                   method_def.method_name.c_str(), method_def.return_type.c_str(), method_def.args.size());
            for (auto var : method_def.stack_vars)
            {
                printf("\t\tVarName:%s, VarClass:%s VarValueRaw:%s\n",
                   var.var_def.arg_name.c_str(), var.var_def.class_name.c_str(), var.var_def.val.raw.c_str());
                printf("\t\t\tStackVar:%s Offset:%ld\n", var.stack_var.alias.c_str(), var.stack_var.stack_offset);
            }
        }
    }

    Checker checker(&project);
    checker.start_checking();

    CodeGenerator gen(&project);
    gen.generate();
}