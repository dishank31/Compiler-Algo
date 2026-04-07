/**
 * ============================================================
 *  IRGenerator.h — BharatLang IR Code Generator (Declaration)
 * ============================================================
 *  Generates Three-Address Code (TAC) from the AST.
 *  TAC is an intermediate representation used by real
 *  compilers between parsing and machine code generation.
 *
 *  Each TAC instruction has at most 3 addresses:
 *    result = operand1 OP operand2
 *
 *  This demonstrates the code generation phase of a compiler
 *  and produces human-readable output for debugging.
 *
 *  OOP Concepts:
 *    - Visitor Pattern (reusing the same AST visitors)
 *    - Factory Pattern (temporary variable generation)
 *    - Encapsulation (instruction list management)
 * ============================================================
 */
#pragma once

#include "AST.h"
#include "Value.h"
#include <vector>
#include <string>
#include <sstream>

// ── A single Three-Address Code instruction ────────────────
struct TACInstruction {
    std::string op;         // operation: ASSIGN, ADD, SUB, etc.
    std::string arg1;       // first operand
    std::string arg2;       // second operand (may be empty)
    std::string result;     // destination

    std::string toString() const {
        if (op == "LABEL") {
            return result + ":";
        }
        if (op == "GOTO") {
            return "    GOTO " + result;
        }
        if (op == "IF_FALSE") {
            return "    IF_FALSE " + arg1 + " GOTO " + result;
        }
        if (op == "IF_TRUE") {
            return "    IF_TRUE " + arg1 + " GOTO " + result;
        }
        if (op == "PARAM") {
            return "    PARAM " + arg1;
        }
        if (op == "CALL") {
            return "    " + result + " = CALL " + arg1 + ", " + arg2;
        }
        if (op == "RETURN") {
            return "    RETURN " + arg1;
        }
        if (op == "PRINT") {
            return "    PRINT " + arg1;
        }
        if (op == "INPUT") {
            return "    " + result + " = INPUT " + arg1;
        }
        if (op == "ASSIGN") {
            return "    " + result + " = " + arg1;
        }
        if (op == "FUNC_BEGIN") {
            return "\nFUNC_BEGIN " + arg1;
        }
        if (op == "FUNC_END") {
            return "FUNC_END " + arg1;
        }

        // General binary operation
        if (!arg2.empty()) {
            return "    " + result + " = " + arg1 + " " + op + " " + arg2;
        }
        // Unary operation
        return "    " + result + " = " + op + " " + arg1;
    }
};

// ═══════════════════════════════════════════════════════════
//  IR GENERATOR CLASS
// ═══════════════════════════════════════════════════════════
class IRGenerator : public ExprVisitor, public StmtVisitor {
public:
    IRGenerator();

    /**
     * Generate IR for the entire program.
     */
    void generate(std::vector<StmtPtr>& statements);

    /**
     * Get the generated instructions.
     */
    const std::vector<TACInstruction>& getInstructions() const {
        return instructions_;
    }

    /**
     * Pretty-print all instructions.
     */
    std::string toString() const;

private:
    std::vector<TACInstruction> instructions_;
    int tempCount_  = 0;    // for generating t0, t1, t2, ...
    int labelCount_ = 0;    // for generating L0, L1, L2, ...
    std::string lastResult_;// result of last expression eval

    // ── Factory methods for temps and labels ───────────────
    std::string newTemp();
    std::string newLabel();

    // ── Emit an instruction ────────────────────────────────
    void emit(const std::string& op,
              const std::string& arg1 = "",
              const std::string& arg2 = "",
              const std::string& result = "");

    // ── Expression visitors (return Value, but we use
    //    lastResult_ to track the result name) ──────────────
    Value visit(BinaryExpr& expr)   override;
    Value visit(UnaryExpr& expr)    override;
    Value visit(LiteralExpr& expr)  override;
    Value visit(VariableExpr& expr) override;
    Value visit(AssignExpr& expr)   override;
    Value visit(LogicalExpr& expr)  override;
    Value visit(CallExpr& expr)     override;
    Value visit(GroupingExpr& expr) override;
    Value visit(InputExpr& expr)    override;

    // ── Statement visitors ─────────────────────────────────
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
};
