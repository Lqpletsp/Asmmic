#include "../errorhandling/ErrorHandler.h"
#include "../main/ImportantInternalFunctions.h"
#include "../main/types.h"
#include <iostream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace {
double OperateMathExpr();
bool OperateBoolExpr();
void AddNullChar();
std::pair<std::string, TokenTypes> GetDataFromToken();
bool CheckIfAppBCP() { return (BCP + 1 >= ByteCode.size()) ? false : true; }

int AllocateSBmemory() {
  int MemoryAdr = SMalloc();
  if (MemoryAdr < 0)
    ShowError(ByteCode.at(BCP), ErrorTypes::MemoryFull);
  return MemoryAdr;
}
void ReleaseMemoryFromStream() {
  VariableDT &SrcV = *GetVariableMetaData(0);
  if (SrcV.MemorySlotsAssigned.empty())
    ShowError(ByteCode.at(BCP), ErrorTypes::StreamVarEmpty);
  g_TotalMemPool.push(SrcV.MemorySlotsAssigned.front());
  SrcV.MemorySlotsAssigned.pop_front();
}
std::pair<TokenTypes, std::string> GetWholeDataFromStream() {
  VariableDT &SrcV = *GetVariableMetaData(0);
  std::string Data;
  TokenTypes DataType;

  auto ReleaseStream = [&]() {};
  g_TotalMemPool.push(SrcV.MemorySlotsAssigned.front());
  SrcV.MemorySlotsAssigned.pop_front();

  while (true) {
    if (SrcV.MemorySlotsAssigned.empty())
      ShowError(ByteCode.at(BCP), ErrorTypes::StreamVarEmpty);
    RawDataRepr TopData = SBMemory.at(SrcV.MemorySlotsAssigned.front());
    if (TopData.DataType == TokenTypes::Unknown) {
      ReleaseStream();
      break;
    }
    Data += TopData.Data;
    DataType = TopData.DataType;
    ReleaseStream();
  }
  if (DataType == TokenTypes::CharVal && Data.size() > 1)
    DataType = TokenTypes::StringVal;
  return {DataType, Data};
}
void ValidateType(const int &MemoryAdr1, const int &MemoryAdr2) {
  RawDataRepr Var1 = SBMemory.at(MemoryAdr1), Var2 = SBMemory.at(MemoryAdr2);
  if (!(Var1.DataType == Var2.DataType ||
        (Var1.DataType == TokenTypes::DoubleVal &&
         Var2.DataType == TokenTypes::IntVal) ||
        (Var1.DataType == TokenTypes::IntVal &&
         Var2.DataType == TokenTypes::DoubleVal)))
    ShowError(ByteCode.at(BCP), ErrorTypes::InvalidDataTypeInVariable);
}
void ValidateType(const TokenTypes &T1, const TokenTypes &T2) {
  if (!(T1 == T2 || (T1 == TokenTypes::DoubleVal && T2 == TokenTypes::IntVal) ||
        (T1 == TokenTypes::IntVal && T2 == TokenTypes::DoubleVal)))
    ShowError(ByteCode.at(BCP), ErrorTypes::InvalidDataTypeInVariable);
}

