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
  BoolExpr,
  BoolExprEnd,
  ArrEnd,
  Module,
  ENDCODE,
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
  LessThan,
  GreaterThan,
  Equal,
  LessEqual,
  GreaterEqual,
  NotEqual,
  // mid-line commands
  var,
  mem,
  clc,
  evl,
  // commands
  out,
  inp,
  dec,
  set,
  mlc,
  gotoln,
  rpt, // loop
  end,
  cmp,
  elf,
  ele,
  gto,
  // symbols
  Colon,   //":"
  Stopper, // "]"
  Parenthesis,
  Period,
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
)";

inline std::vector<ByteCodeDT> ByteCode;

struct RawDataRepr {
  int VariableID; // Variable that uses this address. -1 means no variable uses
                  // it
  std::string Data;
  TokenTypes DataType;
};
struct ModuleDT {
  int ModuleID;
  int ByteCodeStart;
};
inline int CurrentModuleID = 0;
inline int VarCount = 0;
inline std::string ErrorInstance = "CT";
// CT -> Compile Time
// IT -> Interpreting
// when looking for identifiers inside a module, it first checks for local
// sand box memory and then the global
inline int BCP = -1;
inline bool global = false;
inline int TotalMemSize;
inline std::stack<int> g_TotalMemPool;
inline std::unordered_map<int, VariableDT> g_VariableTable; // global
inline std::unordered_map<int, VariableDT> l_VariableTable; // local
inline std::unordered_map<int, VariableDT> *c_VariableTable = &g_VariableTable;
inline std::unordered_map<int, ModuleDT> ModuleTable;
inline std::vector<RawDataRepr> SBMemory; // Where the actual data lies
inline std::unordered_map<std::string, int> g_MapVariableNameAndID;
inline std::unordered_map<std::string, int> l_MapVariableNameAndID;
inline std::unordered_map<std::string, int> *c_MapVariableNameAndID =
    &g_MapVariableNameAndID;
inline std::unordered_map<std::string, int> MapModuleNameAndID;
inline std::unordered_map<int, std::unordered_map<int, VariableDT>>
    LocalModuleVariableTable;
inline std::stack<int> InterpreterModuleStack;
inline std::stack<int> TrackModuleDecLine;
inline int CurrentVariableID = 0;
