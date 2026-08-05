#include "../errorhandling/ErrorHandler.h"
#include "../main/ImportantInternalFunctions.h"
#include "../main/types.h"
#include "Declarators.h"
#include <deque>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

bool IsMathOperator(TokenTypes &type) {
  return type == TokenTypes::Add || type == TokenTypes::Min ||
         type == TokenTypes::Mlt || type == TokenTypes::Div;
};
std::unordered_map<std::string, TokenTypes> MapStringAndCommand = {
    {"clc", TokenTypes::clc},       {"inp", TokenTypes::evl},
    {"set", TokenTypes::set},       {"dec", TokenTypes::dec},
    {"mlc", TokenTypes::mlc},       {"and", TokenTypes::And},
    {"evl", TokenTypes::evl},       {"not", TokenTypes::Not},
    {"out", TokenTypes::out},       {"rpt", TokenTypes::rpt},
    {"cmp", TokenTypes::cmp},       {"end", TokenTypes::end},
    {"<=", TokenTypes::LessEqual},  {">=", TokenTypes::GreaterEqual},
    {"<", TokenTypes::LessThan},    {">", TokenTypes::GreaterThan},
    {"orr", TokenTypes::Or},        {"@", TokenTypes::MemoryAddressIndicator},
    {"!=", TokenTypes::NotEqual},   {"(", TokenTypes::Parenthesis},
    {")", TokenTypes::Parenthesis}, {"+", TokenTypes::Add},
    {"-", TokenTypes::Min},         {"*", TokenTypes::Mlt},
    {"/", TokenTypes::Div},         {":", TokenTypes::Colon},
    {"]", TokenTypes::Stopper},     {"T", TokenTypes::TrueVal},
    {"F", TokenTypes::FalseVal},    {"==", TokenTypes::Equal},
    {"=", TokenTypes::Equal}};

bool CheckIfCommand(const TokenTypes &EnumTokenVal) {
  switch (EnumTokenVal) {
  case TokenTypes::out:
  case TokenTypes::inp:
  case TokenTypes::set:
  case TokenTypes::dec:
  case TokenTypes::mlc:
  case TokenTypes::rpt:
  case TokenTypes::end:
  case TokenTypes::cmp:

    return true;
  default:
    return false;
  }
}

bool CheckIfMidLineCommand(const TokenTypes &EnumTokenVal) {
  switch (EnumTokenVal) {
  case TokenTypes::clc:
  case TokenTypes::evl:
  case TokenTypes::Not:
  case TokenTypes::And:
  case TokenTypes::Or:
    return true;
  default:
    return false;
  }
}

TokenTypes DetermineType(const std::string &Token) {
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

  std::string SingleToken = Token;
  if (Token.front() == '.') {
    SingleToken = SliceStuff(1, Token.size() - 1, Token);
    return (!CheckIfMidLineCommand(MapStringAndCommand[SingleToken]))
               ? TokenTypes::Unknown
               : MapStringAndCommand[SingleToken];
  } else {
    SingleToken = Token;
  }

  if (MapStringAndCommand.find(SingleToken) == MapStringAndCommand.end()) {
    if (Token.front() == '~')
      return TokenTypes::Flag;
    else if ((Token.front() == '"' && Token.back() == '"') ||
             (Token.front() == '\'' && Token.back() == '\'')) {
      if (Token.size() == 3)
        return TokenTypes::CharVal; // char
      return TokenTypes::StringVal; // string
    } else if (ValidateName(Token))
      return TokenTypes::Identifier; // identifier
    return TokenTypes::Unknown;      // garbage token
  }
  return MapStringAndCommand[SingleToken];
}

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
      TokenTypes TypeOfToken = DetermineType(Token.LiteralToken);
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
    ByteCode.push_back(
        CreateByteCodeToken(VarName, LiN, CoN, TokenTypes::VariableID));
    LinePointer++;
  }
  return LinePointer;
}

