#include "../../errorhandling/ErrorHandler.h"
#include "../../main/ImportantInternalFunctions.h"
#include "../../main/types.h"
#include "../VM.h"
#include <iostream>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace {
double OperateMathExpr();

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
void ValidateType(const int &MemoryAdr1, const int &MemoryAdr2) {
  RawDataRepr Var1 = SBMemory.at(MemoryAdr1), Var2 = SBMemory.at(MemoryAdr2);
  if (!(Var1.DataType == Var2.DataType ||
        (Var1.DataType == TokenTypes::DoubleVal &&
         Var2.DataType == TokenTypes::IntVal) ||
        (Var1.DataType == TokenTypes::IntVal &&
         Var2.DataType == TokenTypes::DoubleVal)))
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
  while (BCR.TypeRepr != TokenTypes::NewLine &&
         BCR.TypeRepr != TokenTypes::End &&
         BCR.TypeRepr != TokenTypes::ArrEnd) {
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
      std::cout << BCR.LiteralToken << std::endl;
      std::cout << static_cast<int>(BCR.TypeRepr) << std::endl;
      std::cout << "--INVALID--ResolveArrays()-TypeMismatch-"
                   "PossibleErrorInLexer-NOTdisasmError\n";
      break;
    }
    BCP++;
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
         BCR.TypeRepr != TokenTypes::End) {
    TokenTypes CurrentTokenType = BCR.TypeRepr;
    switch (CurrentTokenType) {
    case TokenTypes::VariableID: {
      // what if the DestV is char but the stream has string?
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
      if (DestV.MemorySlotsAssigned.empty()) {
        DestAddr = AllocateSBmemory();
        if (DestV.DataType != TokenTypes::StringVal)
          SBMemory.at(DestAddr).DataType = DestV.DataType;
        else
          SBMemory.at(DestAddr).DataType = TokenTypes::CharVal;
        DestV.MemorySlotsAssigned.push_back(DestAddr);
      } else
        DestAddr = DestV.MemorySlotsAssigned.at(0);

      ValidateType(SrcAddr, DestV.MemorySlotsAssigned.front());

      while (SBMemory.at(SrcAddr).DataType != TokenTypes::Unknown) {
        if (MemorySlotCounter > 0 && DestV.DataType == TokenTypes::CharVal) {
          ShowError(BCR, ErrorTypes::InvalidDataTypeInVariable);
        }
        if (MemorySlotCounter >= DestV.MemorySlotsAssigned.size()) {
          int MemoryAllocated = AllocateSBmemory();
          SBMemory.at(MemoryAllocated).DataType = SBMemory.at(SrcAddr).DataType;
          DestV.MemorySlotsAssigned.push_back(MemoryAllocated);
        }
        DestAddr = DestV.MemorySlotsAssigned.at(MemorySlotCounter);
        SBMemory.at(DestAddr).Data = SBMemory.at(SrcAddr).Data;
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
std::string GetDataFromToken() {
  // returns the value of a token
  // if a value constant or a literal value (int, dobule, bool, string, char),
  // returns that
  // if a variable, returns the data stored in it
  ByteCodeDT BCR = ByteCode.at(BCP);
  std::string Data = BCR.LiteralToken;
  switch (BCR.TypeRepr) {
  case TokenTypes::StringVal:
  case TokenTypes::CharVal:
  case TokenTypes::IntVal:
  case TokenTypes::DoubleVal:
    break;
  case TokenTypes::Flag:
    for (const char &ch : BCR.LiteralToken) {
      switch (ch) {
      case 'n':
        Data = "\n";
        break;
      default:
        ShowError(BCR, ErrorTypes::InvalidFlag);
      }
    }
    break;
  case TokenTypes::BoolVal:
    if (Data == "T")
      Data = "true";
    else
      Data = "false";
    break;
  case TokenTypes::VariableID: {
    VariableDT &srcVar = *GetVariableMetaData(std::stoi(BCR.LiteralToken));
    int DataMAdr;
    for (int idx = 0; idx < srcVar.MemorySlotsAssigned.size(); idx++) {
      DataMAdr = srcVar.MemorySlotsAssigned.at(idx);
      Data = SBMemory.at(DataMAdr).Data;
    }
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
    break;
  }
  case TokenTypes::MathExpr:
    Data = std::to_string(OperateMathExpr());
    break;
  default:
    ShowError(BCR, ErrorTypes::GarbageArgInACommand);
    break;
  }

  return Data;
}
double OperateMathExpr() {
  // the function is called when MathExpr byte code
  BCP++;
  std::stack<double> EvalStack;

  if (CheckIfAppBCP()) {

    ByteCodeDT BCR = ByteCode.at(BCP);
    double a, b; // numbers to pop and evaluate
    while (BCR.TypeRepr != TokenTypes::End &&
           BCR.TypeRepr != TokenTypes::NewLine &&
           BCR.TypeRepr != TokenTypes::MathExprEnd) {
      switch (BCR.TypeRepr) {
      case TokenTypes::IntVal:
      case TokenTypes::DoubleVal:
        EvalStack.push(std::stoi(BCR.LiteralToken));
        break;
      case TokenTypes::VariableID: {
        TokenTypes DT =
            GetVariableMetaData(std::stoi(BCR.LiteralToken))->DataType;
        if (DT != TokenTypes::IntVal && DT != TokenTypes::DoubleVal)
          ShowError(BCR, ErrorTypes::NonDigitDataForclc);
        EvalStack.push(std::stoi(GetDataFromToken()));
        break;
      }
      case TokenTypes::ArrayHint: {
        TokenTypes DT =
            GetVariableMetaData(std::stoi(BCR.LiteralToken))->DataType;
        if (DT != TokenTypes::IntVal && DT != TokenTypes::DoubleVal)
          ShowError(BCR, ErrorTypes::NonDigitDataForclc);
        EvalStack.push(std::stoi(GetArrayData()));
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
    }
  }
  return EvalStack.top();
}

void ResolveReadMode(TokenTypes cmd) {
  ByteCodeDT BCR = ByteCode.at(BCP); // no issue here
  Mode CurrentState = Read;
  bool NoOtherThanInt = cmd == TokenTypes::mlc;
  while (BCR.TypeRepr != TokenTypes::NewLine &&
         BCR.TypeRepr != TokenTypes::End) { // no issue on the first iteration

    TokenTypes CurrentTokenType = BCR.TypeRepr;
    switch (CurrentTokenType) {
    case TokenTypes::Colon:
      CurrentState = Write;
      BCP++;
      return;

    case TokenTypes::IntVal:
    case TokenTypes::CharVal:
    case TokenTypes::DoubleVal:
    case TokenTypes::BoolVal: {
      if (NoOtherThanInt) {
        if (CurrentTokenType != TokenTypes::IntVal)
          ShowError(BCR, ErrorTypes::GarbageArgInACommand);
        else if (std::stoi(BCR.LiteralToken) < 0)
          ShowError(BCR, ErrorTypes::ZeroOrNegativeMemoryAllocation);
      }
      InsertDataInSB(BCR.LiteralToken, CurrentTokenType);
      AddNullChar();
      break; // Breaks inner switch
    }
    case TokenTypes::MathExpr: {
      int Value = OperateMathExpr();
      InsertDataInSB(std::to_string(Value), TokenTypes::DoubleVal);
      AddNullChar();
      break;
    }
    case TokenTypes::StringVal: {
      if (NoOtherThanInt)
        ShowError(BCR, ErrorTypes::GarbageArgInACommand);
      int MemoryAddressToStore;
      VariableDT &streamVar = *GetVariableMetaData(0);
      for (int idx = 0; idx < BCR.LiteralToken.size(); idx++) {
        InsertDataInSB(BCR.LiteralToken.at(idx), TokenTypes::CharVal);
      }
      AddNullChar();
      break;
    }
    case TokenTypes::VariableID: {
      int MemoryAddressToStore;
      VariableDT &SrcVar = *GetVariableMetaData(std::stoi(BCR.LiteralToken));
      VariableDT &StreamVar = *GetVariableMetaData(0);
      if (NoOtherThanInt && SrcVar.DataType != TokenTypes::IntVal)
        ShowError(BCR, ErrorTypes::GarbageArgInACommand);
      for (int idx = 0; idx < SrcVar.MemorySlotsAssigned.size(); idx++) {
        int SrcDataAdr = SrcVar.MemorySlotsAssigned.at(idx);
        InsertDataInSB(SBMemory.at(SrcDataAdr).Data,
                       SBMemory.at(SrcDataAdr).DataType);
      }
      AddNullChar();
      break;
    }
    case TokenTypes::ArrayHint: {
      if (NoOtherThanInt &&
          GetVariableDataType(BCR.LiteralToken) != TokenTypes::IntVal)
        ShowError(BCR, ErrorTypes::GarbageArgInACommand);
      std::string Data = GetArrayData();
      VariableDT &ArrVar = *GetVariableMetaData(std::stoi(BCR.LiteralToken));
      if (ArrVar.DataType == TokenTypes::StringVal) {
        for (size_t idx = 0; idx < Data.size(); idx++) {
          InsertDataInSB(Data.at(idx), TokenTypes::CharVal);
        }
        AddNullChar();
      }
      InsertDataInSB(Data, BCR.TypeRepr);
      AddNullChar();
      break;
    }
    case TokenTypes::NewLine:
      return;
    default:
      ShowError(BCR, ErrorTypes::GarbageArgInACommand);
    }
    BCP++;
    if (!CheckIfAppBCP())
      return;
    BCR = ByteCode.at(BCP);
  }
  BCP++;
}
std::pair<TokenTypes, std::string> GetTopStreamData() {
  VariableDT &SrcV = *GetVariableMetaData(0);
  if (SrcV.MemorySlotsAssigned.empty())
    ShowError(ByteCode.at(BCP), ErrorTypes::StreamVarEmpty);
  TokenTypes DT = SBMemory.at(SrcV.MemorySlotsAssigned.front()).DataType;
  std::string Data = SBMemory.at(SrcV.MemorySlotsAssigned.front()).Data;
  ReleaseMemoryFromStream(); // to remove the int data
  ReleaseMemoryFromStream(); // to remove the null char
  return {DT, Data};
}
} // namespace

void outCommand() {
  // when called, increment BCP to access the first line argument
  BCP++;
  ByteCodeDT BCR = ByteCode.at(BCP);
  while (BCR.TypeRepr != TokenTypes::NewLine &&
         BCR.TypeRepr != TokenTypes::End) {
    std::cout << GetDataFromToken();
    BCP++;
    if (!CheckIfAppBCP())
      return;
    BCR = ByteCode.at(BCP);
  }
}

void setCommand() {
  BCP++;
  ResolveReadMode(TokenTypes::set);
  if (!CheckIfAppBCP())
    return;
  ResolveWriteMode();
}

void mlcCommand() {
  BCP++;
  ResolveReadMode(TokenTypes::set);
  ByteCodeDT BCR = ByteCode.at(BCP);
  while (BCR.TypeRepr != TokenTypes::NewLine &&
         BCR.TypeRepr != TokenTypes::End) {

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

      auto [DT, MemorySpaceAllocated] = GetTopStreamData();
      int Allocated = std::stoi(MemorySpaceAllocated);
      if (Allocated < 0)
        ShowError(BCR, ErrorTypes::ZeroOrNegativeMemoryAllocation);
      if (DT != TokenTypes::IntVal)
        ShowError(BCR, ErrorTypes::InvalidTypeForNumberOfAllocations);

      IncreaseMemorySpaces(VariableID, Allocated);
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
