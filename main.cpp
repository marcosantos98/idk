#include "global.hpp"

#include <cstdio>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <list>
#include <time.h>

#include "debug.hpp"
#include "tokenizer.hpp"
#include "ast.hpp"
#include "checker.hpp"
#include "x86_64_nasm_linux.hpp"
#include "argparser.hpp"
#include "log.hpp"

Log logger("Project");

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

MethodExpr parse_binop(const BinaryExpression *binop)
{

    BinopDef def;

    auto left = static_cast<const ValueExpression *>(binop->p_lhs.get());

    Value left_val;
    left_val.raw = left->p_value;
    left_val.type = left->p_type;

    auto right = static_cast<const ValueExpression *>(binop->p_rhs.get());

    Value right_val;
    right_val.raw = right->p_value;
    right_val.type = right->p_type;

    def.left = left_val;
    def.rigth = right_val;

    def.op = binop->p_op;

    MethodExpr method_expr;
    method_expr.is_final = false;
    method_expr.binop_def = def;
    method_expr.type = MethodExprType::BINOP;

    return method_expr;
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
    logger.log_tdbg("Parsing file: %s\n", file_path.c_str());
    Tokenizer tokenizer(input, file_path);
    tokenizer.run();

    // for (auto token : tokenizer.get_tokens())
    //     tokenizer.print_token(token);

    AST ast(file_path, tokenizer.get_tokens());
    ast.parse();

    auto root = ast.get_root_class();
    auto imports = ast.get_imports();
    // printf("%s\n", root.get()->to_json().dump(4).c_str());

    String package = "";
    if (root.get()->p_package.get() != nullptr)
        package = static_cast<const PackageExpression *>(root.get()->p_package.get())->p_path;

    String classpath = package + "$" + root.get()->p_definition.class_name;

    logger.log_tdbg("%s in %s. Classpath: %s\n", root.get()->p_definition.class_name.c_str(), package.length() == 0 ? "root" : package.c_str(), classpath.c_str());

    if (project->project_classes.contains(classpath))
        logger.log_err("Classpath already defined!\n");

    Vec<VariableDef> class_variables_def = {};
    Vec<MethodDef> class_methods_def = {};
    Vec<String> class_imports = {};

    for (auto &imp : imports)
    {
        auto import = static_cast<const ImportExpression *>(imp.get());

        class_imports.emplace_back(import->p_path);
    }

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
        val.type = value->p_type;
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

        Vec<MethodExpr> stack_vars = {};

        for (auto &body_expr : method->p_body)
        {
            if (auto var_expr = dynamic_cast<const VariableDeclarationExpression *>(body_expr.get()))
            {

                VariableDef var_def;
                var_def.arg_name = var_expr->p_definition.arg_name;
                var_def.class_name = var_expr->p_definition.class_name;

                size_t type_offset = NAVA::primitive_byte_sizes[var_def.class_name];
                def.stack_offset += type_offset;

                StackVar var;
                var.alias = var_expr->p_definition.arg_name;
                var.stack_offset = def.stack_offset;

                if (auto value = dynamic_cast<const ValueExpression *>(var_expr->p_value.get()))
                {

                    Value val;
                    val.raw = value->p_value;
                    val.type = value->p_type;
                    var_def.val = val;

                    // fixme 22/10/15: Check if is a primitive before retrieving the actual value.

                    MethodExpr method_var_def;
                    method_var_def.type = MethodExprType::VAR;
                    method_var_def.is_final = false; // fixme 22/10/14: This should be handled.
                    method_var_def.var_def = std::make_tuple(var_def, var);

                    stack_vars.emplace_back(method_var_def);
                }
                else if (auto binop = dynamic_cast<const BinaryExpression *>(var_expr->p_value.get()))
                {
                    MethodExpr expr = parse_binop(binop);
                    expr.var_def = std::make_tuple(var_def, var);
                    stack_vars.emplace_back(expr);
                }
            }
            else if (auto call_expr = dynamic_cast<const CallExpression *>(body_expr.get()))
            {
                FuncallDef funcall_def;
                funcall_def.call_name = call_expr->p_method_name;
                funcall_def.ret_type = "void"; // fixme 22/10/15: Implement this.

                Vec<Value> arguments = {};

                for (auto &arg : call_expr->p_args)
                {
                    // fixme 22/10/15: This should check what is the type of the expression
                    auto val_exp = static_cast<const ValueExpression *>(arg.get());

                    Value val;
                    val.raw = val_exp->p_value;
                    val.type = val_exp->p_type;

                    arguments.emplace_back(val);
                }

                funcall_def.args = arguments;

                MethodExpr method_exp;
                method_exp.is_final = false;
                method_exp.type = MethodExprType::FUNCALL;
                method_exp.func_def = funcall_def;

                stack_vars.emplace_back(method_exp);
            }
            else if (auto if_exp = dynamic_cast<const IfExpression *>(body_expr.get()))
            {
                IfDef if_def;

                if (auto val_exp = dynamic_cast<const ValueExpression *>(if_exp->p_condition.get()))
                {
                    Value val;
                    val.raw = val_exp->p_value;
                    val.type = val_exp->p_type;

                    CondDef def;
                    def.val = val;

                    if_def.cond = def;
                    if_def.type = CondType::VAL;
                }
                else if (auto binop_exp = dynamic_cast<const BinaryExpression *>(if_exp->p_condition.get()))
                {
                    auto exp = parse_binop(binop_exp);
                    CondDef def;
                    def.binop = exp.binop_def;
                    if_def.type = CondType::BINOP;
                    if_def.cond = def;
                }

                Vec<MethodExpr> body = {};

                for (auto &exp : if_exp->p_body)
                {
                    if (auto call_expr = dynamic_cast<const CallExpression *>(exp.get()))
                    {
                        FuncallDef funcall_def;
                        funcall_def.call_name = call_expr->p_method_name;
                        funcall_def.ret_type = "void"; // fixme 22/10/15: Implement this.

                        Vec<Value> arguments = {};

                        for (auto &arg : call_expr->p_args)
                        {
                            // fixme 22/10/15: This should check what is the type of the expression
                            auto val_exp = static_cast<const ValueExpression *>(arg.get());

                            Value val;
                            val.raw = val_exp->p_value;
                            val.type = val_exp->p_type;

                            arguments.emplace_back(val);
                        }

                        funcall_def.args = arguments;

                        MethodExpr method_exp;
                        method_exp.is_final = false;
                        method_exp.type = MethodExprType::FUNCALL;
                        method_exp.func_def = funcall_def;

                        body.emplace_back(method_exp);
                    }
                }

                if_def.body_expr = body;

                MethodExpr method_expr;
                method_expr.is_final = false;
                method_expr.type = MethodExprType::IF;
                method_expr.if_def = if_def;

                stack_vars.emplace_back(method_expr);
            }
            else if (auto while_exp = dynamic_cast<const WhileExpression *>(body_expr.get()))
            {

                WhileDef while_def;

                if (auto val_exp = dynamic_cast<const ValueExpression *>(while_exp->p_condition.get()))
                {
                    Value val;
                    val.raw = val_exp->p_value;
                    val.type = val_exp->p_type;

                    CondDef def;
                    def.val = val;

                    while_def.cond = def;
                    while_def.type = CondType::VAL;
                }
                else if (auto binop_exp = dynamic_cast<const BinaryExpression *>(while_exp->p_condition.get()))
                {
                    auto exp = parse_binop(binop_exp);
                    CondDef def;
                    def.binop = exp.binop_def;
                    while_def.type = CondType::BINOP;
                    while_def.cond = def;
                }

                Vec<MethodExpr> body = {};

                for (auto &exp : while_exp->p_body)
                {
                    if (auto call_expr = dynamic_cast<const CallExpression *>(exp.get()))
                    {
                        FuncallDef funcall_def;
                        funcall_def.call_name = call_expr->p_method_name;
                        funcall_def.ret_type = "void"; // fixme 22/10/15: Implement this.

                        Vec<Value> arguments = {};

                        for (auto &arg : call_expr->p_args)
                        {
                            // fixme 22/10/15: This should check what is the type of the expression
                            auto val_exp = static_cast<const ValueExpression *>(arg.get());

                            Value val;
                            val.raw = val_exp->p_value;
                            val.type = val_exp->p_type;

                            arguments.emplace_back(val);
                        }

                        funcall_def.args = arguments;

                        MethodExpr method_exp;
                        method_exp.is_final = false;
                        method_exp.type = MethodExprType::FUNCALL;
                        method_exp.func_def = funcall_def;

                        body.emplace_back(method_exp);
                    }
                    else if (auto binop_expr = dynamic_cast<const BinaryExpression *>(exp.get()))
                    {
                        auto expr = parse_binop(binop_expr);
                        body.emplace_back(expr);
                    }
                    else if (auto var_expr = dynamic_cast<const VariableDeclarationExpression *>(exp.get()))
                    {

                        VariableDef var_def;
                        var_def.arg_name = var_expr->p_definition.arg_name;
                        var_def.class_name = var_expr->p_definition.class_name;

                        size_t type_offset = NAVA::primitive_byte_sizes[var_def.class_name];
                        def.stack_offset += type_offset;

                        StackVar var;
                        var.alias = var_expr->p_definition.arg_name;
                        var.stack_offset = def.stack_offset;

                        if (auto value = dynamic_cast<const ValueExpression *>(var_expr->p_value.get()))
                        {

                            Value val;
                            val.raw = value->p_value;
                            val.type = value->p_type;
                            var_def.val = val;

                            // fixme 22/10/15: Check if is a primitive before retrieving the actual value.

                            MethodExpr method_var_def;
                            method_var_def.type = MethodExprType::VAR;
                            method_var_def.is_final = false; // fixme 22/10/14: This should be handled.
                            method_var_def.var_def = std::make_tuple(var_def, var);

                            body.emplace_back(method_var_def);
                        }
                        else if (auto binop = dynamic_cast<const BinaryExpression *>(var_expr->p_value.get()))
                        {
                            MethodExpr expr = parse_binop(binop);
                            expr.var_def = std::make_tuple(var_def, var);
                            body.emplace_back(expr);
                        }
                    }
                    else if (auto assign_expr = dynamic_cast<const AssignExpression *>(exp.get()))
                    {
                        AssignDef assign_def;
                        assign_def.alias = assign_expr->p_alias;

                        if (auto binop = dynamic_cast<const BinaryExpression *>(assign_expr->p_value.get()))
                        {

                            MethodExpr expr = parse_binop(binop);
                            std::optional<std::tuple<VariableDef, StackVar>> var_def;

                            for (auto method_exp : stack_vars)
                            {
                                if (method_exp.type == MethodExprType::VAR && std::get<0>(method_exp.var_def.value()).arg_name == assign_def.alias)
                                {
                                    var_def = method_exp.var_def;
                                    break;
                                }
                            }

                            expr.var_def = var_def;

                            assign_def.val = {expr};
                        }
                        else if (auto value_expr = dynamic_cast<const ValueExpression *>(assign_expr->p_value.get()))
                        {
                            Value val;
                            val.raw = value_expr->p_value;
                            val.type = value_expr->p_type;

                            MethodExpr method_var_def;
                            method_var_def.type = MethodExprType::VAR;
                            
                            std::optional<std::tuple<VariableDef, StackVar>> var_def;

                            for (auto method_exp : stack_vars)
                            {
                                if (method_exp.type == MethodExprType::VAR && std::get<0>(method_exp.var_def.value()).arg_name == assign_def.alias)
                                {
                                    var_def = method_exp.var_def;
                                    break;
                                }
                            }

                            std::get<0>(var_def.value()).val = val;

                            method_var_def.var_def = var_def;

                            assign_def.val = {method_var_def};
                        }
                        else
                        {
                            logger.log_terr("Not implemented yet! %s\n", assign_expr->p_value.get()->to_json()["type"].dump().c_str());
                        }

                        MethodExpr method_expr;
                        method_expr.type = MethodExprType::ASSIGN;
                        method_expr.assign_def = assign_def;

                        body.emplace_back(method_expr);
                    }
                }

                while_def.body_expr = body;

                MethodExpr method_expr;
                method_expr.type = MethodExprType::WHILE;
                method_expr.while_def = while_def;

                stack_vars.emplace_back(method_expr);
            }
            else if (auto assign_expr = dynamic_cast<const AssignExpression *>(body_expr.get()))
            {
                logger.log_terr("ASSGIN IN Method: Not implemented yet!\n");
            }
        }

        def.method_expressions = stack_vars;
        def.args = method_args;

        for (auto method_def : class_methods_def)
        {
            // fixme 22/10/11: Check args as well
            if (method_def.method_name == def.method_name)
                logger.log_err("Method name already exits.");
        }
        class_methods_def.emplace_back(def);
    }

    ClassDef def = {
        .class_methods = class_methods_def,
        .class_variables = class_variables_def,
        .imports = class_imports,
        .in_file = file_path,
    };

    project->project_classes[classpath] = def;
}

