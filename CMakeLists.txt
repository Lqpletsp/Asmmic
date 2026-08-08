cmake_minimum_required(VERSION 3.15)
project(DisasmImprov2 VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    # main folder
    main/main.cpp
    main/ImportantInternalFunctions.cpp

    # errorhandling folder
    errorhandling/ErrorHandler.cpp

    # lexer folder
    lexer/ByteCodeGenerate.cpp
    lexer/Declarators.cpp
    lexer/Tokenizer.cpp

    # disasmVM folder
    IRinterpreter/VM.cpp
    IRinterpreter/HandleCommands/LineCommands.cpp
    IRinterpreter/HandleCommands/MidLineCommands.cpp
)

add_executable(DisasmImprov2 ${SOURCES})

target_include_directories(DisasmImprov2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main
    ${CMAKE_CURRENT_SOURCE_DIR}/errorhandling
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/IRinterpreter
    ${CMAKE_CURRENT_SOURCE_DIR}/IRinterpreter/HandleCommands
)
