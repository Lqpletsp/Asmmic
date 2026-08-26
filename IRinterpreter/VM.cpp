#include "VM.h"
#include "../errorhandling/ErrorHandler.h"
#include "../main/types.h"
#include "ExecuteCommands.h"
#include <iostream>
#include <vector>

void InterpretByteCode() {
  BCP = 0;
  TokenTypes EnumRepr;
  if (ByteCode.size() == 0)
    return;
  ByteCodeDT BCR = ByteCode.at(0);
  while (BCR.TypeRepr != TokenTypes::ENDCODE && BCP < ByteCode.size()) {
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
    case TokenTypes::NewLine:
      break;
    default:
      std::cout << static_cast<int>(EnumRepr);
      ShowError(BCR, ErrorTypes::GarbageArgInACommand);
      break;
    }
    BCP++;
    if (BCP > ByteCode.size())
      return;
    BCR = ByteCode.at(BCP);
  }
  std::cout << '\n';
}
