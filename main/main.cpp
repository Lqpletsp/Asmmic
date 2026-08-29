#include "../IRinterpreter/VM.h"
#include "../lexer/lexer.h"
#include "types.h"
#include <iostream>

void DebugFunction();

void PrintDetails() {
  std::cout << "________________________\n" << std::endl;
  std::cout << "Memory declared: " << TotalMemSize << " spaces" << std::endl;
  std::cout << "Memory remaining: " << g_TotalMemPool.size() << " spaces"
            << std::endl;
  std::cout << "Total variable count: " << VarCount << std::endl;
  std::cout << "________________________\n" << std::endl;
}

int main() {
  HandleLexer();
  DebugFunction();
  return 0;
  ErrorInstance = "IT";
  InterpretByteCode();
  PrintDetails();
  return 0;
}

void DebugFunction() {
  for (const ByteCodeDT BC : ByteCode) {
    std::cout << static_cast<int>(BC.TypeRepr) << "->" << BC.LiteralToken
              << std::endl;
  }
}
