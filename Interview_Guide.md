# 🎯 Interview Guide: BharatLang Compiler Project

This document is extremely important for your placements/interviews. Since you built a full compiler from scratch using C++, interviewers will be very impressed, but they **will** ask deep technical questions.

Here is exactly how you can explain the project in an interview, step-by-step, in very simple terms.

---

## 1. 🌟 The "Elevator Pitch" (Introduction)
**What to say:**
> *"For my mini-project, I wanted to apply my C++ and OOP skills to a complex systems-level problem. So, I built 'BharatLang', a fully operational programming language and interpreter from scratch. It has a custom Hinglish syntax. I implemented the entire compilation pipeline myself: a lexical analyzer, a recursive descent parser generating an Abstract Syntax Tree (AST), environment-based scope management, and the tree-walking interpreter. I used advanced OOP concepts like Inheritance, Polymorphism, and the Visitor Design Pattern to manage the complexity."*

---

## 2. 🧩 Explaining The Core Phases of Your Compiler

If the interviewer asks: **"Walk me through how your compiler executes code?"**

Explain these 4 phases sequentially:

### Phase 1: Lexical Analysis (The Tokenizer)
*   **Simple term:** "Cutting the paragraph into words."
*   **What you did:** *"The Lexer takes the raw `.bl` file character-by-character and groups them into meaningful chunks called `Tokens`. For example, if it sees `rakho x = 5;`, it doesn't see a string, it generates 5 tokens: `[RAKHO keyword], [IDENTIFIER 'x'], [ASSIGN operator], [NUMBER 5], [SEMICOLON]`."*
*   **Key Detail:** *"I mapped Hindi words to my token types in a C++ `unordered_map`, e.g., mapping `"agar"` to `TokenType::AGAR`."*

### Phase 2: Syntax Analysis (The Parser)
*   **Simple term:** "Checking the grammar and making a tree."
*   **What you did:** *"It takes the flat list of tokens and builds a hierarchical tree called the Abstract Syntax Tree (AST). I used a technique called **Recursive Descent Parsing**. It recursively evaluates expressions checking operator precedence (e.g. `*` is applied before `+`)."*
*   **Key Detail:** *"If the user writes `5 + 3 * 2`, my parser ensures `*` is deeper in the tree so it gets evaluated first."*

### Phase 3: Environment & Scope (The Symbol Table)
*   **Simple term:** "Memory manager for tracking variables."
*   **What you did:** *"Every time a variable is created, its value is stored in an `Environment` class. To handle block scoping (e.g., variables dying at the end of an `if` block), I implemented a **Scope Chain**. Each Environment has a pointer (`std::shared_ptr`) to its parent environment."*
*   **Key Detail:** *"When I look up variable `x`, my code checks the current block. If it doesn't find it, it traverses up the pointer to the parent block, all the way to the global scope."*

### Phase 4: Execution (The Interpreter)
*   **Simple term:** "Running the tree."
*   **What you did:** *"I built a **Tree-Walking Interpreter**. It recursively visits every node in the AST and performs the actual C++ operations. For a `BinaryExpr` with a `PLUS` token, it takes the left child's value, the right child's value, and does C++ `+` on them."*


---

## 3. 🧠 How You Applied Object-Oriented Programming (OOP)

This is the most critical part. Interviewers **love** asking how you used OOP.

**1. Polymorphism and Inheritance (The AST)**
*   *"I used Inheritance extensively for the AST. There is an abstract base class `Expression`. Specific node types like `BinaryExpr`, `LiteralExpr`, `CallExpr` inherit from it. This allowed me to treat everything as an abstract `Expression` pointer (`std::unique_ptr<Expression>`)."*

**2. The Visitor Design Pattern (Crucial!)**
*   **Interviewer question:** "How did you execute the tree without putting massive `execute()` functions inside every node?"
*   **Your answer:** *"I used the **Visitor Design Pattern**. I created an `ExprVisitor` interface. The `Interpreter` class inherits from this visitor. Inside every AST node, there is a tiny virtual method `accept(Visitor& v) { v.visit(*this); }`. This achieves **double dispatch**. It perfectly separates the Data (the AST nodes) from the Operations acting on them (the Interpreter), avoiding the pollution of my data models."*

**3. Abstraction (The Value System)**
*   *"Because BharatLang is dynamically typed (like Python/JS), a variable can hold an int, float, string, or boolean. I encapsulated this using C++ `std::variant`. My universal `Value` type abstracts away the underlying C++ type management."*

---

## 4. 🚀 Advanced Capabilities to Brag About

If the interviewer asks: **"What was the most challenging part?"** or **"What makes this project special?"**

Bring up these features:

*   **Closures / First-Class Functions:** *"Functions in my language are first-class citizens. You can pass a function to another function. When a `kaam` (function) is created, it captures its current `Environment` scope as a closure. This means inner functions can remember variables from outer functions even after the outer function finishes executing."*
*   **Exception-Based Control Flow:** *"Implementing `rutao` (return) and `ruko/agla` (break/continue) in a recursive tree walker was tricky. I solved this by throwing custom C++ Exceptions. When a return is hit deep in the tree, I throw a custom `ReturnSignal` exception holding the return value, and the function executor node catches it and returns the value safely."*
*   **Intermediate Representation (IR):** *"To show that I understand backend compiler phases as well, I implemented an `IRGenerator` that walks the same AST as the Interpreter, but instead of executing it, it outputs actual **Three-Address Code (TAC)**, generating temp variables (`t0`, `t1`) and assembly-like `GOTO` labels."*
*   **Smart Pointers for memory Safety:** *"Tree data structures are notorious for memory leaks. I strictly used `std::unique_ptr` for tree ownership to ensure RAII, and `std::shared_ptr` for my scope chain closures because of shared lifecycle requirements. So my compiler has 0 memory leaks without needing a Garbage Collector."*

---

## 5. Potential Follow-Up Questions & How to Answer Them

**Q: C++ is statically typed, but your language is dynamically typed. How did you do that?**
> *"I used C++17's `std::variant` to create a `Value` type that acts as a discriminated union. Then I used the `std::visit` and the `overloaded` struct pattern to safely type-check and evaluate logic at runtime based on what the variant is currently holding."*

**Q: How does your parser handle infinite recursion if it expects an expression inside an expression?**
> *"It doesn't go infinite because my Recursive Descent Parser follows a strictly decaying precedence hierarchy. `expression()` calls `assignment()`, which calls `logicOr()`, all the way down to `primary()`. Every recursive call consumes tokens and goes down a level, eventually hitting a literal or parenthesis."*

**Q: What would you do to make it faster?**
> *"A tree-walking interpreter is fundamentally slow because of the high overhead of virtual function calls (`accept()` -> `visit()`) and cache-misses from fragmented AST heap allocations. To make it 10x-100x faster, I would write a **Bytecode Compiler** that translates the AST into a flat array of bytecode instructions, and execute them in a stack-based **Virtual Machine (VM)** loop."*
