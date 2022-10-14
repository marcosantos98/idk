#include "expressions.hpp"

Json ValueExpression::to_json()
{
    Json json;
    json["type"] = "ValueExpression";
    json["value"] = p_value;
    return json;
}

String ValueExpression::code_gen(NAVA::GlobalContext *ctx)
{
    char buf[1024];
    // snprintf(buf, 1024, "\t%s_%s db \"%s\", 10\n", ctx->current_def.class_name.c_str(), ctx->current_def.arg_name.c_str(), p_value.c_str());
    ctx->data_section.append("\t;; Value expression\n");
    return "";
}

Json BinaryExpression::to_json()
{
    Json json;
    json["type"] = "BinaryExpression";
    String op = {p_op};
    json["operator"] = op;
    json["lhs"] = p_lhs.get()->to_json();
    json["rhs"] = p_rhs.get()->to_json();
    return json;
}

String BinaryExpression::code_gen(NAVA::GlobalContext *ctx)
{

    return "";
}

Json VariableExpression::to_json()
{
    Json json;

    json["type"] = "VariableExpression";
    json["var_name"] = p_var_name;

    return json;
}

String VariableExpression::code_gen(NAVA::GlobalContext *ctx)
{
    char buf[1024];
    snprintf(buf, 1024, "mov eax, %d", 0);
    return {buf};
}

Json VariableDeclarationExpression::to_json()
{
    Json json = def_to_json(p_definition);

    json["type"] = "VariableDeclarationExpression";
    json["value"] = p_value.get()->to_json();

    return json;
}

String VariableDeclarationExpression::code_gen(NAVA::GlobalContext *ctx)
{

    ctx->current_def = p_definition;
    p_value.get()->code_gen(ctx);
    return "";
}

Json MethodExpression::to_json()
{
    Json json = def_to_json(p_definition);

    json["type"] = "MethodExpression";

    Json args_arr = nlohmann::json::array();

    for (auto arg : p_args)
        args_arr.emplace_back(def_to_json(arg));

    json["args"] = args_arr;

    Json body_arr = nlohmann::json::array();

    for (auto &expr : p_body)
        body_arr.emplace_back(expr->to_json());

    json["body"] = body_arr;

    return json;
}

String MethodExpression::code_gen(NAVA::GlobalContext *ctx)
{
    ctx->text_section.append("global ").append(ctx->class_name).append("$").append(p_definition.arg_name).append("\n");
    ctx->text_section.append(ctx->class_name).append("$").append(p_definition.arg_name).append(":\n");
    ctx->text_section.append("\tpush rbp\n");
    ctx->text_section.append("\tmov rbp, rsp\n");

    bool has_calls = false;
    for (auto &body : p_body)
    {
        if (dynamic_cast<const CallExpression *>(body.get()) != nullptr)
        {
            has_calls = true;
            break;
        }
    }

    if (has_calls)
        ctx->text_section.append("\tsub rsp, 16\n");

    for (auto &body : p_body)
    {
        body.get()->code_gen(ctx);
    }

    if (has_calls)
    {
        ctx->text_section.append("\tleave\n");
    }
    else
    {
        ctx->text_section.append("\tpop rbp\n");
        ctx->text_section.append("\tret\n");
    }

    ctx->text_section.append("\tret\n");

    return "";
}

Json CallExpression::to_json()
{
    Json json;

    json["type"] = "CallExpression";

    Json args = nlohmann::json::array();

    for (auto &arg : p_args)
    {
        args.emplace_back(arg.get()->to_json());
    }

    json["args"] = args;

    json["method_name"] = p_method_name;

    return json;
}

String CallExpression::code_gen(NAVA::GlobalContext *ctx)
{

    return "";
}

Json ImportExpression::to_json()
{
    Json json;

    json["type"] = "ImportExpression";
    json["path"] = p_path;

    return json;
}

String ImportExpression::code_gen(NAVA::GlobalContext *ctx)
{
    char buf[1024];
    snprintf(buf, 1024, "mov eax, %d", 0);
    return {buf};
}

Json IfExpression::to_json()
{
    Json json;

    json["type"] = "IfExpression";
    json["condition"] = p_condition.get()->to_json();

    Json body_arr = nlohmann::json::array();

    for (auto &expr : p_body)
        body_arr.emplace_back(expr->to_json());

    json["body"] = body_arr;

    return json;
}

String IfExpression::code_gen(NAVA::GlobalContext *ctx)
{

    return "";
}

Json WhileExpression::to_json()
{
    Json json;

    json["type"] = "WhileExpression";
    json["condition"] = p_condition.get()->to_json();

    Json body_arr = nlohmann::json::array();

    for (auto &expr : p_body)
        body_arr.emplace_back(expr->to_json());

    json["body"] = body_arr;

    return json;
}

String WhileExpression::code_gen(NAVA::GlobalContext *ctx)
{
    if (auto binop = dynamic_cast<const BinaryExpression *>(p_condition.get()))
    {
        p_condition.get()->code_gen(ctx);
        ctx->text_section.append("\tje ").append(".JL_").append(std::to_string(ctx->label_cnt)).append("\n");
    }

    for (auto &body : p_body)
    {
        body.get()->code_gen(ctx);
    }

    ctx->text_section.append(".JL_").append(std::to_string(ctx->label_cnt++)).append(":\n");

    return "";
}

Json ClassExpression::to_json()
{
    Json json = def_to_json(p_definition);

    json["type"] = "ClassExpression";

    Json variables = nlohmann::json::array();

    for (auto &variable : p_class_variables)
        variables.emplace_back(variable->to_json());

    json["variables"] = variables;

    Json methods = nlohmann::json::array();

    for (auto &method : p_methods)
        methods.emplace_back(method->to_json());

    json["methods"] = methods;

    return json;
}

String ClassExpression::code_gen(NAVA::GlobalContext *ctx)
{
    char buf[1024];
    snprintf(buf, 1024, "mov eax, %d", 0);
    return {buf};
}

Json PackageExpression::to_json()
{
    Json json;
    json["type"] = "ClassExpression";
    json["path"] = p_path;
    return json;
}

String PackageExpression::code_gen(NAVA::GlobalContext *ctx)
{
    return "";
}