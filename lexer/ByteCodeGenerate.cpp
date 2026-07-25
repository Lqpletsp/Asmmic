#include "../errorhandling/ErrorHandler.h"
#include "../main/ImportantInternalFunctions.h"
#include "../main/types.h"
#include "Declarators.h"
#include <deque>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>

namespace {
bool CheckIfCommand(const TokenTypes &EnumTokenVal) {
  switch (EnumTokenVal) {
  case TokenTypes::out:
  case TokenTypes::inp:
  case TokenTypes::set:
  case TokenTypes::dec:
  case TokenTypes::mlc:
    return true;
  default:
    return false;
  }
}

bool CheckIfMidLineCommand(const TokenTypes &EnumTokenVal) {
  switch (EnumTokenVal) {
  case TokenTypes::clc:
    return true;
  default:
    return false;
  }
}
TokenTypes ReturnMidLineCommandVal(const std::string &Token) {
  if (Token == "clc")
    return TokenTypes::clc;
  else if (Token == "evl")
    return TokenTypes::evl;
  else
    return TokenTypes::Unknown;
}

TokenTypes ReturnCommandVal(const std::string &Token) {
  if (Token == "out")
    return TokenTypes::out;
  else if (Token == "inp")
    return TokenTypes::inp;
  else if (Token == "dec")
    return TokenTypes::dec;
  else if (Token == "set")
    return TokenTypes::set;
  else if (Token == "mlc")
    return TokenTypes::mlc;
  else
    return TokenTypes::Unknown;
}
TokenTypes DetermineType(const std::string &Token) {
  if (Token.size() == 0)
    return TokenTypes::Empty;
  TokenTypes CommandType = ReturnCommandVal(Token);

  if (CommandType != TokenTypes::Unknown) {
    return CommandType;
  }
  try {
    std::stoi(Token);
    std::stringstream ss(Token);
    double num;
    if ((ss >> num) && ss.eof()) {
      if (Token.find('.') != std::string::npos)
        return TokenTypes::DoubleVal; // float
      return TokenTypes::IntVal;      // int
    }
  } catch (...) {
  }
  if (Token.front() == '.') {
    std::string RawMLC = SliceStuff(1, Token.size() - 1, Token);
    CommandType = ReturnMidLineCommandVal(RawMLC);
    return CommandType;
  } else if (Token.front() == '~') {
    return (Token.size() > 1) ? TokenTypes::Flag : TokenTypes::Or;
  } else if (Token == "+")
    return TokenTypes::Add;
  else if (Token == "@")
    return TokenTypes::MemoryAddressIndicator;
  else if (Token == "(" || Token == ")")
    return TokenTypes::Parenthesis;
  else if (Token == "-")
    return TokenTypes::Min;
  else if (Token == "*")
    return TokenTypes::Mlt;
  else if (Token == "/")
    return TokenTypes::Div;
  else if (Token == ":")
    return TokenTypes::Colon;
  else if (Token == "]")
    return TokenTypes::Stopper;
  else if (Token == "<=")
    return TokenTypes::LessEqual;
  else if (Token == ">=")
    return TokenTypes::GreaterEqual;
  else if (Token == "!=")
    return TokenTypes::NotEqual;
  else if (Token == "==" || Token == "=")
    return TokenTypes::Equal;
  else if (Token == ">")
    return TokenTypes::GreaterThan;
  else if (Token == "<")
    return TokenTypes::LessThan;
  else if (Token == "!")
    return TokenTypes::Not;
  else if (Token == "&")
    return TokenTypes::And;
  else if ((Token.front() == '"' && Token.back() == '"') ||
           (Token.front() == '\'' && Token.back() == '\'')) {
    if (Token.size() == 3)
      return TokenTypes::CharVal; // char
    return TokenTypes::StringVal; // string
  } else if (Token.size() == 1 && (Token == "T" || Token == "F"))
    return TokenTypes::BoolVal; // boolean
  else if (ValidateName(Token))
    return TokenTypes::Identifier; // identifier
  return TokenTypes::Unknown;      // garbage token
}
// LT -> Literal Token
// LN -> Line Number
// CN -> Column Number
// TR -> Type representation
ByteCodeDT CreateByteCodeToken(const std::string &LT, const int &LN,
                               const int &CN, const TokenTypes &TR) {
  ByteCodeDT ByteCodeToken;
  ByteCodeToken.LiteralToken = LT;
  ByteCodeToken.LineNum = LN;
  ByteCodeToken.ColNum = CN;
  ByteCodeToken.TypeRepr = TR;
  return ByteCodeToken;
}
std::string GetStrVariableID(const std::string &VariableName) {
  int VariableID = GetAssignedVariableID(VariableName);
  if (VariableID < 0 || !CheckIfValidGlobalVariable(VariableID))
    return "!"; // means the variable does not exist
  std::stringstream ss;
  ss << std::fixed << MapVariableNameAndID[VariableName];
  return ss.str();
}
template <typename T> int HandleVariables(const T &Line, TokenDT &Token) {
  int LinePointer = 0, LiN = Token.LineNum, CoN = Token.ColNum;
  int VariableID = GetAssignedVariableID(Token.LiteralToken);
  if (VariableID < 0 || !CheckIfValidGlobalVariable(VariableID))
    ShowError(Token, ErrorTypes::IdentifierNotFound);
  VariableDT &VariableMetaData = *GetVariableMetaData(VariableID);
  std::string VarName = GetStrVariableID(Token.LiteralToken);
  if (VarName == "!")
    ShowError(Token, ErrorTypes::IdentifierNotFound);
  if (VariableMetaData.Array) {
    // handling arrays
    // first create the bytecode with variable ID
    ByteCode.push_back(
        CreateByteCodeToken(VarName, LiN, CoN, TokenTypes::ArrayHint));
    LinePointer++;
    bool ArrayStEnd = false, AddrOprtGiven = false;
    while (LinePointer < Line.size() && !ArrayStEnd) {
      Token = Line.at(LinePointer);
      if (Token.LiteralToken == "@") {
        AddrOprtGiven = true;
        ByteCode.push_back(
            CreateByteCodeToken("@", Token.LineNum, Token.ColNum,
                                TokenTypes::MemoryAddressIndicator));
        LinePointer++;
        continue;
      }
      if (!AddrOprtGiven) {
        ArrayStEnd = true;
        continue;
      }
      TokenTypes TypeOfToken;
      TypeOfToken = DetermineType(Token.LiteralToken);
      switch (TypeOfToken) {
      case TokenTypes::IntVal: {
        ByteCode.push_back(CreateByteCodeToken(Token.LiteralToken,
                                               Token.LineNum, Token.ColNum,
                                               TokenTypes::MemoryAddress));
        ArrayStEnd = true;
        break;
      }
      case TokenTypes::Identifier: {
        std::string VarName = GetStrVariableID(Token.LiteralToken);
        int VariableID = MapVariableNameAndID[Token.LiteralToken];
        if (VariableID < 0 || !CheckIfValidGlobalVariable(VariableID))
          ShowError(Token, ErrorTypes::IdentifierNotFound);
        VariableDT &VariableMetaData = *GetVariableMetaData(VariableID);
        if (VariableMetaData.DataType != TokenTypes::IntVal)
          // can cause an issue if tried to convert string to int
          // for digits
          ShowError(Token, ErrorTypes::NoIntArrayAddress);
        else if (VariableMetaData.Array) {
          ByteCode.push_back(CreateByteCodeToken(
              VarName, Token.LineNum, Token.ColNum, TokenTypes::ArrayHint));
        } else {
          ByteCode.push_back(CreateByteCodeToken(
              VarName, Token.LineNum, Token.ColNum, TokenTypes::VariableID));
          ArrayStEnd = true;
        }
        break;
      }
      default:
        ShowError(Token, ErrorTypes::NoIntArrayAddress);
        break;
      }
      LinePointer++;
      AddrOprtGiven = false;
      if (ArrayStEnd) {
        break;
      }
    }
    ByteCode.push_back(CreateByteCodeToken("", -1, -1, TokenTypes::ArrEnd));
  } else {
    // handling normal variables
    ByteCode.push_back(
        CreateByteCodeToken(VarName, LiN, CoN, TokenTypes::VariableID));
    LinePointer++;
  }
  return LinePointer;
}
int GetPrecedence(TokenTypes type) {
  switch (type) {
  case TokenTypes::Not: // '!' (Unary)
    return 6;
  case TokenTypes::Mlt: // '*'
  case TokenTypes::Div: // '/'
    return 5;
  case TokenTypes::Add: // '+'
  case TokenTypes::Min: // '-'
    return 4;
  case TokenTypes::LessThan:     // '<'
  case TokenTypes::GreaterThan:  // '>'
  case TokenTypes::LessEqual:    // '<='
  case TokenTypes::GreaterEqual: // '>='
  case TokenTypes::Equal:        // '=='
  case TokenTypes::NotEqual:     // '!='
    return 3;
  case TokenTypes::And: // '&'
    return 2;
  case TokenTypes::Or: // '~' (Logical OR)
    return 1;
  default:
    return 0;
  }
}
bool IsLeftAssociative(TokenTypes type) {
  // Standard arithmetic operators (+, -, *, /) are left-associative
  switch (type) {
  case TokenTypes::Add:
  case TokenTypes::Min:
  case TokenTypes::Mlt:
  case TokenTypes::And:
  case TokenTypes::Or:
  case TokenTypes::Not:
  case TokenTypes::NotEqual:
  case TokenTypes::Equal:
  case TokenTypes::LessEqual:
  case TokenTypes::GreaterEqual:
  case TokenTypes::Div:
    return true;
  default:
    return false;
  }
}
bool IsOperator(TokenTypes type) {
  return type == TokenTypes::Add || type == TokenTypes::Min ||
         type == TokenTypes::Mlt || type == TokenTypes::Div ||
         type == TokenTypes::GreaterEqual || type == TokenTypes::LessEqual ||
         type == TokenTypes::GreaterThan || type == TokenTypes::Not ||
         type == TokenTypes::And || type == TokenTypes::Or ||
         type == TokenTypes::LessThan || type == TokenTypes::NotEqual ||
         type == TokenTypes::Equal;
}
bool IsOperand(TokenTypes type) {
  switch (type) {
  case TokenTypes::IntVal:
  case TokenTypes::DoubleVal:
  case TokenTypes::ArrayHint:
  case TokenTypes::Identifier:
  case TokenTypes::BoolVal:
  case TokenTypes::MemoryAddressIndicator:
    return true;
  default:
    return false;
  }
}
int HandleShuntingYard(const TokenizedLineDT &Line, const int &LPT,
                       const std::string &cmd) {
  // shunting yard algorithm
  // if the cmd is clc, add MathExpr but if cmd is evl add BoolExpr
  // if evl "+" is "OR", "*" is "AND", "!" is "NOT"
  std::deque<TokenDT> outputQueue;
  std::stack<TokenDT> operatorStack;

  int InterruptedPtr = 0;
  size_t LinePointer = 0;
  while (LinePointer < Line.size()) {
    TokenDT token = Line.at(LinePointer);
    std::string LS = token.LiteralToken;
    if (LS == "<" || LS == ">" || LS == "!" || LS == "=") {
      // to support multi len tokens (>=, <=, !=, ==)
      ++LinePointer;
      if (LinePointer >= Line.size())
        break;
      token = Line.at(LinePointer);
      if (token.LiteralToken != "=")
        --LinePointer; // go back, different token
      else {
        LS += token.LiteralToken;
        token.LiteralToken = LS;
      }
    }
    TokenTypes type = DetermineType(token.LiteralToken);
    std::cout << static_cast<int>(type) << std::endl;
    if (token.LiteralToken == "]") {
      InterruptedPtr =
          LinePointer; // stopper stops the execution of the command
      break;
    } else if (IsOperand(type)) {
      outputQueue.push_back(token);
    } else if (IsOperator(type)) {
      while (!operatorStack.empty()) {
        TokenDT topToken = operatorStack.top();
        TokenTypes topType = DetermineType(topToken.LiteralToken);

        if (IsOperator(topType)) {
          int precCurr = GetPrecedence(type);
          int precTop = GetPrecedence(topType);

          if ((IsLeftAssociative(type) && precCurr <= precTop) ||
              (!IsLeftAssociative(type) && precCurr < precTop)) {
            outputQueue.push_back(topToken);
            operatorStack.pop();
            continue;
          }
        }
        break;
      }
      operatorStack.push(token);
    } else if (token.LiteralToken == "(") {
      operatorStack.push(token);
    } else if (token.LiteralToken == ")") {
      bool matched = false;
      std::string openMatch = "(";

      while (!operatorStack.empty()) {
        TokenDT topToken = operatorStack.top();
        if (topToken.LiteralToken == openMatch) {
          matched = true;
          operatorStack.pop(); // Pop matching parenthesis
          break;
        }
        outputQueue.push_back(topToken);
        operatorStack.pop();
      }

      if (!matched) {
        // Syntax Error: Mismatched parentheses
        ShowError(token, ErrorTypes::GarbageToken);
      }
    }
    InterruptedPtr = LinePointer;
    LinePointer++;
  }

  while (!operatorStack.empty()) {
    TokenDT topToken = operatorStack.top();
    if (topToken.LiteralToken == "(" || topToken.LiteralToken == ")") {
      // Syntax Error: Unmatched parenthesis remaining
      ShowError(topToken, ErrorTypes::GarbageToken);
    }
    outputQueue.push_back(topToken);
    operatorStack.pop();
  }
  // load the infix form for the bytecode
  TokenTypes StartIndication =
      (cmd == "clc") ? TokenTypes::MathExpr : TokenTypes::BoolExpr;
  TokenTypes EndIndication =
      (cmd == "clc") ? TokenTypes::MathExprEnd : TokenTypes::BoolExprEnd;
  ByteCode.push_back(CreateByteCodeToken("", -1, -1, StartIndication));
  while (!outputQueue.empty()) {
    TokenDT Token = outputQueue.front();
    TokenTypes TypeOfToken = DetermineType(Token.LiteralToken);
    int LiN = Token.LineNum, CoN = Token.ColNum;
    switch (TypeOfToken) {
    case TokenTypes::IntVal:
    case TokenTypes::CharVal:
    case TokenTypes::StringVal:
    case TokenTypes::DoubleVal:
    case TokenTypes::Add:
    case TokenTypes::Min:
    case TokenTypes::Mlt:
    case TokenTypes::Div:
    case TokenTypes::And:
    case TokenTypes::Or:
    case TokenTypes::Not:
    case TokenTypes::LessEqual:
    case TokenTypes::LessThan:
    case TokenTypes::GreaterEqual:
    case TokenTypes::GreaterThan:
    case TokenTypes::Equal:
      ByteCode.push_back(
          CreateByteCodeToken(Token.LiteralToken, LiN, CoN, TypeOfToken));
      outputQueue.pop_front();
      break;
    case TokenTypes::Identifier: {
      int NumberOfIteration = HandleVariables(outputQueue, Token);
      for (int _ = 0; _ < NumberOfIteration; _++) {
        outputQueue.pop_front();
      }
      break;
    }
    default:
      break;
    }
  }
  ByteCode.push_back(CreateByteCodeToken("", -1, -1, EndIndication));

  return InterruptedPtr + 2;
}
} // namespace

