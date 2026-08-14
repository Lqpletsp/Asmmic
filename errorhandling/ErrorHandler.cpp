#include "ErrorHandler.h"
#include "../main/types.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {
std::vector<std::string> SplitLines() {
  bool InString = false, InComment = false;
  std::string Line;
  std::vector<std::string> TokenLineCodes;
  for (const char ch : MAINCODE) {
    switch (ch) {
    case '"':
      if (!InString)
        InString = true;
      else
        InString = false;
      Line += ch;
      break;
    case '\n':
      if (InString) {
        Line += ch;
        break;
      }
      TokenLineCodes.push_back(Line);
      Line = "";
      break;
    default:
      Line += ch;
      break;
    }
  }
  if (!Line.empty()) {
    TokenLineCodes.push_back(Line);
  }
  return TokenLineCodes;
}

std::string GetLine(const std::vector<std::string> &TokenizedCode,
                    const int &LineNumber) {
  int targetIndex = LineNumber - 1;

  if (targetIndex < 0 || targetIndex >= TokenizedCode.size()) {
    return "[Source line text unavailable]";
  }
  return TokenizedCode.at(targetIndex);
}
} // namespace

void ShowError(const TokenDT &Token, const ErrorTypes &Type) {
  int ErrLineNum = Token.LineNum;
  int ErrCol = Token.ColNum;

  std::string ErrLine = GetLine(SplitLines(), ErrLineNum);
  std::cout << ErrLine << '\n';
  if (!(ErrCol == -1 && ErrLineNum == -1)) {
    // Print spaces up to the start of the token
    if (ErrCol > 1) {
      std::cout << std::string(ErrCol - 1, ' ');
    }
    // Underline the exact length of the bad token
    std::cout << std::string(Token.LiteralToken.size(), '^') << "\n\n";
  }

  if (Token.LiteralToken == "" || Type == ErrorTypes::MemoryFull)
    std::cout << "Err[ln:" << ErrLineNum << "]: " << std::endl;
  else
    std::cout << "Err[ln:" << ErrLineNum
              << "]: " << '"' + Token.LiteralToken + '"' << " was given.\n";

  std::cout << CorrespondingErrorStrings[Type] << std::endl;
  std::exit(1);
}
void ShowError(const ByteCodeDT &Token, const ErrorTypes &Type) {
  int ErrLineNum = Token.LineNum;
  int ErrCol = Token.ColNum;

  std::string ErrLine = GetLine(SplitLines(), ErrLineNum);
  std::cout << ErrLine << '\n';
  std::string BadToken;
  if (Token.TypeRepr == TokenTypes::VariableID) {
    bool found = false;
    for (const auto &pair : *c_MapVariableNameAndID) {
      if (pair.second == std::stoi(Token.LiteralToken)) {
        BadToken = pair.first;
        found = true;
        break;
      }
    }
    if (!found)
      BadToken = Token.LiteralToken;
  }
  if (!(ErrCol == -1 && ErrLineNum == -1)) {
    // Print spaces up to the start of the token
    if (ErrCol > 1) {
      std::cout << std::string(ErrCol - 1, ' ');
    }
    // Underline the exact length of the bad token
    // std::string BadToken = Token.LiteralToken;
    // if (Token.TypeRepr == TokenTypes::VariableID)
    // BadToken = ErrLine.at(ErrCol);
    std::cout << std::string(BadToken.size(), '^') << "\n\n";
  }
  if (Token.LiteralToken == "" || Type == ErrorTypes::MemoryFull)
    std::cout << "Err[ln:" << ErrLineNum << "]: ";
  else
    std::cout << "Err[ln:" << ErrLineNum << "]: " << '"' + BadToken + '"'
              << " was given.\n";
  std::cout << CorrespondingErrorStrings[Type] << std::endl;
  std::exit(1);
}
