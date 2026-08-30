#include "../IRinterpreter/VM.h"
#include "../lexer/lexer.h"
#include "types.h"
#include <iostream>

void DebugFunction();

void PrintDetails() {

  std::cout << "Memory declared: " << TotalMemSize << " spaces" << std::endl;
  std::cout << "Memory remaining: " << g_TotalMemPool.size() << " spaces"
            << std::endl;
  std::cout << "Total variable count: " << VarCount << std::endl;
}

int main() {
  HandleLexer();
  ErrorInstance = "IT";
  std::cout << "________________________\n" << std::endl;
  InterpretByteCode();
  std::cout << "\n________________________\n" << std::endl;
  PrintDetails();
  return 0;
}

void DebugFunction() {
  int idx = 0;
  for (const ByteCodeDT BC : ByteCode) {
    std::cout << idx << ". " << static_cast<int>(BC.TypeRepr) << "->"
              << BC.LiteralToken << std::endl;
    ++idx;
  }
}
