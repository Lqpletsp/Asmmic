#pragma once
#include "../main/types.h"
#include <string>

TokenizedCodeDT TokenizeCode(const std::string &MAINCODE);
void GenerateByteCode(const TokenizedCodeDT &TokenizedCode);

inline void HandleLexer() { GenerateByteCode(TokenizeCode(MAINCODE)); }
