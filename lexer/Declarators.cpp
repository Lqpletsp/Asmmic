#include "../errorhandling/ErrorHandler.h"
#include "../main/ImportantInternalFunctions.h"
#include "../main/types.h"
#include <iostream> // db

void DeclareMemory(const TokenizedLineDT &MemDecLine) {
  if (MemDecLine.size() != 1) {
    if (MemDecLine.empty()) {
      return;
    }
    ShowError(MemDecLine.at(0), ErrorTypes::GarbageArgInACommand);
  }
  TotalMemSize = std::stoi(MemDecLine.at(0).LiteralToken);
  if (TotalMemSize <= 0)
    ShowError(MemDecLine.at(0), ErrorTypes::ZeroOrNegativeMemorySapces);
  for (int i = TotalMemSize - 1; i > -1; i--) {
    g_TotalMemPool.push(i);
    RawDataRepr EmptyRawData;
    SBMemory.push_back(EmptyRawData);
  }
  VariableDT tempVar;
  int VariableIDTemp = GetVariableID();
  g_VariableTable[VariableIDTemp] = tempVar;
}

void DeclareVariable(const TokenizedLineDT &VarDecLine) {
  TokenTypes DataType;
  bool Array = false;
  for (size_t idx = 0; idx < VarDecLine.size(); idx++) {
    TokenDT token = VarDecLine.at(idx);
    std::string LiteralToken = token.LiteralToken;
    if (LiteralToken == "]") {
      // resets array (and other but features not supported)
      Array = false;
    } else if (LiteralToken.size() > 1 && LiteralToken.front() == '~')
      for (const char flag : LiteralToken) {
        switch (flag) {
        case 'a':
          Array = true;
          break;
        case '~':
          break;
        case 'c':
          DataType = TokenTypes::CharVal;
          break;
        case 'b':
          DataType = TokenTypes::BoolVal;
          break;
        case 'i':
          DataType = TokenTypes::IntVal;
          break;
        case 's':
          DataType = TokenTypes::StringVal;
          break;
        case 'd':
          DataType = TokenTypes::DoubleVal;
          break;
        default:
          ShowError(token, ErrorTypes::InvalidFlag);
          break;
        }
        continue;
      }
    else if (ValidateName(
                 LiteralToken)) { // if variable name. (dec .var ~i MyNum).
                                  // "MyNum" is valid variable name
      if (DataType == TokenTypes::StringVal && Array)
        ShowError(token, ErrorTypes::AttemptedMultiDimensionalArrays);
      auto &CurrentMemory = global ? g_VariableTable : l_VariableTable;
      VariableDT Variable;
      Variable.DataType = DataType;
      if (Array)
        Variable.Array = true;
      else
        Variable.Array = false;
      int VariableID = GetVariableID();
      CurrentMemory[VariableID] = Variable;
      MapVariableNameAndID[LiteralToken] = VariableID;
      VarCount++;
    } else
      ShowError(token, ErrorTypes::GarbageArgInACommand);
  }
}

void decCommand(const TokenizedLineDT &DecLine) {
  if (DecLine.size() < 2) { // takes line with command sliced
    return;
  }
  std::string MLC = DecLine.at(0).LiteralToken;
  TokenizedLineDT LineArguments = SliceStuff(1, DecLine.size() - 1, DecLine);
  if (MLC == "mem")
    DeclareMemory(LineArguments);
  else if (MLC == "var") {
    if (g_TotalMemPool.size() == 0)
      ShowError(DecLine.at(0), ErrorTypes::MemoryFull);
    DeclareVariable(LineArguments);
  } else
    ShowError(DecLine.at(0), ErrorTypes::ExpectedAValidMidLineCommand);
}
