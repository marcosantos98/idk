#pragma once

#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>

#include "json.hpp"

#define VEC_HAS(vec, val) (std::find(vec.begin(), vec.end(), val) != vec.end())

template <class T>
using OwnPtr = std::unique_ptr<T>;

template <class T>
using Vec = std::vector<T>;

template <class K, class V>
using Map = std::map<K, V>;

template <class T>
using OwnPtrVec = Vec<OwnPtr<T>>;

using String = std::string;

using Json = nlohmann::json;

enum class ValueType : int
{
    STRING,
    NUMBER,
    BOOL,
    VAR_REF,
    ARRAY_VAR_REF
};

namespace NAVA
{
    static Map<String, int> primitive_byte_sizes = {
        {"int", 4},
        {"boolean", 4}};

    static Vec<String> base_types = {
        "int",
        "class",
    };

    static Vec<String> modifiers = {
        "public",
        "private",
        "final",
        "static",
        "abstract",
    };

    static Map<String, int> op_precedence = {
        {"!=", 10},
        {"<=", 10},
        {"<", 10},
        {">=", 10},
        {">", 10},
        {"+", 20},
        {"-", 20},
        {"%", 30},
        {"*", 30},
        {"/", 30},
    };

    enum class NumberType
    {
        INT,
        DOUBLE,
    };

    struct Modifier
    {
        bool is_public = false;
        bool is_static = false;
        bool is_final = false;
        bool is_abstract = false;
    };

    struct Definition
    {
        std::string arg_name;
        std::string class_name;
        Modifier mod;
        size_t start = 0;
        size_t end = 0;
    };

    enum class VarContext
    {
        STACK,
        DATA,
    };

    struct StackVarInfo
    {
        size_t offset;
        String class_name;
    };

    struct GlobalContext
    {
        NAVA::VarContext var_ctx;
        String text_section;
        String extern_section;
        String data_section;
        String class_name;
        size_t tmp_str_cnt = 0;
        size_t label_cnt = 0;
        NAVA::Definition current_def;
        size_t current_stack_offset = 0;
        Vec<String> data_vars = {};
        Vec<String> externs = {};
        Map<String, NAVA::StackVarInfo> stack_vars = {};
    };

    static String modifier_to_str(Modifier modifier)
    {
        String str = "";
        str.append("PUBLIC: ").append(std::to_string(modifier.is_public));
        str.append(", FINAL: ").append(std::to_string(modifier.is_final));
        str.append(", ABSTRACT: ").append(std::to_string(modifier.is_abstract));
        str.append(", STATIC: ").append(std::to_string(modifier.is_static));
        return str;
    }

    // fixme 22/10/06: This is not ideal.
    inline static Json def_to_json(Definition def)
    {
        Json json;

        json["arg_name"] = def.arg_name;
        json["class_name"] = def.class_name;
        json["modifier"] = modifier_to_str(def.mod);

        return json;
    }
}
