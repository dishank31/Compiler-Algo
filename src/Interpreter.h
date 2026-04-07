/**
 * ============================================================
 *  Interpreter.h — BharatLang Tree-Walking Interpreter
 * ============================================================
 *  The Interpreter is the EXECUTION phase.
 *  It walks the AST produced by the Parser and evaluates
 *  each node, producing side effects (printing, storing
 *  variables, calling functions).
 *
 *  Implements both ExprVisitor and StmtVisitor interfaces
 *  using the Visitor design pattern.
 *
 *  OOP Concepts:
 *    - Multiple Interface Implementation (ExprVisitor + StmtVisitor)
 *    - Visitor Pattern (double dispatch)
 *    - Polymorphism (BharatCallable subclasses)
 *    - Closure (functions capture their environment)
 *    - Exception-based control flow (return/break/continue)
 * ============================================================
 */
#pragma once

#include "AST.h"
#include "Value.h"
#include "Environment.h"
#include "ErrorReporter.h"
#include <vector>
#include <memory>
#include <iostream>
#include <functional>

// ── Forward declaration ────────────────────────────────────
class Interpreter;

// ═══════════════════════════════════════════════════════════
//  BharatFunction — User-defined function (kaam)
//  Captures the closure environment where it was declared.
// ═══════════════════════════════════════════════════════════
class BharatFunction : public BharatCallable {
public:
    FunctionStmt*                declaration_;
    std::shared_ptr<Environment> closure_;

    BharatFunction(FunctionStmt* declaration,
                   std::shared_ptr<Environment> closure)
        : declaration_(declaration), closure_(std::move(closure)) {}

    int arity() const override {
        return static_cast<int>(declaration_->params.size());
    }

    Value call(Interpreter& interpreter,
               std::vector<Value> arguments) override;

    std::string toString() const override {
        return "<kaam " + declaration_->name.lexeme + ">";
    }
};

// ═══════════════════════════════════════════════════════════
//  NativeFunction — Built-in function wrapper
//  Wraps a C++ lambda as a BharatLang callable.
// ═══════════════════════════════════════════════════════════
class NativeFunction : public BharatCallable {
public:
    using NativeFn = std::function<Value(Interpreter&, std::vector<Value>)>;

    NativeFunction(std::string name, int arity, NativeFn function)
        : name_(std::move(name)), arity_(arity),
          function_(std::move(function)) {}

    int arity() const override { return arity_; }

    Value call(Interpreter& interpreter,
               std::vector<Value> arguments) override {
        return function_(interpreter, std::move(arguments));
    }

    std::string toString() const override {
        return "<native kaam " + name_ + ">";
    }

private:
    std::string name_;
    int         arity_;
    NativeFn    function_;
};

// ═══════════════════════════════════════════════════════════
//  INTERPRETER CLASS
// ═══════════════════════════════════════════════════════════
class Interpreter : public ExprVisitor, public StmtVisitor {
public:
    Interpreter(ErrorReporter& reporter);

    /**
     * Execute a list of statements (the whole program).
     */
    void interpret(std::vector<StmtPtr>& statements);

    /**
     * Execute a block of statements in the given environment.
     * Used by BharatFunction::call for function bodies.
     */
    void executeBlock(std::vector<StmtPtr>& statements,
                      std::shared_ptr<Environment> env);

    /**
     * Get the global environment (for native function registration).
     */
    std::shared_ptr<Environment> globalEnvironment() { return globals_; }

private:
    ErrorReporter&               reporter_;
    std::shared_ptr<Environment> globals_;
    std::shared_ptr<Environment> env_;      // current environment

    // ── Register native (built-in) functions ───────────────
    void registerNatives();

    // ── Statement execution ────────────────────────────────
    void execute(StmtPtr& stmt);

    // ── Expression evaluation ──────────────────────────────
    Value evaluate(ExprPtr& expr);

    // ═══════════════════════════════════════════════════════
    //  EXPRESSION VISITOR METHODS
    // ═══════════════════════════════════════════════════════
    Value visit(BinaryExpr& expr)   override;
    Value visit(UnaryExpr& expr)    override;
    Value visit(LiteralExpr& expr)  override;
    Value visit(VariableExpr& expr) override;
    Value visit(AssignExpr& expr)   override;
    Value visit(LogicalExpr& expr)  override;
    Value visit(CallExpr& expr)     override;
    Value visit(GroupingExpr& expr) override;
    Value visit(InputExpr& expr)    override;

    // ═══════════════════════════════════════════════════════
    //  STATEMENT VISITOR METHODS
    // ═══════════════════════════════════════════════════════
    void visit(ExpressionStmt& stmt) override;
    void visit(PrintStmt& stmt)      override;
    void visit(VarDeclStmt& stmt)    override;
    void visit(BlockStmt& stmt)      override;
    void visit(IfStmt& stmt)         override;
    void visit(WhileStmt& stmt)      override;
    void visit(ForStmt& stmt)        override;
    void visit(FunctionStmt& stmt)   override;
    void visit(ReturnStmt& stmt)     override;
    void visit(BreakStmt& stmt)      override;
    void visit(ContinueStmt& stmt)   override;

    // ── Arithmetic helpers ─────────────────────────────────
    void checkNumberOperand(const Token& op, const Value& val);
    void checkNumberOperands(const Token& op, const Value& left, const Value& right);
};
