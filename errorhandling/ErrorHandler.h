#pragma once
#include "../main/types.h"
#include <cerrno>
#include <string>
#include <unordered_map>

enum class ErrorTypes {
  CommandNotFound,
  IdentifierNotFound,
  MemoryNotDeclared,
  MemoryFull,
  GarbageToken,
  NoCommandInFrontofLine,
  InvalidFlag,
  GarbageArgInACommand,
  NotEnoughArgGivenForCommand,
  ZeroOrNegativeMemorySapces,
  InvalidDataTypeInVariable,
  AttemptedToStoreInsideConstants,
  AttemptedToStoreInUnSupportedVariable,
  InvalidTypeForNumberOfAllocations,
  ZeroOrNegativeMemoryAllocation,
  StreamVarEmpty,
  CannotTransformStaticVaribles,
  AddressOperatorGivenButNoAddress,
  NoIntArrayAddress,
  EmptyArrayPointer,
  OutOfBounds,
  CannotOutputDataStructures,
  AttemptedMultiDimensionalArrays,
  IncompleteArgumentsForLineCommand,
  NoArrayIndexGiven,
  NonDigitDataForclc
};

inline std::unordered_map<ErrorTypes, std::string> CorrespondingErrorStrings = {
    {ErrorTypes::CommandNotFound,
     "is not a user-made command, nor a primitive command"}, // format: <invalid
                                                             // command> string
    {ErrorTypes::IdentifierNotFound,
     "was not declared"}, // format: <Identifier name> string
    {ErrorTypes::MemoryNotDeclared,
     "Memory was not declared"}, // format: string
    {ErrorTypes::MemoryFull,
     "Memory full"}, // format: string <total memory used>
    {ErrorTypes::GarbageToken, "garbage token given"},
    {ErrorTypes::NoCommandInFrontofLine,
     "Every line must begin with a command but was not found"},
    {ErrorTypes::InvalidFlag, "is not a valid flag for the line command"},
    {ErrorTypes::GarbageArgInACommand, "is not supported by the line command"},
    {ErrorTypes::ZeroOrNegativeMemorySapces,
     "Cannot declare memory space that is <= 0"},
    {ErrorTypes::InvalidDataTypeInVariable,
     "identifier does not support the attempted data type"},
    {ErrorTypes::AttemptedToStoreInsideConstants,
     "constants cannot store other data, but was attempted to"},
    {ErrorTypes::AttemptedToStoreInUnSupportedVariable,
     "The target identifier was an array, the data given was not"},
    {ErrorTypes::InvalidTypeForNumberOfAllocations,
     "Allocations require int value data type but was not given"},
    {ErrorTypes::ZeroOrNegativeMemorySapces,
     "Cannot allocate memory space to an identifier that <= 0"},
    {ErrorTypes::StreamVarEmpty,
     "Attempted to clear the stream, but was already empty"},
    {ErrorTypes::CannotTransformStaticVaribles,
     "Attemped to allocate memory in a static variable"},
    {ErrorTypes::AddressOperatorGivenButNoAddress,
     "Adddress operator given but the line ended without memory address"},
    {ErrorTypes::NoIntArrayAddress, "was given. Array addresses as identifiers "
                                    "must be of type int but was not given"},
    {ErrorTypes::EmptyArrayPointer,
     "The identifier stores nil value but was used as an address"},
    {ErrorTypes::OutOfBounds, "The address exceeds the length of "
                              "the array"},
    {ErrorTypes::CannotOutputDataStructures,
     "Cannot load a data structure inside a stream"},
    {ErrorTypes::AttemptedMultiDimensionalArrays,
     "Tried forming an unsupported data structure"},
    {ErrorTypes::IncompleteArgumentsForLineCommand,
     "Incomplete amount of arguments given. The line command was unable to "
     "interpret"},
    {ErrorTypes::NoArrayIndexGiven,
     "Attemped to store data as a data structure"},
    {ErrorTypes::NonDigitDataForclc, "Attempted non digit data for clc"},
};
void ShowError(const TokenDT &Token, const ErrorTypes &Type);
void ShowError(const ByteCodeDT &Token, const ErrorTypes &Type);
