#include <iostream>
#include <string>
#include <unordered_map>

enum TokenTypes {
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

std::unordered_map<TokenTypes, std::string> Map = {
    // constants
    {NewLine, "NewLine"},
    {TrueVal, "TrueVal"},
    {FalseVal, "FalseVal"},
    {Identifier, "Identifier"},
    {Empty, "Empty"},
    {Unknown, "Unknown"},
    {MemoryAddressIndicator, "MemoryAddressIndicator"},
    {ArrayHint, "ArrayHint"},
    {VariableID, "VariableID"},
    {MemoryAddress, "MemoryAddress"},
    {MathExpr, "MathExpr"},
    {MathExprEnd, "MathExprEnd"},
    {ArrEnd, "ArrEnd"},
    {End, "End"},

    // data types
    {DoubleVal, "DoubleVal"},
    {IntVal, "IntVal"},
    {BoolVal, "BoolVal"},
    {CharVal, "CharVal"},
    {StringVal, "StringVal"},
    {Flag, "Flag"},

    // operators
    {Add, "Add"},
    {Min, "Min"},
    {Div, "Div"},
    {Mlt, "Mlt"},
    {Exp, "Exp"},

    // mid-line commands
    {var, "var"},
    {mem, "mem"},
    {clc, "clc"},

    // commands
    {out, "out"},
    {inp, "inp"},
    {dec, "dec"},
    {set, "set"},
    {mlc, "mlc"},

    // symbols
    {Colon, "Colon"},
    {Stopper, "Stopper"}};

void PrintBC(std::string ByteCode) {
  std::string BC;
  for (const char &ch : ByteCode) {
    if (ch == ' ') {
      std::cout << Map[static_cast<TokenTypes>(std::stoi(BC))] << std::endl;
      BC = "";
    } else
      BC += ch;
  }
  std::cout << Map[static_cast<TokenTypes>(std::stoi(BC))] << std::endl;
}
int main() {
  std::string ByteCode;
  std::getline(std::cin, ByteCode);
  PrintBC(ByteCode);
  return 0;
}