int main(int argc, char **argv)
{
    argparse::ArgumentParser program("nava");
    program.add_argument("-s")
        .nargs(1)
        .help("Compile single file.");
    program.add_argument("-p")
        .help("Compile project in path")
        .nargs(1);
    program.add_argument("-i")
        .help("List of files to include when compiling a single file.")
        .nargs(argparse::nargs_pattern::any);
    program.add_argument("-dp")
        .default_value(false)
        .implicit_value(true)
        .help("Analyse the project and print the current project contents. Exits after.");
    program.add_argument("-r")
        .help("Set root path")
        .nargs(1);

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    if (argc == 1)
    {
        std::cerr << program;
        std::exit(1);
    }

    Project project;

    if (program.present<String>("-r"))
    {
        project.root_path = program.get<String>("-r");
    }

    auto parse_start = std::chrono::high_resolution_clock::now();

    if (program.present("-s"))
    {
        if (!program.present<String>("-r"))
            project.root_path = program.get<String>("-s");
        project.single_file = true;
        parse_input(&project, program.get<String>("-s"), read_file_source(program.get<String>("-s").c_str()));

        if (program.present("-i"))
        {
            auto files = program.get<Vec<String>>("-i");
            for (auto s : files)
            {
                parse_input(&project, s, read_file_source(s.c_str()));
            }
        }
    }
    else if (program.present<String>("-p"))
    {
        if (!program.present<String>("-r"))
            project.root_path = program.get<String>("-p");
        for (auto globpath : glob_with_ext(program.get<String>("-p"), ".nava"))
        {
            parse_input(&project, globpath, read_file_source(globpath.c_str()));
        }
    }

    auto parse_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - parse_start);

    if (program.get<bool>("-dp"))
    {
        print_project(project);
        exit(0);
    }

    auto checker_start = std::chrono::high_resolution_clock::now();

    Checker checker(&project);
    checker.start_checking();

    auto checker_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - checker_start);

    auto gen_start = std::chrono::high_resolution_clock::now();

    CodeGenerator gen(&project);
    gen.generate();

    auto gen_duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - gen_start);

    printf("\n");
    logger.log_tdbg("Execution time:\n");
    logger.log_info("Parser: %ldms\n", parse_duration);
    logger.log_info("Checker: %ldms\n", checker_duration);
    logger.log_info("Generator: %ldms\n", gen_duration);
    printf("\n");
}