template <typename T> // only char or string as DataToStore
void InsertDataInSB(const T &DataToStore, const TokenTypes &DataType) {
  // function only stores at stream
  int MemoryAddressToStore;
  VariableDT &streamVar = *GetVariableMetaData(0);
  MemoryAddressToStore = AllocateSBmemory();
  SBMemory.at(MemoryAddressToStore).Data = DataToStore;
  SBMemory.at(MemoryAddressToStore).DataType = DataType;
  SBMemory.at(MemoryAddressToStore).VariableID = 0;
  streamVar.MemorySlotsAssigned.push_back(MemoryAddressToStore);
}
void InsertWholeDataInSB(const std::string &Data, const TokenTypes &DT) {
  switch (DT) {
  case (TokenTypes::StringVal):
    for (const char ch : Data) {
      InsertDataInSB(ch, TokenTypes::CharVal);
    }

    break;
  case (TokenTypes::IntVal):
  case (TokenTypes::TrueVal):
  case (TokenTypes::CharVal):
  case (TokenTypes::FalseVal):
  case (TokenTypes::DoubleVal):
    InsertDataInSB(Data, DT);
    break;
  default:
    std::cout << "FAILED:InsertWholeDataInSB;TTYPE:" << static_cast<int>(DT);

    break;
  }
  AddNullChar();
}
void AddNullChar() {
  int NullCharMemoryAdr = AllocateSBmemory();
  VariableDT &streamVar = *GetVariableMetaData(0);
  streamVar.MemorySlotsAssigned.push_back(NullCharMemoryAdr);
  SBMemory.at(NullCharMemoryAdr).Data = "-";
  SBMemory.at(NullCharMemoryAdr).DataType = TokenTypes::Unknown;
  SBMemory.at(NullCharMemoryAdr).VariableID = 0;
}
std::pair<int, int> ResolveArrays() {
  // called when ArrayHint type
  // Loops until it assumes the end of the array argument as given
  // returns the memory location that is being used inside the first array
  std::stack<int> VarStack, AddrStack;
  ByteCodeDT BCR = ByteCode.at(BCP);
  int VariableID = std::stoi(BCR.LiteralToken);
  VarStack.push(VariableID);
  BCP++;
  BCR = ByteCode.at(BCP);
  while (BCR.TypeRepr != TokenTypes::ArrEnd) {
    switch (BCR.TypeRepr) {
    case TokenTypes::MemoryAddressIndicator: {
      break;
    }
    case TokenTypes::ArrayHint:
      VarStack.push(std::stoi(BCR.LiteralToken));
      break;
    case TokenTypes::MemoryAddress:
      AddrStack.push(std::stoi(BCR.LiteralToken));
      break;
    case TokenTypes::VariableID: {
      VariableDT &AddrVariable =
          *GetVariableMetaData(std::stoi(BCR.LiteralToken));
      if (AddrVariable.MemorySlotsAssigned.size() <= 0)
        ShowError(BCR, ErrorTypes::EmptyArrayPointer);
      int MemoryAddr =
          std::stoi(SBMemory.at(AddrVariable.MemorySlotsAssigned.front()).Data);
      AddrStack.push(MemoryAddr);
      break;
    }
    default:
      std::cout << "--INVALID--ResolveArrays()-TypeMismatch-"
                   "PossibleErrorInLexer-NOTdisasmError\n";
      break;
    }

    ++BCP;
    if (!CheckIfAppBCP())
      break;
    BCR = ByteCode.at(BCP);
  }
  if (AddrStack.size() < 1 && VarStack.size() > 1) {
    ShowError(BCR,
              ErrorTypes::CannotOutputDataStructures); // invalid error message
  } else if (AddrStack.size() == 0 && VarStack.size() == 1)
    return {VarStack.top(), -1};
  while (VarStack.size() > 1 && AddrStack.size() == 1) {
    VariableID = VarStack.top();
    int Address = AddrStack.top();

    VariableDT &Arr = *GetVariableMetaData(VariableID);
    VarStack.pop();
    AddrStack.pop();
    AddrStack.push(
        std::stoi(SBMemory.at(Arr.MemorySlotsAssigned.front()).Data));
  }
  return {VarStack.top(), AddrStack.top()};
}
TokenTypes GetVariableDataType(const std::string &VarID) {
  return GetVariableMetaData(std::stoi(VarID))->DataType;
}