int HandleShuntingYard(const TokenizedLineDT &Line, const std::string &cmd) {
  std::deque<TokenDT> outputQueue;
  std::stack<TokenDT> operatorStack;

  auto GetPrecedence = [&](TokenTypes &type) {
    switch (type) {
    case TokenTypes::Not: // '.not' (Unary)
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
    case TokenTypes::And: // '.and'
      return 2;
    case TokenTypes::Or: // '.orr'
      return 1;
    default:
      return 0;
    }
  };

  auto IsLeftAssociative = [&](TokenTypes &type) {
    switch (type) {
    case TokenTypes::Not:
      return false; // Unary NOT is RIGHT-associative
    case TokenTypes::Add:
    case TokenTypes::Min:
    case TokenTypes::Mlt:
    case TokenTypes::And:
    case TokenTypes::Or:
    case TokenTypes::NotEqual:
    case TokenTypes::Equal:
    case TokenTypes::LessEqual:
    case TokenTypes::GreaterEqual:
    case TokenTypes::Div:
    case TokenTypes::LessThan:
    case TokenTypes::GreaterThan:
      return true;
    default:
      return false;
    }
  };

  auto IsOperator = [&](TokenTypes &type) {
    return type == TokenTypes::Add || type == TokenTypes::Min ||
           type == TokenTypes::Mlt || type == TokenTypes::Div ||
           type == TokenTypes::GreaterEqual || type == TokenTypes::LessEqual ||
           type == TokenTypes::GreaterThan || type == TokenTypes::Not ||
           type == TokenTypes::And || type == TokenTypes::Or ||
           type == TokenTypes::LessThan || type == TokenTypes::NotEqual ||
           type == TokenTypes::Equal;
  };

  auto IsOperand = [&](TokenTypes &type) {
    switch (type) {
    case TokenTypes::IntVal:
    case TokenTypes::DoubleVal:
    case TokenTypes::ArrayHint:
    case TokenTypes::Identifier:
    case TokenTypes::TrueVal:
    case TokenTypes::FalseVal:
    case TokenTypes::MemoryAddressIndicator:
      return true;
    default:
      return false;
    }
  };

  int InterruptedPtr = 0;
  size_t LinePointer = 0;

  while (LinePointer < Line.size()) {
    TokenDT token = Line.at(LinePointer);
    std::string LS = token.LiteralToken;

    if (LS == "<" || LS == ">" || LS == "!" || LS == "=") {
      ++LinePointer;
      if (LinePointer >= Line.size())
        break;
      token = Line.at(LinePointer);
      if (token.LiteralToken != "=") {
        --LinePointer;
        token = Line.at(LinePointer);
      } else {
        LS += token.LiteralToken;
        token.LiteralToken = LS;
      }
    }

    TokenTypes type = DetermineType(token.LiteralToken);

    if (type == TokenTypes::Stopper) {
      InterruptedPtr = LinePointer;
      break;
    } else if (type == TokenTypes::clc || type == TokenTypes::evl) {
      std::string cmd = (type == TokenTypes::clc) ? "clc" : "evl";
      TokenizedLineDT SlicedLine =
          SliceStuff(LinePointer, Line.size() - 1, Line);
      int Increment = HandleShuntingYard(SlicedLine, cmd);
      LinePointer += Increment;
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
          operatorStack.pop();
          break;
        }
        outputQueue.push_back(topToken);
        operatorStack.pop();
      }

      if (!matched) {
        ShowError(token, ErrorTypes::GarbageToken);
      }
    }
    InterruptedPtr = LinePointer;
    LinePointer++;
  }

  while (!operatorStack.empty()) {
    TokenDT topToken = operatorStack.top();
    if (topToken.LiteralToken == "(" || topToken.LiteralToken == ")") {
      ShowError(topToken, ErrorTypes::GarbageToken);
    }
    outputQueue.push_back(topToken);
    operatorStack.pop();
  }

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
    // --- ADDED TrueVal and FalseVal HERE ---
    case TokenTypes::TrueVal:
    case TokenTypes::FalseVal:
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
    case TokenTypes::NotEqual:
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
      // Prevent infinite loop if an unhandled token makes it to the queue
      outputQueue.pop_front();
      break;
    }
  }

  ByteCode.push_back(CreateByteCodeToken("", -1, -1, EndIndication));

  return InterruptedPtr + 2;
}

} // namespace