void GenerateByteCode(const TokenizedCodeDT &TokenizedCode) {
  ByteCodeDT ByteCodeReprOfTokens;
  for (const auto &Line : TokenizedCode) {
    if (Line.size() == 0)
      continue;
    else if (!CheckIfCommand(DetermineType(Line.at(0).LiteralToken))) {
      ShowError(Line.at(0), ErrorTypes::NoCommandInFrontofLine);
    } else if (Line.at(0).LiteralToken == "dec") {
      TokenizedLineDT SlicedLine = SliceStuff(1, Line.size() - 1, Line);
      decCommand(SlicedLine);
      continue; // already handled. No need to create a bytecode for it
    }
    int LinePointer = 0;
    while (LinePointer < Line.size()) {
      TokenDT Token = Line.at(LinePointer);
      TokenTypes TypeOfToken = DetermineType(Token.LiteralToken);
      int LiN = Token.LineNum, CoN = Token.ColNum;
      if (TypeOfToken == TokenTypes::Unknown)
        ShowError(Token, ErrorTypes::GarbageToken);
      else if (TypeOfToken == TokenTypes::Identifier) {
        // check whether the variable actually exists.
        // if it does not, check if it is a function, user made command
        // (not supported right now) push error if everything fails
        TokenizedLineDT SlicedLine =
            SliceStuff(LinePointer, Line.size() - 1, Line);
        int iteration = HandleVariables(SlicedLine, Token);
        LinePointer += iteration;
        continue;
      }
      std::string LiteralString;
      if (TypeOfToken == TokenTypes::Flag)
        LiteralString =
            SliceStuff(1, Token.LiteralToken.size() - 1, Token.LiteralToken);
      else if (TypeOfToken == TokenTypes::clc ||
               TypeOfToken == TokenTypes::evl) {
        TokenizedLineDT SlicedLine =
            SliceStuff(LinePointer + 1, Line.size() - 1, Line);
        std::string LS =
            SliceStuff(1, Token.LiteralToken.size() - 1, Token.LiteralToken);
        int iteration = HandleShuntingYard(SlicedLine, LinePointer, LS);
        LinePointer += iteration;
        continue;

      } else {
        if (TypeOfToken == TokenTypes::StringVal ||
            TypeOfToken == TokenTypes::CharVal) {
          LiteralString =
              SliceStuff(1, Token.LiteralToken.size() - 2, Token.LiteralToken);
        } else
          LiteralString = Token.LiteralToken;
      }
      ByteCode.push_back(
          CreateByteCodeToken(LiteralString, LiN, CoN, TypeOfToken));
      LinePointer++;
    }
    ByteCode.push_back(CreateByteCodeToken(";", -1, -1, TokenTypes::NewLine));
    // new line flag set by bytecode-- has to do nothing with the user
    // so -1 tells the error handler that it was by the generator itself
  }
  ByteCode.push_back(CreateByteCodeToken("", -1, -1, TokenTypes::End));
}
