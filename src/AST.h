/**
 * ============================================================
 *  AST.h — BharatLang Abstract Syntax Tree
 * ============================================================
 *  Defines every node in the AST using an OOP inheritance
 *  hierarchy with the Visitor design pattern.
 *
 *  Hierarchy:
 *    ASTNode (abstract base)
 *      ├── Expression (abstract)
 *      │     ├── BinaryExpr      (a + b, a == b, etc.)
 *      │     ├── UnaryExpr       (-x, nahi x)
 *      │     ├── LiteralExpr     (42, "hello", sahi)
 *      │     ├── VariableExpr    (variable reference)
 *      │     ├── AssignExpr      (x = 5)
 *      │     ├── LogicalExpr     (a aur b, a ya b)
 *      │     ├── CallExpr        (factorial(5))
 *      │     ├── GroupingExpr    ((a + b))
 *      │     └── InputExpr       (padho("prompt"))
 *      └── Statement (abstract)
 *            ├── ExpressionStmt  (standalone expression)
 *            ├── PrintStmt       (likho(...))
 *            ├── VarDeclStmt     (rakho x = 5)
 *            ├── BlockStmt       ({ ... })
 *            ├── IfStmt          (agar/warna_agar/warna)
 *            ├── WhileStmt       (jabtak)
 *            ├── ForStmt         (har)
 *            ├── FunctionStmt    (kaam name() { })
 *            ├── ReturnStmt      (lautao)
 *            ├── BreakStmt       (ruko)
 *            └── ContinueStmt    (agla)
 *
 *  OOP Concepts Demonstrated:
 *    - Inheritance & Polymorphism (virtual accept)
 *    - Visitor Design Pattern (double dispatch)
 *    - Smart Pointers (unique_ptr for ownership)
 *    - Abstract Classes (pure virtual methods)
 * ============================================================
 */
#pragma once

#include "Token.h"
#include "Value.h"
#include <memory>
#include <vector>
#include <string>

// ═══════════════════════════════════════════════════════════
//  FORWARD DECLARATIONS — needed for Visitor interfaces
// ═══════════════════════════════════════════════════════════
struct BinaryExpr;
struct UnaryExpr;
struct LiteralExpr;
struct VariableExpr;
struct AssignExpr;
struct LogicalExpr;
struct CallExpr;
struct GroupingExpr;
struct InputExpr;

struct ExpressionStmt;
struct PrintStmt;
struct VarDeclStmt;
struct BlockStmt;
struct IfStmt;
struct WhileStmt;
struct ForStmt;
struct FunctionStmt;
struct ReturnStmt;
struct BreakStmt;
struct ContinueStmt;

// ═══════════════════════════════════════════════════════════
//  VISITOR INTERFACES
// ═══════════════════════════════════════════════════════════

/**
 * ExprVisitor — visits expression nodes, returns a Value.
 * Implements the Visitor design pattern for expressions.
 */
class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual Value visit(BinaryExpr& expr)   = 0;
    virtual Value visit(UnaryExpr& expr)    = 0;
    virtual Value visit(LiteralExpr& expr)  = 0;
    virtual Value visit(VariableExpr& expr) = 0;
    virtual Value visit(AssignExpr& expr)   = 0;
    virtual Value visit(LogicalExpr& expr)  = 0;
    virtual Value visit(CallExpr& expr)     = 0;
    virtual Value visit(GroupingExpr& expr) = 0;
    virtual Value visit(InputExpr& expr)    = 0;
};

/**
 * StmtVisitor — visits statement nodes, returns void.
 * Statements produce side effects, not values.
 */
class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visit(ExpressionStmt& stmt) = 0;
    virtual void visit(PrintStmt& stmt)      = 0;
    virtual void visit(VarDeclStmt& stmt)    = 0;
    virtual void visit(BlockStmt& stmt)      = 0;
    virtual void visit(IfStmt& stmt)         = 0;
    virtual void visit(WhileStmt& stmt)      = 0;
    virtual void visit(ForStmt& stmt)        = 0;
    virtual void visit(FunctionStmt& stmt)   = 0;
    virtual void visit(ReturnStmt& stmt)     = 0;
    virtual void visit(BreakStmt& stmt)      = 0;
    virtual void visit(ContinueStmt& stmt)   = 0;
};

