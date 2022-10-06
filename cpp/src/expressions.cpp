#include "expressions.hpp"

Json NumberLiteralExpression::to_json()
{
    Json json;
    json["type"] = "NumberLiteralExpression";
    json["value"] = p_value;
    return json;
}

Json StringLiteralExpression::to_json()
{
    Json json;
    json["type"] = "StringLiteralExpression";
    json["value"] = p_value;
    return json;
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

Json VariableExpression::to_json()
{
    Json json;

    json["type"] = "VariableExpression";
    json["var_name"] = p_var_name;

    return json;
}
Json VariableDeclarationExpression::to_json()
{
    Json json = def_to_json(p_definition);

    json["type"] = "VariableDeclarationExpression";
    json["value"] = p_value.get()->to_json();

    return json;
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
    
    return json;
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