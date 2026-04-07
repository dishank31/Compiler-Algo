cmake_minimum_required(VERSION 3.20)

project(BharatLang
    VERSION 1.0.0
    DESCRIPTION "Hinglish Programming Language Compiler"
    LANGUAGES CXX
)

# ── C++20 Standard ──────────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Source Files ────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/Interpreter.cpp
    src/IRGenerator.cpp
)

# ── Headers (for IDE awareness) ─────────────────────────────
set(HEADERS
    src/Token.h
    src/Value.h
    src/AST.h
    src/Lexer.h
    src/Parser.h
    src/Environment.h
    src/Interpreter.h
    src/IRGenerator.h
    src/ErrorReporter.h
)

# ── Main Executable ─────────────────────────────────────────
add_executable(bharatlang ${SOURCES} ${HEADERS})

# ── Include directory ────────────────────────────────────────
target_include_directories(bharatlang PRIVATE src)

# ── Compiler warnings ───────────────────────────────────────
if(MSVC)
    target_compile_options(bharatlang PRIVATE /W4)
else()
    target_compile_options(bharatlang PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Output name ──────────────────────────────────────────────
set_target_properties(bharatlang PROPERTIES
    OUTPUT_NAME "bharatlang"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)