std::string GetArrayData() {
  // strings in arrays are handled by inserting the initial memory address to
  // the vector
  // at the end of the string, add a null character
  // when accessing that memory adderss,
  // loop until you collect all the char until that null
  // character
  auto [VarID, Addr] = ResolveArrays();
  VariableDT &Arr = *GetVariableMetaData(VarID);
  if (Addr < 0 || Addr >= Arr.MemorySlotsAssigned.size())
    ShowError(ByteCode.at(BCP), ErrorTypes::OutOfBounds);
  // strings require pointers
  return SBMemory.at(Arr.MemorySlotsAssigned.at(Addr)).Data;
}
void IncreaseMemorySpaces(const int &VariableID, const int &MemorySpaces) {
  VariableDT &Variable = *GetVariableMetaData(VariableID);
  for (int _ = 0; _ < MemorySpaces; _++) {
    int MemorySpaceALlocated = AllocateSBmemory();
    Variable.MemorySlotsAssigned.push_back(MemorySpaceALlocated);
    SBMemory.at(MemorySpaceALlocated).VariableID = VariableID;
    SBMemory.at(MemorySpaceALlocated).DataType = Variable.DataType;
  }
}
void ResolveWriteMode() {
  ByteCodeDT BCR = ByteCode.at(BCP);
  while (BCR.TypeRepr != TokenTypes::NewLine &&
         BCR.TypeRepr != TokenTypes::ENDCODE) {
    TokenTypes CurrentTokenType = BCR.TypeRepr;
    switch (CurrentTokenType) {
    case TokenTypes::VariableID: {
      VariableDT &SrcV = *GetVariableMetaData(0);
      VariableDT &DestV = *GetVariableMetaData(std::stoi(BCR.LiteralToken));

      if (DestV.MemorySlotsAssigned.size() > 1 &&
          DestV.DataType != TokenTypes::StringVal)
        ShowError(BCR, ErrorTypes::AttemptedToStoreInUnSupportedVariable);
      if (SrcV.MemorySlotsAssigned.empty())
        ShowError(BCR, ErrorTypes::StreamVarEmpty);
      int SrcAddr = SrcV.MemorySlotsAssigned.front();
      int DestAddr;
      int MemorySlotCounter = 0;
      TokenTypes SrcVDT = SBMemory.at(SrcAddr).DataType,
                 DestVDT = DestV.DataType;
      if (DestV.MemorySlotsAssigned.empty()) {
        DestAddr = AllocateSBmemory();
        if (DestVDT != TokenTypes::StringVal && DestVDT != TokenTypes::Unknown)
          SBMemory.at(DestAddr).DataType = DestV.DataType;
        else if (DestVDT == TokenTypes::StringVal)
          SBMemory.at(DestAddr).DataType = TokenTypes::CharVal;
        else if (DestVDT == TokenTypes::Unknown) {
          // to handle parameters or auto typed variables
          if (SrcV.MemorySlotsAssigned.size() > 1 &&
              SrcVDT == TokenTypes::CharVal) {
            SrcAddr = SrcV.MemorySlotsAssigned.at(1);
            SrcVDT = SBMemory.at(SrcAddr).DataType;
            if (SrcVDT == TokenTypes::CharVal) {
              DestV.DataType = TokenTypes::StringVal;
            }
            SrcAddr = SrcV.MemorySlotsAssigned.front();
            SrcVDT = SBMemory.at(SrcAddr).DataType;
            if (DestV.DataType == TokenTypes::Unknown)
              DestV.DataType = SrcVDT;
          } else
            DestV.DataType = SrcVDT;

          SBMemory.at(DestAddr).DataType = SrcVDT;
        }
        DestV.MemorySlotsAssigned.push_back(DestAddr);
      } else
        DestAddr = DestV.MemorySlotsAssigned.at(0);

      ValidateType(SrcAddr, DestV.MemorySlotsAssigned.front());

      std::string Data;

      while (SBMemory.at(SrcAddr).DataType != TokenTypes::Unknown) {
        if (MemorySlotCounter > 0 && DestV.DataType == TokenTypes::CharVal) {
          ShowError(BCR, ErrorTypes::InvalidDataTypeInVariable);
        }
        if (MemorySlotCounter >= DestV.MemorySlotsAssigned.size()) {
          int MemoryAllocated = AllocateSBmemory();
          SBMemory.at(MemoryAllocated).DataType = SBMemory.at(SrcAddr).DataType;
          DestV.MemorySlotsAssigned.push_back(MemoryAllocated);
        }
        if (DestVDT == TokenTypes::IntVal && SrcVDT == TokenTypes::DoubleVal &&
            SBMemory.at(SrcAddr).Data != "")
          // convert to int
          Data = std::to_string(std::stoi(SBMemory.at(SrcAddr).Data));
        else
          Data = SBMemory.at(SrcAddr).Data;
        DestAddr = DestV.MemorySlotsAssigned.at(MemorySlotCounter);
        SBMemory.at(DestAddr).Data = Data;
        SBMemory.at(SrcAddr).DataType = TokenTypes::Empty;
        SBMemory.at(SrcAddr).Data = "";
        ReleaseMemoryFromStream();
        SrcAddr = SrcV.MemorySlotsAssigned.front();
        MemorySlotCounter++;
      }
      ReleaseMemoryFromStream(); // to remove the null char
      break;
    }
    case TokenTypes::ArrayHint: {
      auto [VarID, ArrayIdx] = ResolveArrays();
      VariableDT &DestV = *GetVariableMetaData(VarID),
                 &SrcV = *GetVariableMetaData(0);
      int SrcAddr = SrcV.MemorySlotsAssigned.front();
      if (SrcV.MemorySlotsAssigned.empty())
        ShowError(BCR, ErrorTypes::StreamVarEmpty);
      if (ArrayIdx < 0)
        ShowError(BCR, ErrorTypes::NoArrayIndexGiven);
      if (DestV.MemorySlotsAssigned.empty())
        ShowError(BCR, ErrorTypes::OutOfBounds);

      int DestAddr = DestV.MemorySlotsAssigned.at(ArrayIdx);

      ValidateType(SrcAddr, DestV.MemorySlotsAssigned.front());
      if (DestV.MemorySlotsAssigned.size() == 0)
        DestV.MemorySlotsAssigned.push_back(AllocateSBmemory());
      SBMemory.at(DestAddr).Data = SBMemory.at(SrcAddr).Data;
      SBMemory.at(DestAddr).DataType = SBMemory.at(SrcAddr).DataType;

      ReleaseMemoryFromStream(); // to remove the data
      ReleaseMemoryFromStream(); // to remove the null char
      break;
    }

    default:
      ShowError(BCR, ErrorTypes::GarbageArgInACommand);
      break;
    }
    BCP++;
    if (!CheckIfAppBCP())
      return;
    BCR = ByteCode.at(BCP);
  }
}
std::pair<std::string, TokenTypes> GetDataFromToken() {
  // returns the value of a token
  // if a value constant or a literal value (int, dobule, bool, string, char),
  // returns that
  // if a variable, returns the data stored in it
  ByteCodeDT BCR = ByteCode.at(BCP);
  std::string Data = BCR.LiteralToken;
  TokenTypes type = BCR.TypeRepr;
  switch (BCR.TypeRepr) {
  case TokenTypes::StringVal:
  case TokenTypes::CharVal:
  case TokenTypes::IntVal:
  case TokenTypes::DoubleVal:
  case TokenTypes::BoolVal:
    break;
  case TokenTypes::BoolExpr: {
    Data = (OperateBoolExpr()) ? "T" : "F";
    type = (Data == "T") ? TokenTypes::TrueVal : TokenTypes::FalseVal;
    break;
  }
  case TokenTypes::VariableID: {
    VariableDT &srcVar = *GetVariableMetaData(std::stoi(BCR.LiteralToken));
    int DataMAdr;
    Data = "";
    for (int idx = 0; idx < srcVar.MemorySlotsAssigned.size(); ++idx) {
      DataMAdr = srcVar.MemorySlotsAssigned.at(idx);
      Data += SBMemory.at(DataMAdr).Data;
    }
    type = srcVar.DataType;
    break;
  }
  case TokenTypes::ArrayHint: {
    VariableDT &Arr = *GetVariableMetaData(std::stoi(BCR.LiteralToken));
    std::string PrintData = GetArrayData();
    if (Arr.DataType == TokenTypes::BoolVal) {
      if (PrintData == "T")
        Data = "true";
      else
        Data = "false";
    } else
      Data = PrintData;
    type = Arr.DataType;
    break;
  }
  case TokenTypes::MathExpr:
    Data = std::to_string(OperateMathExpr());
    type = TokenTypes::DoubleVal;
    break;
  case TokenTypes::Colon:
    Data = "-";
    break;
  default:
    ShowError(BCR, ErrorTypes::GarbageArgInACommand);
    break;
  }

  return {Data, type};
}
bool OperateBoolExpr() {
  BCP++;

  struct EvalItem {
    std::string Data;
    TokenTypes DataType;
  };

  std::stack<EvalItem> EvalStack;

  auto ToDouble = [](const std::string &val) -> double {
    try {
      return std::stod(val);
    } catch (...) {
      return 0.0;
    }
  };

  auto EvaluateComparison = [&](const EvalItem &lhs, const EvalItem &rhs,
                                TokenTypes op) -> bool {
    bool isLhsNumeric = (lhs.DataType == TokenTypes::IntVal ||
                         lhs.DataType == TokenTypes::DoubleVal ||
                         lhs.DataType == TokenTypes::CharVal);
    bool isRhsNumeric = (rhs.DataType == TokenTypes::IntVal ||
                         rhs.DataType == TokenTypes::DoubleVal ||
                         rhs.DataType == TokenTypes::CharVal);

    if (isLhsNumeric && isRhsNumeric) {
      double numL = ToDouble(lhs.Data);
      double numR = ToDouble(rhs.Data);

      switch (op) {
      case TokenTypes::Equal:
        return numL == numR;
      case TokenTypes::LessThan:
        return numL < numR;
      case TokenTypes::GreaterThan:
        return numL > numR;
      case TokenTypes::LessEqual:
        return numL <= numR;
      case TokenTypes::GreaterEqual:
        return numL >= numR;
      case TokenTypes::NotEqual:
        return numL != numR;
      default:
        return false;
      }
    }

    switch (op) {
    case TokenTypes::Equal:
      return lhs.Data == rhs.Data;
    case TokenTypes::LessThan:
      return lhs.Data < rhs.Data;
    case TokenTypes::GreaterThan:
      return lhs.Data > rhs.Data;
    case TokenTypes::LessEqual:
      return lhs.Data <= rhs.Data;
    case TokenTypes::GreaterEqual:
      return lhs.Data >= rhs.Data;
    case TokenTypes::NotEqual:
      return lhs.Data != rhs.Data;
    default:
      return false;
    }
  };

  if (!CheckIfAppBCP())
    return false;

  // FETCH INSIDE THE LOOP OR CONDITION
  while (BCP < ByteCode.size() &&
         ByteCode.at(BCP).TypeRepr != TokenTypes::BoolExprEnd) {
    ByteCodeDT BCR =
        ByteCode.at(BCP); // <--- Fetch current bytecode instruction here!

    switch (BCR.TypeRepr) {
    case TokenTypes::IntVal:
    case TokenTypes::StringVal:
    case TokenTypes::CharVal:
    case TokenTypes::DoubleVal:
    case TokenTypes::TrueVal:
    case TokenTypes::FalseVal: {
      EvalStack.push({BCR.LiteralToken, BCR.TypeRepr});
      break;
    }

    case TokenTypes::VariableID: {
      auto [data, DT] = GetDataFromToken();
      EvalStack.push({data, DT});
      break;
    }
    case TokenTypes::MathExpr: {
      std::string Result = std::to_string(OperateMathExpr());
      EvalStack.push({Result, TokenTypes::DoubleVal});
      break;
    }

    case TokenTypes::ArrayHint: {
      std::string data = GetArrayData();
      TokenTypes dataType =
          GetVariableMetaData(std::stoi(BCR.LiteralToken))->DataType;
      EvalStack.push({data, dataType});
      break;
    }

    case TokenTypes::LessEqual:
    case TokenTypes::GreaterEqual:
    case TokenTypes::Equal:
    case TokenTypes::LessThan:
    case TokenTypes::GreaterThan:
    case TokenTypes::NotEqual: {
      if (EvalStack.size() < 2) {
        ShowError(BCR, ErrorTypes::InvalidBooleanExpression);
      }
      EvalItem rhs = EvalStack.top();
      EvalStack.pop();
      EvalItem lhs = EvalStack.top();
      EvalStack.pop();

      bool result = EvaluateComparison(lhs, rhs, BCR.TypeRepr);
      EvalStack.push({result ? "T" : "F", TokenTypes::BoolVal});
      break;
    }

    case TokenTypes::And: {
      if (EvalStack.size() < 2) {
        ShowError(BCR, ErrorTypes::InvalidBooleanExpression);
      }
      EvalItem rhs = EvalStack.top();
      EvalStack.pop();
      EvalItem lhs = EvalStack.top();
      EvalStack.pop();

      bool b1 = (lhs.Data == "T");
      bool b2 = (rhs.Data == "T");

      EvalStack.push({(b1 && b2) ? "T" : "F", TokenTypes::BoolVal});
      break;
    }

    case TokenTypes::Or: {
      if (EvalStack.size() < 2) {
        ShowError(BCR, ErrorTypes::InvalidBooleanExpression);
      }
      EvalItem rhs = EvalStack.top();
      EvalStack.pop();
      EvalItem lhs = EvalStack.top();
      EvalStack.pop();

      bool b1 = (lhs.Data == "T");
      bool b2 = (rhs.Data == "T");

      EvalStack.push({(b1 || b2) ? "T" : "F", TokenTypes::BoolVal});
      break;
    }

    case TokenTypes::Not: {
      if (EvalStack.empty()) {
        ShowError(BCR, ErrorTypes::InvalidBooleanExpression);
      }
      EvalItem item = EvalStack.top();
      EvalStack.pop();

      bool b = (item.Data == "T");
      EvalStack.push({(!b) ? "T" : "F", TokenTypes::BoolVal});
      break;
    }

    default:
      break;
    }

    ++BCP;
  }

  if (EvalStack.empty()) {
    return false;
  }

  return (EvalStack.top().Data == "T");
}

