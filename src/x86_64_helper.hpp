#pragma once

#include <cstddef>

#include "nava.hpp"
#include "global.hpp"
#include "str_helper.hpp"

void mov_mX_reg(String*, String const&, size_t, const char*);
void mov_mX_mX(String*, String const&, size_t, size_t);
void mov_reg_mX(String*, String const&, size_t, const char*);
void mov_reg_immX(String*, String const&, const char*, Value);
void mov_mX_immX(String*, String const&, size_t, Value);
void mov_reg_data(String*, const char*, String const&);