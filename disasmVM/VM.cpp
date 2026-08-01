#include "VM.h"
#include "../main/types.h"
#include "HandleCommands/Commands.h"
#include <iostream>
#include <vector>

void InterpretByteCode() {
  BCP = 0;
  TokenTypes EnumRepr;
  if (ByteCode.size() == 0)
    return;
  ByteCodeDT BCR = ByteCode.at(0);
  while (BCR.TypeRepr != TokenTypes::ENDCODE) {
    EnumRepr = BCR.TypeRepr;
    switch (EnumRepr) {
    case TokenTypes::out:
      outCommand();
      break;
    case TokenTypes::set:
      setCommand();
      break;
    case TokenTypes::mlc:
      mlcCommand();
      break;
    case TokenTypes::gotoln:
      Handlegotoln();
      break;
    default:
      // std::cout << "NOT READY";
      break;
    }
    BCP++;
    if (BCP > ByteCode.size())
      return;
    BCR = ByteCode.at(BCP);
  }
  std::cout << '\n';
}
