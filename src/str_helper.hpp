#pragma once

#include <cstddef>

#include "nava.hpp"

Vec<String> chop_delimiter(String const&, char);

#ifndef STR_FORMAT
#define STR_FORMAT

template <typename... Args>
void string_format(String* out, const std::string & format, Args ... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    if (size_s <= 0)
    {
        throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    out->append(String(buf.get(), buf.get() + size - 1)); // We don't want the '\0' inside
}

#endif