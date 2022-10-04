#pragma once

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

#include "tokenizer.hpp"
#include "json.hpp"

enum class Modifiers
{
    PUBLIC,
    PRIVATE
};

class Expression
{
public:
    virtual ~Expression() = default;

    virtual nlohmann::json to_str() = 0;
};

class NumberLiteral : public Expression
{
public:
    double value;
    NumberLiteral(double val) : value(val) {}

    nlohmann::json to_str() override
    {
        nlohmann::json json;
        json["type"] = "NumberLiteral";
        json["value"] = value;
        return json;
    }
};

class StringLiteral : public Expression
{
public:
    std::string value;
    StringLiteral(std::string val) : value(val) {}

    nlohmann::json to_str() override
    {
        nlohmann::json json;
        json["type"] = "StringLiteral";
        json["value"] = value;
        return json;
    }
};

class VariableDeclarationExpression : public Expression
{
public:
    std::string class_name;
    std::string variable_name;
    std::unique_ptr<Expression> value;
    VariableDeclarationExpression(std::string name, std::string var_name, std::unique_ptr<Expression> val)
        : class_name(name), variable_name(var_name), value(std::move(val)) {}

    nlohmann::json to_str() override
    {
        nlohmann::json json;
        json["type"] = "Variable Declaration";
        json["name"] = variable_name;
        json["class"] = class_name;
        json["value"] = value.get()->to_str();
        return json;
    }
};

class BinaryExpression : public Expression
{
public:
    char op;
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;

    BinaryExpression(char op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : op(op), lhs(std::move(left)), rhs(std::move(right)) {}

    nlohmann::json to_str() override
    {
        nlohmann::json json;
        json["type"] = "Binary Expression";
        json["operator"] = op;
        json["lhs"] = lhs.get()->to_str();
        json["rhs"] = rhs.get()->to_str();
        return json;
    }
};

class MethodDefinitionExpression : public Expression
{
public:
    std::vector<std::unique_ptr<Expression>> body;
    std::string method_name;
    Modifiers modifier;
    bool is_static;
    std::string return_type;
    std::map<std::string, std::string> args;

    MethodDefinitionExpression(std::string method_name, std::string ret_type, Modifiers mods, bool statics, std::vector<std::unique_ptr<Expression>> body, std::map<std::string, std::string> arguments)
        : body(std::move(body)), method_name(method_name), modifier(mods), is_static(statics), return_type(ret_type), args(arguments) {}

    nlohmann::json to_str() override
    {
        nlohmann::json json;
        json["type"] = "Method Definition";
        json["static"] = is_static;
        json["name"] = method_name;
        json["return"] = return_type;
        json["modifier"] = modifier == Modifiers::PUBLIC ? "PUBLIC" : "PRIVATE";

        nlohmann::json args_arr = nlohmann::json::array();

        for(auto arg : args)
        {
            nlohmann::json obj;
            obj["class"] = arg.first;
            obj["arg_name"] = arg.second;

            args_arr.emplace_back(obj);
        }

        json["args"] = args_arr;

        nlohmann::json body_arr = nlohmann::json::array();

        for (auto &variable : body)
        {
            body_arr.emplace_back(variable->to_str());
        }

        json["body"] = body_arr;

        return json;
    }
};

class ClassDefinitionExpression : public Expression
{
public:
    std::vector<std::unique_ptr<Expression>> variables;
    std::vector<std::unique_ptr<Expression>> methods;
    Modifiers modifier;
    bool is_static;
    std::string class_name;

    ClassDefinitionExpression(std::vector<std::unique_ptr<Expression>> vars, std::vector<std::unique_ptr<Expression>> methods, Modifiers mod, bool is_static, std::string class_name)
        : variables(std::move(vars)), methods(std::move(methods)), modifier(mod), is_static(is_static), class_name(class_name) {}

    nlohmann::json to_str() override
    {
        nlohmann::json json;
        json["type"] = "Class Definition";
        json["static"] = is_static;
        json["class"] = class_name;
        json["modifier"] = modifier == Modifiers::PUBLIC ? "PUBLIC" : "PRIVATE";

        nlohmann::json variables_arr = nlohmann::json::array();
        nlohmann::json methods_arr = nlohmann::json::array();

        for (auto &variable : variables)
            variables_arr.emplace_back(variable->to_str());

        json["variables"] = variables_arr;

        for (auto &method : methods)
            methods_arr.emplace_back(method->to_str());

        json["methods"] = methods_arr;

        return json;
    }
};

class ASTBuilder
{
public:
    ASTBuilder(std::vector<Token> tokens) : m_tokens(tokens) {}

    void run();
    
    std::string to_json_str();

    std::vector<std::unique_ptr<Expression>> get_expression()
    {
        return std::move(m_expressions);
    }

private:
    std::map<char, int> m_op_precedence = {
        {'+', 20},
        {'-', 20},
        {'%', 30},
        {'*', 30},
        {'/', 30},
    };

    std::vector<std::string> m_keywords = {
        "public",
        "static",
        "private",
        "class",
    };

    std::vector<Token> m_tokens;
    std::vector<std::unique_ptr<Expression>> m_expressions = {};
    u_int64_t m_current_token = 0;

    Modifiers m_current_mod = Modifiers::PRIVATE;
    bool has_static = false;
    std::string m_current_class_name;

    void advance_token();
    int get_token_precedence();

    std::unique_ptr<ClassDefinitionExpression> parse_class_definition();
    std::unique_ptr<Expression> parse_primary();
    std::unique_ptr<Expression> parse_expression();
    std::unique_ptr<NumberLiteral> parse_number_literal();
    std::unique_ptr<StringLiteral> parse_string_literal();
    std::unique_ptr<MethodDefinitionExpression> parse_method();
    std::unique_ptr<VariableDeclarationExpression> parse_variable_declaration();
    std::unique_ptr<Expression> try_parse_identifer();
    std::unique_ptr<Expression> parse_binary_right(int, std::unique_ptr<Expression>);

    Token get_token() const
    {
        return m_current_token >= m_tokens.size() ? m_tokens[m_tokens.size() - 1] : m_tokens[m_current_token];
    }
};