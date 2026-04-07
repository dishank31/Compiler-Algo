/**
 * ============================================================
 *  IRGenerator.cpp — Three-Address Code Generation
 * ============================================================
 *  Walks the AST and produces Three-Address Code (TAC).
 *
 *  Example input:
 *    rakho x = 5 + 3 * 2;
 *    likho(x);
 *
 *  Example output:
 *    t0 = 3 * 2
 *    t1 = 5 + t0
 *    x = t1
 *    PRINT x
 * ============================================================
 */

#include "IRGenerator.h"

// ═══════════════════════════════════════════════════════════
//  CONSTRUCTOR
// ═══════════════════════════════════════════════════════════
IRGenerator::IRGenerator() {}

// ═══════════════════════════════════════════════════════════
//  GENERATE — Entry point
// ═══════════════════════════════════════════════════════════
void IRGenerator::generate(std::vector<StmtPtr>& statements) {
    for (auto& stmt : statements) {
        stmt->accept(*this);
    }
}

// ═══════════════════════════════════════════════════════════
//  PRETTY-PRINT
// ═══════════════════════════════════════════════════════════
std::string IRGenerator::toString() const {
    std::ostringstream oss;
    oss << "═══════════════════════════════════════\n";
    oss << "  THREE-ADDRESS CODE (TAC)\n";
    oss << "═══════════════════════════════════════\n";
    for (const auto& instr : instructions_) {
        oss << instr.toString() << "\n";
    }
    oss << "═══════════════════════════════════════\n";
    return oss.str();
}

// ═══════════════════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════════════════
std::string IRGenerator::newTemp() {
    return "t" + std::to_string(tempCount_++);
}

std::string IRGenerator::newLabel() {
    return "L" + std::to_string(labelCount_++);
}

void IRGenerator::emit(const std::string& op,
                       const std::string& arg1,
                       const std::string& arg2,
                       const std::string& result) {
    instructions_.push_back({op, arg1, arg2, result});
}

// ═══════════════════════════════════════════════════════════
//  EXPRESSION VISITORS
// ═══════════════════════════════════════════════════════════

Value IRGenerator::visit(LiteralExpr& expr) {
    std::string val = valueToString(expr.value);
    // Wrap string literals in quotes
    if (std::holds_alternative<std::string>(expr.value)) {
        val = "\"" + val + "\"";
    }
    lastResult_ = val;
    return std::monostate{};
}

Value IRGenerator::visit(VariableExpr& expr) {
    lastResult_ = expr.name.lexeme;
    return std::monostate{};
}

Value IRGenerator::visit(GroupingExpr& expr) {
    expr.expression->accept(*this);
    return std::monostate{};
}

Value IRGenerator::visit(BinaryExpr& expr) {
    // Evaluate left operand
    expr.left->accept(*this);
    std::string leftResult = lastResult_;

    // Evaluate right operand
    expr.right->accept(*this);
    std::string rightResult = lastResult_;

    // Generate the binary operation
    std::string temp = newTemp();
    std::string op;
    switch (expr.op.type) {
        case TokenType::PLUS:          op = "+";  break;
        case TokenType::MINUS:         op = "-";  break;
        case TokenType::STAR:          op = "*";  break;
        case TokenType::SLASH:         op = "/";  break;
        case TokenType::MODULO:        op = "%";  break;
        case TokenType::EQUAL:         op = "=="; break;
        case TokenType::NOT_EQUAL:     op = "!="; break;
        case TokenType::LESS:          op = "<";  break;
        case TokenType::LESS_EQUAL:    op = "<="; break;
        case TokenType::GREATER:       op = ">";  break;
        case TokenType::GREATER_EQUAL: op = ">="; break;
        default: op = expr.op.lexeme; break;
    }

    emit(op, leftResult, rightResult, temp);
    lastResult_ = temp;
    return std::monostate{};
}

Value IRGenerator::visit(UnaryExpr& expr) {
    expr.operand->accept(*this);
    std::string operand = lastResult_;

    std::string temp = newTemp();
    std::string op = (expr.op.type == TokenType::MINUS) ? "NEG" : "NOT";
    emit(op, operand, "", temp);
    lastResult_ = temp;
    return std::monostate{};
}

Value IRGenerator::visit(AssignExpr& expr) {
    expr.value->accept(*this);
    emit("ASSIGN", lastResult_, "", expr.name.lexeme);
    lastResult_ = expr.name.lexeme;
    return std::monostate{};
}

Value IRGenerator::visit(LogicalExpr& expr) {
    std::string endLabel = newLabel();

    // Evaluate left
    expr.left->accept(*this);
    std::string leftResult = lastResult_;
    std::string temp = newTemp();
    emit("ASSIGN", leftResult, "", temp);

    if (expr.op.type == TokenType::YA) {
        // Short-circuit OR: if left is true, skip right
        emit("IF_TRUE", temp, "", endLabel);
    } else {
        // Short-circuit AND: if left is false, skip right
        emit("IF_FALSE", temp, "", endLabel);
    }

    // Evaluate right
    expr.right->accept(*this);
    emit("ASSIGN", lastResult_, "", temp);

    emit("LABEL", "", "", endLabel);
    lastResult_ = temp;
    return std::monostate{};
}

