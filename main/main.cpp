#include "../IRinterpreter/VM.h"
#include "../lexer/lexer.h"
#include "types.h"
#include <fstream>
#include <iostream>

void DebugFunction();
int ExtractTextFromFile();

void PrintDetails() {

  std::cout << "Memory declared: " << TotalMemSize << " spaces" << std::endl;
  std::cout << "Memory remaining: " << g_TotalMemPool.size() << " spaces"
            << std::endl;
  std::cout << "Total variable count: " << VarCount << std::endl;
}

int main() {
  if (ExtractTextFromFile() > 0)
    return 1;
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

int ExtractTextFromFile() {
  std::ifstream InFile(dir_path);
  if (!InFile.is_open()) {
    std::cout << "ERR[FE] : File extraction failed for path '" << dir_path
              << "'" << std::endl;
    return 1;
  }
  std::string line;
  while (std::getline(InFile, line)) {
    MAINCODE = MAINCODE + line + '\n';
  }
  return 0;
}
