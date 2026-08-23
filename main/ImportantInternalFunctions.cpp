#include "types.h"
#include <algorithm>
#include <sstream>
#include <string>

int SMalloc() { // SMalloc -> Sandbox Memory allocation
  if (g_TotalMemPool.empty())
    return -1;
  int topMemory = g_TotalMemPool.top();
  g_TotalMemPool.pop();
  return topMemory;
}

bool ValidateName(const std::string &Name) {
  return std::all_of(Name.begin(), Name.end(), [](unsigned char ch) {
    return std::isalnum(ch) || ch == '_';
  });
}

TokenTypes DetermineDataType(const std::string &Token) {
  if (Token.empty())
    return TokenTypes::Empty;
  if ((Token.front() == '"' && Token.back() == '"') ||
      (Token.front() == '\'' && Token.back() == '\'')) {
    if (Token.size() == 3)
      return TokenTypes::CharVal; // char (e.g., 'a')
    return TokenTypes::StringVal; // string (e.g., "abc")
  }
  try {
    std::stringstream ss(Token);
    double num;
    // Checks if the entire token can be successfully parsed into a number
    if ((ss >> num) && ss.eof()) {
      if (Token.find('.') != std::string::npos)
        return TokenTypes::DoubleVal; // double/float
      return TokenTypes::IntVal;      // int
    }
  } catch (...) {
  }

  if (Token.size() == 1 && (Token == "T" || Token == "F"))
    return TokenTypes::BoolVal; // boolean
  if (ValidateName(Token))
    return TokenTypes::Identifier; // identifier
  return TokenTypes::Unknown;
}

bool CheckIfValidGlobalVariable(
    const int &VariableID) { // no functions so no local variables right now
  return (g_VariableTable.find(VariableID) == g_VariableTable.end()) ? false
                                                                     : true;
}

ModuleDT GetModuleMetaData(const int &ModuleID) {
  return ModuleTable[ModuleID];
}
VariableDT *GetVariableMetaData(const int &VariableID) {
  return &(*c_VariableTable)[VariableID];
}

std::string GetDataFromAddress(const std::string &StrAddress) {
  return SBMemory.at(stoi(StrAddress)).Data;
}

void StoreDataInAddress(const std::string &StrAddress,
                        const std::string &DataToStore) {
  SBMemory.at(std::stoi(StrAddress)).Data = DataToStore;
}

RawDataRepr *GetMemorySpaceMetaData(const std::string &StrAddress) {
  return &SBMemory.at(std::stoi(StrAddress));
}

int GetVariableID() {
  CurrentVariableID++;
  return (CurrentVariableID - 1);
}

int GetModuleID() {
  CurrentModuleID++;
  return (CurrentModuleID - 1);
}

int GetAssignedVariableID(const std::string &VariableName) {
  auto it = (*c_MapVariableNameAndID).find(VariableName);
  if (it == (*c_MapVariableNameAndID).end()) {
    return -1; // Or whatever sentinel value you use for "not found"
  }
  return it->second; // Return the actual ID stored in the map
}

int GetAssignedModuleID(const std::string &ModuleName) {
  auto it = MapModuleNameAndID.find(ModuleName);
  if (it == MapModuleNameAndID.end()) {
    return -1;
  }
  return it->second;
}
