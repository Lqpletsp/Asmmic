#include "../errorhandling/ErrorHandler.h"
#include "../main/ImportantInternalFunctions.h"
#include "../main/types.h"

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
  for (int i = TotalMemSize - 1; i > -1; --i) {
    g_TotalMemPool.push(i);
    RawDataRepr EmptyRawData;
    SBMemory.push_back(EmptyRawData);
  }
  VariableDT tempVar;
  int VariableIDTemp = GetVariableID();
  g_VariableTable[VariableIDTemp] = tempVar;
}
void DeclareModules(const TokenizedLineDT &ModDecLine) {
  if (ModDecLine.empty())
    return;
  int ModID = GetModuleID();
  MapModuleNameAndID[ModDecLine.at(0).LiteralToken] = ModID;
  ModuleDT ModuleInfo = {ModID};
  if (ModDecLine.size() < 2)
    return;
  else if (ModDecLine.at(1).LiteralToken != ":")
    ShowError(ModDecLine.at(2), ErrorTypes::NoParameterIndication);
  VariableDT parameter{
      .DataType = TokenTypes::Unknown,
      .MemorySlotsAssigned = {},
      .Array = false,
  };
  global = false;
  c_VariableTable = &l_VariableTable;
  c_MapVariableNameAndID = &l_MapVariableNameAndID;
  for (size_t i = 2; i < ModDecLine.size(); ++i) {
    int VarID = GetVariableID();
    ModuleInfo.Parameters.push_back(parameter);
    (*c_MapVariableNameAndID)[ModDecLine.at(i).LiteralToken] = VarID;
    (*c_VariableTable)[VarID] = parameter;
  }
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
      VariableDT Variable{.DataType = DataType, .Array = Array};

      int VariableID = GetVariableID();
      (*c_VariableTable)[VariableID] = Variable;
      (*c_MapVariableNameAndID)[LiteralToken] = VariableID;
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
