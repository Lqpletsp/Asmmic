#include "../main/types.h"
#include <cctype>

TokenDT PushToken(const std::string &CurrentToken, const int RowCount,
                  const int ColCount) {
  TokenDT Token;
  Token.ColNum = ColCount;
  Token.LineNum = RowCount;
  Token.LiteralToken = CurrentToken;
  return Token;
}

TokenizedCodeDT TokenizeCode(const std::string &MAINCODE) {
  std::string CurrentToken, PastToken;
  TokenizedLineDT TokenizedLine;
  TokenizedCodeDT TokenizedCode;
  int RowCount = 1, ColCount = 0, TokenStartCol = 1, CodePTR = 0;
  bool inString = false, inComment = false;

  while (CodePTR < MAINCODE.size()) {
    char ch = MAINCODE.at(CodePTR);
    ColCount += 1;

    // Handle Newlines
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

    // Handle Comments (| ... |)
    if (inComment) {
      if (ch == '|')
        inComment = false;
      ++CodePTR;
      continue;
    }

    // Handle String Literals ("...")
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

    // Normal State
    if (ch == '|') {
      inComment = true;
    } else if (ch == '"') {
      inString = true;
      TokenStartCol = ColCount;
      CurrentToken += ch;
    } else if (ch == ';') {
      if (!CurrentToken.empty()) {
        TokenizedLine.push_back(
            PushToken(CurrentToken, RowCount, TokenStartCol));
        PastToken = CurrentToken;
        CurrentToken.clear();
      }
      if (!TokenizedLine.empty()) {
        TokenizedCode.push_back(TokenizedLine);
      }
      TokenizedLine.clear();
    } else if (ch == '.') {
      // Lookahead check for numbers (e.g. 0.2 or .2)
      bool isNextDigit = (CodePTR + 1 < MAINCODE.size()) &&
                         std::isdigit(MAINCODE.at(CodePTR + 1));

      if (!CurrentToken.empty()) {
        // If CurrentToken is purely numeric (e.g. "0" before "."), treat dot as
        // decimal point
        bool isCurrentTokenNumeric = true;
        for (char c : CurrentToken) {
          if (!std::isdigit(c)) {
            isCurrentTokenNumeric = false;
            break;
          }
        }

        if (isCurrentTokenNumeric) {
          CurrentToken += ch; // "0" + "." -> "0."
        } else {
          // Flush existing non-numeric token and start new dot token
          TokenizedLine.push_back(
              PushToken(CurrentToken, RowCount, TokenStartCol));
          PastToken = CurrentToken;
          CurrentToken = ch;
          TokenStartCol = ColCount;
        }
      } else {
        // CurrentToken is empty: could be ".clc" or ".2"
        TokenStartCol = ColCount;
        CurrentToken += ch;
      }
    } else if (ch == ' ' || ch == '\t' || ch == '*' || ch == '@' || ch == ':' ||
               ch == '+' || ch == '-' || ch == '/' || ch == ']' || ch == ',' ||
               ch == '&' || ch == '(' || ch == ')' || ch == '>' || ch == '<' ||
               ch == '=') {

      // 1. Flush pending token (e.g., numbers, keywords, or .clc)
      if (!CurrentToken.empty()) {
        TokenizedLine.push_back(
            PushToken(CurrentToken, RowCount, TokenStartCol));
        PastToken = CurrentToken;
        CurrentToken.clear();
      }

      // 2. Emit operator if not whitespace
      if (ch != ' ' && ch != '\t' && ch != ',') {
        TokenizedLine.push_back(
            PushToken(std::string(1, ch), RowCount, ColCount));
        PastToken = std::string(1, ch);
      }
    } else {
      // Alphanumeric / Identifier character
      if (CurrentToken.empty()) {
        TokenStartCol = ColCount;
      }
      CurrentToken += ch;
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
