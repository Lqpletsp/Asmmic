
#include "../main/types.h"
#include <string>
TokenDT PushToken(const std::string &CurrentToken, const int RowCount,
                  const int ColCount) {
  TokenDT Token;
  Token.ColNum = ColCount;
  Token.LineNum = RowCount;
  Token.LiteralToken = CurrentToken;
  return Token;
}

TokenizedCodeDT TokenizeCode(const std::string &MAINCODE) {
  std::string CurrentToken;
  TokenizedLineDT TokenizedLine;
  TokenizedCodeDT TokenizedCode;
  int RowCount = 1, ColCount = 0, TokenStartCol = 1, CodePTR = 0;
  bool inString = false, inComment = false;
  while (CodePTR < MAINCODE.size()) {

    char ch = MAINCODE.at(CodePTR);
    ColCount += 1;

    if (ch == '\n' || ch == '\r') {
      if (!inString && !inComment) {
        if (!CurrentToken.empty()) {
          TokenizedLine.push_back(
              PushToken(CurrentToken, RowCount, TokenStartCol));
          CurrentToken.clear();
        }
      }
      RowCount += 1;
      ColCount = 0;
      ++CodePTR;
      continue;
    }
    if (inComment) {
      if (ch == '|')
        inComment = false;
      ++CodePTR;
      continue;
    }
    if (inString) {
      CurrentToken += ch;
      if (ch == '"') {
        inString = false;
        TokenizedLine.push_back(
            PushToken(CurrentToken, RowCount, TokenStartCol));
        CurrentToken.clear();
      }
      ++CodePTR;
      continue;
    }

    // Normal coding state
    if (!inString && !inComment) {
      if (ch == '|') {
        inComment = true;
      } else if (ch == '"') {
        inString = true;
        TokenStartCol = ColCount; // String starts here
        CurrentToken += ch;
      } else if (ch == ';') {
        // Commit last token before semicolon using ITS start col
        if (!CurrentToken.empty()) {
          TokenizedLine.push_back(
              PushToken(CurrentToken, RowCount, TokenStartCol));
          CurrentToken.clear();
        }

        // Commit the finished line to the code structure
        if (!TokenizedLine.empty()) {
          TokenizedCode.push_back(TokenizedLine);
        }
        TokenizedLine.clear();
      } else if (ch == ' ' || ch == '\t' || ch == '*' || ch == '@' ||
                 ch == ':' || ch == '+' || ch == '-' || ch == '/' ||
                 ch == ']' || ch == ',' || ch == '&' || ch == '(' ||
                 ch == ')' || ch == '>' || ch == '<' || ch == '=' ||
                 ch == '.') {

        // Flush existing word token
        bool MLCcheck = false, FloatingDigitCheck = false;
        if (ch == '.') {
          try {
            std::stod(CurrentToken);
            FloatingDigitCheck = true;
          } catch (...) {
          }
        }

        if (!CurrentToken.empty()) {
          MLCcheck = CurrentToken.at(0) == '.' && ch == ' ' &&
                     CurrentToken.size() == 1;
          if (!(MLCcheck || FloatingDigitCheck)) {
            TokenizedLine.push_back(
                PushToken(CurrentToken, RowCount, TokenStartCol));
            CurrentToken.clear();
          }
        }
        // If it's an operator, capture it as its own token
        if (ch != ' ' && ch != '\t' && ch != ',' &&
            !(MLCcheck || FloatingDigitCheck)) {
          MLCcheck = (!CurrentToken.empty())
                         ? (CurrentToken.at(0) == '.' && ch == ' ' &&
                            CurrentToken.size() == 1)
                         : false;
          if (MLCcheck || FloatingDigitCheck) {
            TokenizedLine.push_back(
                PushToken(std::string(1, ch), RowCount, ColCount));
          } else {
            CurrentToken += ch;
          }
        } else if (ch != ' ' && ch != '\t' && ch != ',' &&
                   (MLCcheck || FloatingDigitCheck)) {
          CurrentToken += ch;
        }
      } else {
        if (CurrentToken.empty()) {
          TokenStartCol = ColCount;
        }
        CurrentToken += ch;
      }
    }
    CodePTR++;
  }
  if (!CurrentToken.empty()) {
    TokenizedLine.push_back(PushToken(CurrentToken, RowCount, TokenStartCol));
  }
  if (!TokenizedLine.empty()) {
    TokenizedCode.push_back(TokenizedLine);
  }
  return TokenizedCode;
}