double OperateMathExpr() {
  // the function is called when MathExpr byte code
  BCP++;
  std::stack<double> EvalStack;

  if (CheckIfAppBCP()) {

    ByteCodeDT BCR = ByteCode.at(BCP);
    double a, b; // numbers to pop and evaluate
    while (BCR.TypeRepr != TokenTypes::MathExprEnd) {
      switch (BCR.TypeRepr) {
      case TokenTypes::IntVal:
      case TokenTypes::DoubleVal:
        EvalStack.push(std::stod(BCR.LiteralToken));
        break;
      case TokenTypes::VariableID: {
        TokenTypes DT =
            GetVariableMetaData(std::stoi(BCR.LiteralToken))->DataType;
        if (DT != TokenTypes::IntVal && DT != TokenTypes::DoubleVal)
          ShowError(BCR, ErrorTypes::NonDigitDataForclc);
        EvalStack.push(std::stod(GetDataFromToken().first));
        break;
      }
      case TokenTypes::ArrayHint: {
        TokenTypes DT =
            GetVariableMetaData(std::stoi(BCR.LiteralToken))->DataType;
        if (DT != TokenTypes::IntVal && DT != TokenTypes::DoubleVal)
          ShowError(BCR, ErrorTypes::NonDigitDataForclc);
        EvalStack.push(std::stod(GetArrayData()));
        break;
      }
      case TokenTypes::Add:
      case TokenTypes::Mlt:
      case TokenTypes::Div:
      case TokenTypes::Min:
        a = EvalStack.top();
        EvalStack.pop();
        b = EvalStack.top();
        EvalStack.pop();
        switch (BCR.TypeRepr) {
        case TokenTypes::Add:
          EvalStack.push(a + b);
          break;
        case TokenTypes::Min:
          EvalStack.push(b - a);
          break;
        case TokenTypes::Mlt:
          EvalStack.push(a * b);
          break;
        case TokenTypes::Div:
          EvalStack.push(b / a);
          break;
        default:
          break;
        }
        break;
      default:
        break;
      }
      ++BCP;
      if (!CheckIfAppBCP())
        break;
      BCR = ByteCode.at(BCP);
    }
  }
  return EvalStack.top();
}

