cmake_minimum_required(VERSION 3.15)
project(Assmic VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    main/main.cpp
    main/ImportantInternalFunctions.cpp

    errorhandling/ErrorHandler.cpp

    lexer/BuildIR.cpp
    lexer/Declarators.cpp
    lexer/Tokenizer.cpp

    IRinterpreter/VM.cpp
    IRinterpreter/ExecuteCommands.cpp
)

add_executable(Assmic ${SOURCES})

target_include_directories(Assmic PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/main
    ${CMAKE_CURRENT_SOURCE_DIR}/errorhandling
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer
    ${CMAKE_CURRENT_SOURCE_DIR}/IRinterpreter
)
