#pragma once
#include "types.h"
#include <algorithm>
#include <array>
#include <iterator>
#include <sstream>
#include <string>

int SMalloc();
bool ValidateName(const std::string &Name);
TokenTypes DetermineDataType(const std::string &Token);
bool CheckIfValidGlobalVariable(const int &VariableID);
int GetAssignedVariableID(const std::string &VariableName);
VariableDT *GetVariableMetaData(const int &VariableID);
std::string GetDataFromAddress(const std::string &StrAddress);
bool ValidateDataTypes(const TokenTypes &DataType1,
                       const TokenTypes &DataType2);
void StoreDataInAddress(const std::string &StrAddress,
                        const std::string &DataToStore);
RawDataRepr *GetMemorySpaceMetaData(const std::string &StrAddress);
int GetVariableID();
void RSMemory(const int &AddressToClear, VariableDT &Variable,
              const int &VariableOwnedIDX);
template <typename T>
T SliceStuff(const int Start, const int End, const T &DataToSlice) {
  const int dataSize = static_cast<int>(DataToSlice.size());

  if (Start < 0 || End < 0 || End >= dataSize || Start > End) {
    return T{};
  }
  auto itStart = DataToSlice.begin() + Start;
  auto itEnd = DataToSlice.begin() + End + 1;
  return T(itStart, itEnd);
}
template <typename T, typename U>
bool FindElement(const T &ToSearch, const U &DataToSearchFrom) {
  return std::all_of(
      DataToSearchFrom.begin(), DataToSearchFrom.end(),
      [&ToSearch](const auto &element) { return element != ToSearch; });
}