Value IRGenerator::visit(CallExpr& expr) {
    // Evaluate arguments and emit PARAM instructions
    std::vector<std::string> argResults;
    for (auto& arg : expr.arguments) {
        arg->accept(*this);
        argResults.push_back(lastResult_);
    }

    for (const auto& argRes : argResults) {
        emit("PARAM", argRes);
    }

    // Get function name
    expr.callee->accept(*this);
    std::string funcName = lastResult_;

    // Emit CALL
    std::string temp = newTemp();
    emit("CALL", funcName, std::to_string(argResults.size()), temp);
    lastResult_ = temp;
    return std::monostate{};
}

Value IRGenerator::visit(InputExpr& expr) {
    std::string prompt = "";
    if (expr.prompt) {
        expr.prompt->accept(*this);
        prompt = lastResult_;
    }

    std::string temp = newTemp();
    emit("INPUT", prompt, "", temp);
    lastResult_ = temp;
    return std::monostate{};
}

// ═══════════════════════════════════════════════════════════
//  STATEMENT VISITORS
// ═══════════════════════════════════════════════════════════

void IRGenerator::visit(ExpressionStmt& stmt) {
    stmt.expression->accept(*this);
}

void IRGenerator::visit(PrintStmt& stmt) {
    for (auto& expr : stmt.expressions) {
        expr->accept(*this);
        emit("PRINT", lastResult_);
    }
}

void IRGenerator::visit(VarDeclStmt& stmt) {
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
        emit("ASSIGN", lastResult_, "", stmt.name.lexeme);
    } else {
        emit("ASSIGN", "shunya", "", stmt.name.lexeme);
    }
}

void IRGenerator::visit(BlockStmt& stmt) {
    for (auto& s : stmt.statements) {
        s->accept(*this);
    }
}

void IRGenerator::visit(IfStmt& stmt) {
    std::string endLabel = newLabel();

    // Main condition (agar)
    stmt.condition->accept(*this);
    std::string elseLabel = newLabel();
    emit("IF_FALSE", lastResult_, "", elseLabel);

    // Then branch
    stmt.thenBranch->accept(*this);
    emit("GOTO", "", "", endLabel);

    // Else-if branches (warna_agar)
    std::string currentElse = elseLabel;
    for (size_t i = 0; i < stmt.elifConditions.size(); i++) {
        emit("LABEL", "", "", currentElse);

        stmt.elifConditions[i]->accept(*this);
        std::string nextElse = newLabel();
        emit("IF_FALSE", lastResult_, "", nextElse);

        stmt.elifBranches[i]->accept(*this);
        emit("GOTO", "", "", endLabel);

        currentElse = nextElse;
    }

    // Else branch (warna)
    emit("LABEL", "", "", currentElse);
    if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }

    emit("LABEL", "", "", endLabel);
}

void IRGenerator::visit(WhileStmt& stmt) {
    std::string startLabel = newLabel();
    std::string endLabel   = newLabel();

    emit("LABEL", "", "", startLabel);

    // Evaluate condition
    stmt.condition->accept(*this);
    emit("IF_FALSE", lastResult_, "", endLabel);

    // Body
    stmt.body->accept(*this);
    emit("GOTO", "", "", startLabel);

    emit("LABEL", "", "", endLabel);
}

void IRGenerator::visit(ForStmt& stmt) {
    std::string startLabel = newLabel();
    std::string endLabel   = newLabel();

    // Initializer
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
    }

    emit("LABEL", "", "", startLabel);

    // Condition
    if (stmt.condition) {
        stmt.condition->accept(*this);
        emit("IF_FALSE", lastResult_, "", endLabel);
    }

    // Body
    stmt.body->accept(*this);

    // Increment
    if (stmt.increment) {
        stmt.increment->accept(*this);
    }

    emit("GOTO", "", "", startLabel);
    emit("LABEL", "", "", endLabel);
}

void IRGenerator::visit(FunctionStmt& stmt) {
    emit("FUNC_BEGIN", stmt.name.lexeme);

    for (auto& s : stmt.body) {
        s->accept(*this);
    }

    emit("FUNC_END", stmt.name.lexeme);
}

void IRGenerator::visit(ReturnStmt& stmt) {
    if (stmt.value) {
        stmt.value->accept(*this);
        emit("RETURN", lastResult_);
    } else {
        emit("RETURN", "shunya");
    }
}

void IRGenerator::visit(BreakStmt&) {
    // In a full compiler, this would GOTO the loop end label
    emit("BREAK");
}

void IRGenerator::visit(ContinueStmt&) {
    // In a full compiler, this would GOTO the loop start label
    emit("CONTINUE");
}
