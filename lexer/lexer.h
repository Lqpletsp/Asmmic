#pragma once
#include "../main/types.h"

TokenizedCodeDT TokenizeCode(const std::string &MAINCODE);
void GenerateByteCode(const TokenizedCodeDT &TokenizedCode);

inline void HandleLexer() { GenerateByteCode(TokenizeCode(MAINCODE)); }