bool ResolveReadMode(TokenTypes cmd) {
  ByteCodeDT BCR = ByteCode.at(BCP); // no issue here
  Mode CurrentState = Read;
  bool NoOtherThanInt = cmd == TokenTypes::mlc;
  while ((BCR.TypeRepr != TokenTypes::NewLine &&
          BCR.TypeRepr != TokenTypes::ENDCODE) &&
         CurrentState == Read) { // no issue on the first iteration

    auto [Dat, DTP] = GetDataFromToken();
    if (DTP == TokenTypes::Colon) {
      CurrentState = Write;
      break;
    }
    InsertWholeDataInSB(Dat, DTP);
    ++BCP;
    if (!CheckIfAppBCP())
      return false;
    BCR = ByteCode.at(BCP);
  }
  return (CurrentState == Write) ? true : false;
}
std::pair<std::string, TokenTypes> GetTopStreamData() {
  VariableDT &SrcV = *GetVariableMetaData(0);
  if (SrcV.MemorySlotsAssigned.empty())
    ShowError(ByteCode.at(BCP), ErrorTypes::StreamVarEmpty);
  TokenTypes DT = SBMemory.at(SrcV.MemorySlotsAssigned.front()).DataType;
  std::string Data = SBMemory.at(SrcV.MemorySlotsAssigned.front()).Data;
  ReleaseMemoryFromStream(); // to remove the data
  return {Data, DT};
}

} // namespace
void outCommand() {
  // when called, increment BCP to access the first line argument
  BCP++;
  ByteCodeDT BCR = ByteCode.at(BCP);
  while (BCR.TypeRepr != TokenTypes::NewLine &&
         BCR.TypeRepr != TokenTypes::ENDCODE) {
    auto [Data, DT] = GetDataFromToken();
    InsertWholeDataInSB(Data, DT);
    while (true) {
      auto [DT2, DTP] = GetTopStreamData();
      if (DTP == TokenTypes::Unknown)
        break;
      std::cout << DT2;
    }
    BCP++;
    if (!CheckIfAppBCP())
      return;
    BCR = ByteCode.at(BCP);
  }
}
void Handlegotoln() {
  // called when gotoln token
  // if expression and valid, do not change the BCP
  // if no expression or invalid, change the BCP to go to
  int FallBackBCP = BCP;
  ++BCP;
  ByteCodeDT token = ByteCode.at(BCP);
  switch (token.TypeRepr) {
  case (TokenTypes::BoolExpr):
    if (OperateBoolExpr())
      return;
    else {
      BCP = std::stoi(ByteCode.at(FallBackBCP).LiteralToken);
    }
    break;
  case (TokenTypes::Module): {
    InterpreterModuleStack.push(BCP);
    // gotoln token contains the bytecode index to go
    int ModID = std::stoi(ByteCode.at(BCP).LiteralToken);
    ModuleDT Mod = GetModuleMetaData(ModID);
    BCP = Mod.ByteCodeStart + 1;
    break;
  }
  default:
    --BCP;
    token = ByteCode.at(BCP);
    try {
      BCP = std::stoi(token.LiteralToken);
    } catch (...) {
      if (token.LiteralToken == "!") {
        BCP = InterpreterModuleStack.top() + 1;
        InterpreterModuleStack.pop();
        c_VariableTable = &g_VariableTable;
      }
    }
    break;
  }
}
void setCommand() {
  ++BCP;
  if (ResolveReadMode(TokenTypes::set)) {
    ++BCP;
    ResolveWriteMode();
  }
}
void HandleModule() {
  ByteCodeDT BCR = ByteCode.at(BCP);
  int ModID = std::stoi(BCR.LiteralToken);
  // since the bytecode was validated, no need to check whether the code is
  // valid or not
  ModuleDT ModMD = GetModuleMetaData(ModID);
  c_VariableTable = &LocalModuleVariableTable[ModID];
}
void mlcCommand() {
  ++BCP;
  ResolveReadMode(TokenTypes::set);
  ByteCodeDT BCR = ByteCode.at(BCP);
  while (BCR.TypeRepr != TokenTypes::NewLine &&
         BCR.TypeRepr != TokenTypes::ENDCODE) {

    TokenTypes CurrentTokenType = BCR.TypeRepr;
    switch (CurrentTokenType) {

    case TokenTypes::ArrayHint: {
      // need to resolve nested experssions
      // for example: ArrPtrs@Numbers (after pointers are resolved)
      int VariableID = std::stoi(BCR.LiteralToken);
      VariableDT &DestV = *GetVariableMetaData(VariableID);

      TokenTypes DestVDT = DestV.DataType;
      if (!DestV.Array)
        ShowError(BCR, ErrorTypes::CannotTransformStaticVaribles);

      auto [VarID, Arridx] = ResolveArrays();

      if (Arridx >= 0)
        ShowError(BCR, ErrorTypes::CannotTransformStaticVaribles);

      auto [MemorySpaceAllocated, DT] = GetTopStreamData();
      if (DT != TokenTypes::IntVal)
        ShowError(BCR, ErrorTypes::InvalidTypeForNumberOfAllocations);
      int Allocated = std::stoi(MemorySpaceAllocated);
      if (Allocated < 0)
        ShowError(BCR, ErrorTypes::ZeroOrNegativeMemoryAllocation);

      IncreaseMemorySpaces(VariableID, Allocated);
      break;
    }
    default:
      ShowError(BCR, ErrorTypes::GarbageArgInACommand);
      break;
    }

    ++BCP;
    if (!CheckIfAppBCP())
      return;
    BCR = ByteCode.at(BCP);
  }
}
