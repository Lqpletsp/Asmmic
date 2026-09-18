#include "../IRinterpreter/VM.h"
#include "../lexer/lexer.h"
#include <chrono> // Added for timing
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

  // --- Start Timer ---
  auto start = std::chrono::high_resolution_clock::now();

  InterpretByteCode();

  // --- End Timer ---
  auto end = std::chrono::high_resolution_clock::now();

  std::cout << "\n________________________\n" << std::endl;

  // Calculate duration in milliseconds
  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << "InterpretByteCode execution time: " << duration.count()
            << " ms\n";

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
