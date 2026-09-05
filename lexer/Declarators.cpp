#include "../errorhandling/ErrorHandler.h"
#include "../main/ImportantInternalFunctions.h"
#include "../main/types.h"

namespace {
bool AppendVariableDetails(const std::string &VName, const bool &Array,
                           const TokenTypes &DT) {
  if (DT == TokenTypes::StringVal && Array)
    return false;
  VariableDT Variable{.DataType = DT, .Array = Array};

  int VariableID = GetVariableID();
  (*c_VariableTable)[VariableID] = Variable;
  (*c_MapVariableNameAndID)[VName] = VariableID;
  VarCount++;
  return true;
}
} // namespace
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
  auto AddNewLine = [&]() {
    ByteCode.push_back({
        .LiteralToken = "",
        .LineNum = -1,
        .ColNum = -1,
        .TypeRepr = TokenTypes::NewLine,
    });
  };
  if (ModDecLine.empty())
    return;
  int ModID = GetModuleID();
  MapModuleNameAndID[ModDecLine.at(0).LiteralToken] = ModID;
  ByteCode.push_back({
      .LiteralToken = "",
      .LineNum = -1,
      .ColNum = -1,
      .TypeRepr = TokenTypes::gotoln,
  });
  int BCi = ByteCode.size();
  TrackModuleDecLine.push(BCi - 1);

  ModuleDT ModuleInfo = {.ModuleID = ModID, .ByteCodeStart = BCi};
  AddNewLine();
  ByteCode.push_back({
      .LiteralToken = std::to_string(ModID),
      .LineNum = -1,
      .ColNum = -1,
      .TypeRepr = TokenTypes::Module,
  });

  AddNewLine();

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
  ByteCode.push_back({.LiteralToken = "set",
                      .LineNum = -1,
                      .ColNum = -1,
                      .TypeRepr = TokenTypes::set});
  ByteCode.push_back({.LiteralToken = ":",
                      .LineNum = -1,
                      .ColNum = -1,
                      .TypeRepr = TokenTypes::Colon});
  for (size_t i = 2; i < ModDecLine.size(); ++i) {
    TokenDT token = ModDecLine.at(i);
    AppendVariableDetails(token.LiteralToken, false, TokenTypes::Unknown);
    int VarID = (*c_MapVariableNameAndID)[token.LiteralToken];
    int LiN = token.LineNum, CoN = token.ColNum;
    ByteCode.push_back({.LiteralToken = std::to_string(VarID),
                        .LineNum = LiN,
                        .ColNum = CoN,
                        .TypeRepr = TokenTypes::VariableID});
  }
  AddNewLine();
  ModuleTable[ModID] = ModuleInfo;
  LocalModuleVariableTable[ModID] = {};
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
      if (!AppendVariableDetails(LiteralToken, Array, DataType))
        ShowError(token, ErrorTypes::AttemptedMultiDimensionalArrays);
    } else
      ShowError(token, ErrorTypes::GarbageArgInACommand);
  }
}

void decCommand(const TokenizedLineDT &DecLine) {
  if (DecLine.size() < 2) { // takes line with command sliced
    return;
  }
  auto CheckMemory = [&]() {
    if (g_TotalMemPool.size() == 0)
      ShowError(DecLine.at(0), ErrorTypes::MemoryFull);
  };
  std::string MLC = DecLine.at(0).LiteralToken;
  TokenizedLineDT LineArguments = SliceStuff(1, DecLine.size() - 1, DecLine);
  if (MLC == ".mem")
    DeclareMemory(LineArguments);
  else if (MLC == ".var") {
    CheckMemory();
    DeclareVariable(LineArguments);
  } else if (MLC == ".mod") {
    CheckMemory();
    DeclareModules(LineArguments);
  } else
    ShowError(DecLine.at(0), ErrorTypes::ExpectedAValidMidLineCommand);
}