void GenerateByteCode(const TokenizedCodeDT &TokenizedCode) {
  ByteCodeDT ByteCodeReprOfTokens;
  std::stack<int> RPTStartLine;
  std::stack<int> CMPStartLine;
  std::vector<int> CMPBlockCodeFinish;

  for (size_t i = 0; i < TokenizedCode.size(); ++i) {
    TokenizedLineDT Line = TokenizedCode.at(i);
    if (Line.size() == 0)
      continue;
    else if (!CheckIfCommand(DetermineType(Line.at(0).LiteralToken))) {
      ShowError(Line.at(0), ErrorTypes::NoCommandInFrontofLine);
    } else if (Line.at(0).LiteralToken == "dec") {
      TokenizedLineDT SlicedLine = SliceStuff(1, Line.size() - 1, Line);
      decCommand(SlicedLine);
      continue;
    }
    int LinePointer = 0;
    while (LinePointer < Line.size()) {
      TokenDT Token = Line.at(LinePointer);
      TokenTypes TypeOfToken = DetermineType(Token.LiteralToken);
      int LiN = Token.LineNum, CoN = Token.ColNum;
      if (TypeOfToken == TokenTypes::Unknown)
        ShowError(Token, ErrorTypes::GarbageToken);
      else if (TypeOfToken == TokenTypes::Identifier) {
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
        int iteration = HandleShuntingYard(SlicedLine, LS);
        LinePointer += iteration;
        continue;

      } else if (TypeOfToken == TokenTypes::cmp) {

        if (CMPStartLine.empty()) {
          CMPStartLine.push(ByteCode.size());
          ByteCode.push_back(CreateByteCodeToken(
              "", -1, -1,
              TokenTypes::gotoln)); // will be replaced when "end .rpt" line
          LinePointer++;
          continue;
        } else {
          ByteCode.at(CMPStartLine.top()).LiteralToken =
              std::to_string(ByteCode.size());
          CMPStartLine.pop();
          // to skip past other cmp statements before end .cmp if the code block
          // is executed
          ByteCode.push_back(
              CreateByteCodeToken("", -1, -1, TokenTypes::gotoln));
          CMPBlockCodeFinish.push_back(ByteCode.size() - 1);
          CMPStartLine.push(ByteCode.size());
          ByteCode.push_back(
              CreateByteCodeToken("", -1, -1, TokenTypes::gotoln));
          if (Line.size() == 1) {
            ByteCode.push_back(
                CreateByteCodeToken("", -1, -1, TokenTypes::BoolExpr));
            ByteCode.push_back(
                CreateByteCodeToken("T", -1, -1, TokenTypes::TrueVal));
            ByteCode.push_back(
                CreateByteCodeToken("", -1, -1, TokenTypes::BoolExprEnd));
          }
          LinePointer++;
          continue;
        }
      } else if (TypeOfToken == TokenTypes::Add ||
                 TypeOfToken == TokenTypes::Min) {
        LinePointer++;
        if (LinePointer >= Line.size())
          break;
        TokenTypes type = DetermineType(Line.at(LinePointer).LiteralToken);
        if (type == TokenTypes::DoubleVal || type == TokenTypes::IntVal) {
          Token = Line.at(LinePointer);
          Token.LiteralToken =
              Line.at(LinePointer - 1).LiteralToken + Token.LiteralToken;
          LiN = Token.LineNum;
          CoN = Token.ColNum;
          LiteralString = Token.LiteralToken;
          TypeOfToken = type;
        }

      } else if (TypeOfToken == TokenTypes::rpt) {
        RPTStartLine.push(ByteCode.size());
        ByteCode.push_back(CreateByteCodeToken(
            "", -1, -1,
            TokenTypes::gotoln)); // will be replaced when "end .rpt" line
        LinePointer++;
        continue;
      } else if (TypeOfToken == TokenTypes::end) {
        LinePointer++;
        if (LinePointer >= Line.size())
          ShowError(Token, ErrorTypes::InvalidEndStatement);
        Token = Line.at(LinePointer);
        if (Token.LiteralToken == ".cmp") {
          if (CMPStartLine.size() > 1)
            ShowError(Token, ErrorTypes::PastCMPstatementsStartedbutNotEnded);
          ByteCode.at(CMPStartLine.top()).LiteralToken =
              std::to_string(ByteCode.size());
          while (!CMPBlockCodeFinish.empty()) {
            ByteCode.at(CMPBlockCodeFinish.back()).LiteralToken =
                std::to_string(ByteCode.size());
            CMPBlockCodeFinish.pop_back();
          }
          CMPBlockCodeFinish.clear();
          CMPStartLine = std::stack<int>();

          LinePointer++;
          continue;
        }
        if (Token.LiteralToken == ".all") {
          ByteCode.push_back(
              CreateByteCodeToken("", -1, -1, TokenTypes::ENDCODE));
          LinePointer++;
          continue;
        } else if (Token.LiteralToken == ".rpt") {
          // assuming the most recent rpt line ended
          if (RPTStartLine.empty())
            ShowError(Token, ErrorTypes::endrptStatementGivenButNotStarted);
          ByteCode.at(RPTStartLine.top()).LiteralToken =
              std::to_string(ByteCode.size() + 1);
          // overwrite the start rpt statement to go to line if condition not
          // met
          ByteCode.push_back(
              CreateByteCodeToken(std::to_string(RPTStartLine.top() - 1), -1,
                                  -1, TokenTypes::gotoln));
          // add a bytecode to tell the program to go to the start of rpt
          // statement for loop
          RPTStartLine.pop();
          LinePointer++;
          continue;
        }
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
  }
  ByteCode.push_back(CreateByteCodeToken("", -1, -1, TokenTypes::ENDCODE));
}
