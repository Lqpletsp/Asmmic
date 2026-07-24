#pragma once
#include <deque>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

struct TokenDT {
  std::string LiteralToken;
  int LineNum;
  int ColNum;
};

using TokenizedLineDT = std::vector<TokenDT>;
using TokenizedCodeDT = std::vector<TokenizedLineDT>;
enum Mode {
  Read,
  Write,
};
enum class TokenTypes {
  // constants
  NewLine,
  TrueVal,
  FalseVal,
  Identifier,
  Empty,
  Unknown,
  MemoryAddressIndicator,
  ArrayHint,
  VariableID,
  MemoryAddress,
  MathExpr,
  MathExprEnd,
  ArrEnd,
  End,
  // data types
  DoubleVal,
  IntVal,
  BoolVal,
  CharVal,
  StringVal,
  Flag,
  // operators
  Add,
  Min,
  Div,
  Mlt,
  Exp,
  And,
  Or,
  Not,
  Xor,
  // mid-line commands
  var,
  mem,
  clc,
  // commands
  out,
  inp,
  dec,
  set,
  mlc,
  // symbols
  Colon,   //":"
  Stopper, // "]"
};

struct ByteCodeDT {
  std::string LiteralToken;
  int LineNum;
  int ColNum;
  TokenTypes TypeRepr;
};
struct VariableDT {
  TokenTypes DataType;
  std::deque<int> MemorySlotsAssigned;
  bool Array;
};
// THIS IS THE MAIN CODE!!!
inline std::string MAINCODE = R"(
  out .clc 10 + 10*20; 
 )";
inline std::vector<ByteCodeDT> ByteCode;

struct RawDataRepr {
  int VariableID; // Variable that uses this address. -1 means no variable uses
                  // it
  std::string Data;
  TokenTypes DataType;
};
inline int VarCount = 0;
// when looking for identifiers inside a module, it first checks for local sand
// box memory and then the global
inline bool global = true; // if inside module, global = false
inline int TotalMemSize;
inline std::stack<int> g_TotalMemPool;
inline std::unordered_map<int, VariableDT> g_VariableTable; // global
inline std::unordered_map<int, VariableDT> l_VariableTable; // local
inline std::vector<RawDataRepr> SBMemory; // Where the actual data lies
inline std::unordered_map<std::string, int> MapVariableNameAndID;
inline int CurrentVariableID = 0;