// ═══════════════════════════════════════════════════════════
//  BASE CLASSES
// ═══════════════════════════════════════════════════════════

/**
 * Expression — abstract base for all expression AST nodes.
 * Every expression can produce a Value when evaluated.
 */
struct Expression {
    virtual ~Expression() = default;
    virtual Value accept(ExprVisitor& visitor) = 0;
};

/**
 * Statement — abstract base for all statement AST nodes.
 * Statements are executed for their side effects.
 */
struct Statement {
    virtual ~Statement() = default;
    virtual void accept(StmtVisitor& visitor) = 0;
};

// Type aliases for readability
using ExprPtr = std::unique_ptr<Expression>;
using StmtPtr = std::unique_ptr<Statement>;

// ═══════════════════════════════════════════════════════════
//  EXPRESSION NODES
// ═══════════════════════════════════════════════════════════

/**
 * BinaryExpr — two operands with an operator in between.
 * Examples: a + b, x == y, marks > 90
 */
struct BinaryExpr : Expression {
    ExprPtr left;
    Token   op;
    ExprPtr right;

    BinaryExpr(ExprPtr left, Token op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)),
          right(std::move(right)) {}

    Value accept(ExprVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

/**
 * UnaryExpr — single operand with a prefix operator.
 * Examples: -x, nahi sahi
 */
struct UnaryExpr : Expression {
    Token   op;
    ExprPtr operand;

    UnaryExpr(Token op, ExprPtr operand)
        : op(std::move(op)), operand(std::move(operand)) {}

    Value accept(ExprVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

/**
 * LiteralExpr — a literal value embedded in the source.
 * Examples: 42, 3.14, "namaste", sahi, galat, shunya
 */
struct LiteralExpr : Expression {
    Value value;

    explicit LiteralExpr(Value value) : value(std::move(value)) {}

    Value accept(ExprVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

/**
 * VariableExpr — a reference to a variable by name.
 * Example: x, naam, umar
 */
struct VariableExpr : Expression {
    Token name;

    explicit VariableExpr(Token name) : name(std::move(name)) {}

    Value accept(ExprVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

/**
 * AssignExpr — variable assignment expression.
 * Example: x = 10
 */
struct AssignExpr : Expression {
    Token   name;
    ExprPtr value;

    AssignExpr(Token name, ExprPtr value)
        : name(std::move(name)), value(std::move(value)) {}

    Value accept(ExprVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

/**
 * LogicalExpr — short-circuit logical operators.
 * Example: a aur b, x ya y
 */
struct LogicalExpr : Expression {
    ExprPtr left;
    Token   op;       // AUR or YA
    ExprPtr right;

    LogicalExpr(ExprPtr left, Token op, ExprPtr right)
        : left(std::move(left)), op(std::move(op)),
          right(std::move(right)) {}

    Value accept(ExprVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

/**
 * CallExpr — function call expression.
 * Example: factorial(5), add(a, b)
 */
struct CallExpr : Expression {
    ExprPtr              callee;    // the function being called
    Token                paren;     // closing ')' for error location
    std::vector<ExprPtr> arguments;

    CallExpr(ExprPtr callee, Token paren, std::vector<ExprPtr> arguments)
        : callee(std::move(callee)), paren(std::move(paren)),
          arguments(std::move(arguments)) {}

    Value accept(ExprVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

/**
 * GroupingExpr — parenthesized expression for precedence.
 * Example: (a + b) * c
 */
struct GroupingExpr : Expression {
    ExprPtr expression;

    explicit GroupingExpr(ExprPtr expression)
        : expression(std::move(expression)) {}

    Value accept(ExprVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

/**
 * InputExpr — padho() input expression.
 * Example: padho("Apna naam batao: ")
 * Reads a line from stdin, optionally showing a prompt.
 */
struct InputExpr : Expression {
    ExprPtr prompt;     // optional prompt (can be nullptr)

    explicit InputExpr(ExprPtr prompt)
        : prompt(std::move(prompt)) {}

    Value accept(ExprVisitor& visitor) override {
        return visitor.visit(*this);
    }
};

// ═══════════════════════════════════════════════════════════
//  STATEMENT NODES
// ═══════════════════════════════════════════════════════════

/**
 * ExpressionStmt — an expression used as a statement.
 * Example: factorial(5);
 */
struct ExpressionStmt : Statement {
    ExprPtr expression;

    explicit ExpressionStmt(ExprPtr expression)
        : expression(std::move(expression)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * PrintStmt — likho() print statement.
 * Example: likho("Namaste", naam, umar);
 * Prints all expressions separated by spaces.
 */
struct PrintStmt : Statement {
    std::vector<ExprPtr> expressions;

    explicit PrintStmt(std::vector<ExprPtr> expressions)
        : expressions(std::move(expressions)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * VarDeclStmt — rakho variable declaration.
 * Example: rakho x = 42;  or  rakho naam;
 */
struct VarDeclStmt : Statement {
    Token   name;
    ExprPtr initializer;    // can be nullptr (uninitialized)

    VarDeclStmt(Token name, ExprPtr initializer)
        : name(std::move(name)), initializer(std::move(initializer)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * BlockStmt — a block of statements enclosed in { }.
 * Creates a new scope for variables.
 */
struct BlockStmt : Statement {
    std::vector<StmtPtr> statements;

    explicit BlockStmt(std::vector<StmtPtr> statements)
        : statements(std::move(statements)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * IfStmt — agar / warna_agar / warna conditional.
 * Example:
 *   agar (x > 0) { ... }
 *   warna_agar (x == 0) { ... }
 *   warna { ... }
 *
 * elifConditions[i] pairs with elifBranches[i].
 */
struct IfStmt : Statement {
    ExprPtr              condition;
    StmtPtr              thenBranch;
    std::vector<ExprPtr> elifConditions;     // warna_agar conditions
    std::vector<StmtPtr> elifBranches;       // warna_agar bodies
    StmtPtr              elseBranch;         // warna body (nullable)

    IfStmt(ExprPtr condition, StmtPtr thenBranch,
           std::vector<ExprPtr> elifConditions,
           std::vector<StmtPtr> elifBranches,
           StmtPtr elseBranch)
        : condition(std::move(condition)),
          thenBranch(std::move(thenBranch)),
          elifConditions(std::move(elifConditions)),
          elifBranches(std::move(elifBranches)),
          elseBranch(std::move(elseBranch)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * WhileStmt — jabtak loop.
 * Example: jabtak (x < 10) { ... }
 */
struct WhileStmt : Statement {
    ExprPtr condition;
    StmtPtr body;

    WhileStmt(ExprPtr condition, StmtPtr body)
        : condition(std::move(condition)), body(std::move(body)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * ForStmt — har loop (C-style for).
 * Example: har (rakho i = 0; i < 10; i = i + 1) { ... }
 */
struct ForStmt : Statement {
    StmtPtr initializer;    // loop variable declaration
    ExprPtr condition;      // loop condition
    ExprPtr increment;      // increment expression
    StmtPtr body;

    ForStmt(StmtPtr initializer, ExprPtr condition,
            ExprPtr increment, StmtPtr body)
        : initializer(std::move(initializer)),
          condition(std::move(condition)),
          increment(std::move(increment)),
          body(std::move(body)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * FunctionStmt — kaam function declaration.
 * Example: kaam factorial(n) { lautao n * factorial(n-1); }
 */
struct FunctionStmt : Statement {
    Token              name;
    std::vector<Token> params;
    std::vector<StmtPtr> body;

    FunctionStmt(Token name, std::vector<Token> params,
                 std::vector<StmtPtr> body)
        : name(std::move(name)), params(std::move(params)),
          body(std::move(body)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * ReturnStmt — lautao return statement.
 * Example: lautao x * 2;
 */
struct ReturnStmt : Statement {
    Token   keyword;
    ExprPtr value;      // can be nullptr (void return)

    ReturnStmt(Token keyword, ExprPtr value)
        : keyword(std::move(keyword)), value(std::move(value)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * BreakStmt — ruko break statement.
 */
struct BreakStmt : Statement {
    Token keyword;

    explicit BreakStmt(Token keyword) : keyword(std::move(keyword)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};

/**
 * ContinueStmt — agla continue statement.
 */
struct ContinueStmt : Statement {
    Token keyword;

    explicit ContinueStmt(Token keyword) : keyword(std::move(keyword)) {}

    void accept(StmtVisitor& visitor) override {
        visitor.visit(*this);
    }
};
