# 🌟 BharatLang - Hinglish Programming Language Compiler

![C++20](https://img.shields.io/badge/C++-20-blue.svg)
![OOP](https://img.shields.io/badge/Architecture-OOP%20&%20Design%20Patterns-success.svg)
![Build](https://img.shields.io/badge/Build-CMake-orange.svg)

BharatLang is a complete **Tree-Walking Interpreter and Compiler Front-End** built from scratch in C++20. It implements a fully functioning programming language with an intuitive Hinglish (Hindi + English) syntax. 

This project demonstrates advanced concepts in **System Software Design**, **Compiler Construction**, and **Object-Oriented Programming (OOP)**.

## 🚀 Features

*   **Complete Compilation Pipeline:** Includes Lexical Analysis, Parsing, Semantic Analysis, IR Generation, and Interpreted Execution.
*   **Hinglish Syntax:** Keywords like `agar`, `warna`, `jabtak`, `likho`, based on conversational Hindi.
*   **Dynamic Typing:** Supports variables with types `ank` (integer), `dasham` (float), `shabd` (string), and `haan_naa` (boolean).
*   **Functions & Closures:** First-class functions, recursion, and lexical scoping (closures).
*   **Control Flow:** Fully implements `agar/warna` (if/else), `jabtak` (while), `har` (for), `ruko` (break), and `agla` (continue).
*   **Intermediate Representation (IR):** Capable of lowering AST to Three-Address Code (TAC) for compiler optimization demonstrations.
*   **Interactive REPL:** Built-in shell for interactive programming.
*   **Robust Error Handling:** Custom Hinglish error messages with exact line and column tracking.

---

## 🛠️ Technology Stack & OOP Concepts Used

| Concept | Where it's used in this project |
| :--- | :--- |
| **Encapsulation** | State management inside `Lexer` and `Parser` classes; hiding evaluation context in `Environment`. |
| **Inheritance** | Base classes `Expression` and `Statement` forming the Abstract Syntax Tree (AST); custom exception hierarchies. |
| **Polymorphism** | `BharatCallable` interface implemented by both user-defined functions (`BharatFunction`) and native C++ functions (`NativeFunction`). |
| **Visitor Design Pattern** | Used in `Interpreter` and `IRGenerator` (via `ExprVisitor` and `StmtVisitor`) to traverse the AST structure without polluting the node classes themselves with execution logic. |
| **Factory Pattern** | Used extensively with `std::make_unique` to generate AST nodes dynamically. |
| **Memory Management** | Heavy use of C++11/14/17 smart pointers (`std::unique_ptr` for AST ownership, `std::shared_ptr` for Environment scope chains / closures). |

---

## 💻 Language Syntax Guide

### Data Types
*   **`ank`** : Integer (e.g., `42`)
*   **`dasham`** : Float (e.g., `3.14`)
*   **`shabd`** : String (e.g., `"Namaste"`)
*   **`haan_naa`** : Boolean (`sahi` = true, `galat` = false)
*   **`shunya`** : Null/Void

### Basic Syntax
```javascript
// Variable declaration
rakho naam = "Rahul";
rakho umar = 21;

// Printing output
likho("Mera naam " + naam + " hai.");

// User Input
rakho input = padho("Apna input do: ");
```

### Control Flow
```javascript
// If - Else
agar (umar >= 18) {
    likho("Aap vote de sakte hain!");
} warna_agar (umar >= 13) {
    likho("Aap teenager hain.");
} warna {
    likho("Aap bachche hain.");
}

// While Loop
rakho i = 0;
jabtak (i < 5) {
    likho(i);
    i = i + 1;
}

// For Loop
har (rakho j = 0; j < 5; j = j + 1) {
    agar (j == 3) ruko; // break statement
    likho(j);
}
```

### Functions & Recursion
```javascript
kaam factorial(n) {
    agar (n <= 1) lautao 1;
    lautao n * factorial(n - 1);
}

likho("5 ka factorial: " + factorial(5));
```

---

## ⚙️ Building the Compiler

You will need a C++20 compatible compiler (like GCC 10+ via MSYS2 or MSVC 2019+) and CMake.

### 1. Using CMake
```bash
cmake -B build -S .
cmake --build build
```
*(Alternative: If on MSYS2 Mingw64, you can directly compile using g++:)*
```bash
g++ -std=c++20 -Wall -Isrc -o build/bharatlang.exe src/main.cpp src/Lexer.cpp src/Parser.cpp src/Interpreter.cpp src/IRGenerator.cpp
```

### 2. Running
**Mode 1: Interactive Shell (REPL)**
```bash
./build/bharatlang.exe
भारत>>> 
```

**Mode 2: Running a Script**
```bash
./build/bharatlang.exe examples/namaste.bl
```

**Mode 3: Debug Tools**
```bash
./build/bharatlang.exe examples/namaste.bl --tokens   # View raw tokens from Lexer
./build/bharatlang.exe examples/namaste.bl --ir       # View generated Three-Address Code
```

---

## 📂 File Structure Overview

*   **`src/Lexer`**: Reads raw text, removes whitespace/comments, and outputs a stream of `Token`s.
*   **`src/Parser`**: Implements a top-down Recursive Descent Parser to convert Tokens into an Abstract Syntax Tree (AST).
*   **`src/AST & Value`**: The data structures (nodes & dynamically typed variants) that represent the program.
*   **`src/Environment`**: Lexical scope management (variables mapped to values), connected in a chain (linked list) to represent closures/blocks.
*   **`src/Interpreter`**: Executes the AST tree directly by traversing it and performing C++ operations.
*   **`src/IRGenerator`**: A compiler analysis phase generating 3-Address Code.

## 🤝 Next Steps / Future Scope
*   Adding arrays (`suchi`) and dictionaries (`kosh`).
*   Object-Oriented syntax within BharatLang (Classes / Inheritance).
*   Writing a Bytecode Virtual Machine (VM) backend to replace the Tree-Walking Interpreter for 10x performance.
