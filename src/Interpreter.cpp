/**
 * ============================================================
 *  Interpreter.cpp — BharatLang Tree-Walking Interpreter
 * ============================================================
 *  Walks the AST and evaluates every node.
 *
 *  Expression evaluation returns a Value.
 *  Statement execution produces side effects.
 *
 *  Control flow (return, break, continue) is implemented
 *  using C++ exceptions as signals — this is the standard
 *  technique for tree-walking interpreters.
 * ============================================================
 */

#include "Interpreter.h"
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace std;

// ═══════════════════════════════════════════════════════════
//  BharatFunction::call — Execute a user-defined function
// ═══════════════════════════════════════════════════════════
Value BharatFunction::call(Interpreter &interpreter,
                           std::vector<Value> arguments) {
  // Create a new environment enclosed by the closure
  auto env = std::make_shared<Environment>(closure_);

  // Bind parameters to arguments
  for (int i = 0; i < static_cast<int>(declaration_->params.size()); i++) {
    env->define(declaration_->params[i].lexeme, std::move(arguments[i]));
  }

  // Execute the function body
  try {
    interpreter.executeBlock(declaration_->body, env);
  } catch (ReturnSignal &signal) {
    return std::move(signal.value);
  }

  // If no return statement was hit, return shunya
  return std::monostate{};
}

// ═══════════════════════════════════════════════════════════
//  CONSTRUCTOR — set up global environment with natives
// ═══════════════════════════════════════════════════════════
Interpreter::Interpreter(ErrorReporter &reporter)
    : reporter_(reporter), globals_(std::make_shared<Environment>()),
      env_(globals_) {
  registerNatives();
}

// ═══════════════════════════════════════════════════════════
//  NATIVE FUNCTION REGISTRATION
// ═══════════════════════════════════════════════════════════
void Interpreter::registerNatives() {
  // samay() — returns current time in seconds (like clock())
  globals_->define(
      "samay",
      std::make_shared<NativeFunction>(
          "samay", 0, [](Interpreter &, std::vector<Value>) -> Value {
            auto now = std::chrono::system_clock::now();
            auto epoch = now.time_since_epoch();
            auto seconds =
                std::chrono::duration_cast<std::chrono::milliseconds>(epoch)
                    .count();
            return static_cast<double>(seconds) / 1000.0;
          }));

  // lambai(shabd) — returns length of a string
  globals_->define(
      "lambai",
      std::make_shared<NativeFunction>(
          "lambai", 1, [](Interpreter &, std::vector<Value> args) -> Value {
            if (!std::holds_alternative<std::string>(args[0])) {
              throw BharatRuntimeError("'lambai()' ko shabd (string) chahiye!");
            }
            return static_cast<int>(std::get<std::string>(args[0]).size());
          }));

  // prakar(value) — returns the type name as a string
  globals_->define(
      "prakar",
      std::make_shared<NativeFunction>(
          "prakar", 1, [](Interpreter &, std::vector<Value> args) -> Value {
            return typeName(args[0]);
          }));

  // ank_banao(value) — convert to integer
  globals_->define(
      "ank_banao",
      std::make_shared<NativeFunction>(
          "ank_banao", 1, [](Interpreter &, std::vector<Value> args) -> Value {
            double d;
            if (asDouble(args[0], d)) {
              return static_cast<int>(d);
            }
            if (std::holds_alternative<bool>(args[0])) {
              return std::get<bool>(args[0]) ? 1 : 0;
            }
            throw BharatRuntimeError(
                "Isko ank (integer) mein nahi badal sakte!");
            return Value{std::monostate{}};
          }));

  // shabd_banao(value) — convert to string
  globals_->define("shabd_banao",
                   std::make_shared<NativeFunction>(
                       "shabd_banao", 1,
                       [](Interpreter &, std::vector<Value> args) -> Value {
                         return valueToString(args[0]);
                       }));
}

// ═══════════════════════════════════════════════════════════
//  INTERPRET — Main entry point
// ═══════════════════════════════════════════════════════════
void Interpreter::interpret(std::vector<StmtPtr> &statements) {
  try {
    for (auto &stmt : statements) {
      execute(stmt);
    }
  } catch (BharatRuntimeError &err) {
    reporter_.runtimeError(err);
  }
}

void Interpreter::execute(StmtPtr &stmt) { stmt->accept(*this); }

Value Interpreter::evaluate(ExprPtr &expr) { return expr->accept(*this); }

void Interpreter::executeBlock(std::vector<StmtPtr> &statements,
                               std::shared_ptr<Environment> env) {
  // Save current environment, switch to new one
  auto previous = env_;
  env_ = std::move(env);

  try {
    for (auto &stmt : statements) {
      execute(stmt);
    }
  } catch (...) {
    // Restore environment even on exception
    env_ = previous;
    throw;
  }

  env_ = previous;
}

// ═══════════════════════════════════════════════════════════
//  EXPRESSION VISITORS
// ═══════════════════════════════════════════════════════════

/**
 * LiteralExpr — just return the literal value.
 */
Value Interpreter::visit(LiteralExpr &expr) { return expr.value; }

/**
 * GroupingExpr — evaluate the inner expression.
 */
Value Interpreter::visit(GroupingExpr &expr) {
  return evaluate(expr.expression);
}

/**
 * VariableExpr — look up the variable in the environment.
 */
Value Interpreter::visit(VariableExpr &expr) {
  try {
    return env_->get(expr.name.lexeme);
  } catch (std::runtime_error &e) {
    throw BharatRuntimeError(e.what(), expr.name.line, expr.name.column);
  }
}

/**
 * AssignExpr — assign a value to an existing variable.
 */
Value Interpreter::visit(AssignExpr &expr) {
  Value value = evaluate(expr.value);
  try {
    env_->assign(expr.name.lexeme, value);
  } catch (std::runtime_error &e) {
    throw BharatRuntimeError(e.what(), expr.name.line, expr.name.column);
  }
  return value;
}

/**
 * UnaryExpr — apply prefix operator.
 *   -x    → negate a number
 *   nahi x → logical NOT
 */
Value Interpreter::visit(UnaryExpr &expr) {
  Value right = evaluate(expr.operand);

  switch (expr.op.type) {
  case TokenType::MINUS: {
    if (std::holds_alternative<int>(right))
      return -std::get<int>(right);
    if (std::holds_alternative<double>(right))
      return -std::get<double>(right);
    throw BharatRuntimeError("'-' sirf numbers ke saath kaam karta hai!",
                             expr.op.line, expr.op.column);
  }

  case TokenType::NAHI:
    return !isTruthy(right);

  default:
    break;
  }

  return std::monostate{};
}

/**
 * BinaryExpr — apply an operator to two operands.
 * Handles arithmetic, comparison, equality, and string concat.
 */
Value Interpreter::visit(BinaryExpr &expr) {
  Value left = evaluate(expr.left);
  Value right = evaluate(expr.right);

  switch (expr.op.type) {

  // ── String concatenation with + ────────────────────
  case TokenType::PLUS: {
    // String + anything → string concatenation
    if (std::holds_alternative<std::string>(left) ||
        std::holds_alternative<std::string>(right)) {
      return valueToString(left) + valueToString(right);
    }
    // int + int → int
    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      return std::get<int>(left) + std::get<int>(right);
    }
    // numeric + numeric → double
    double l, r;
    if (asDouble(left, l) && asDouble(right, r)) {
      return l + r;
    }
    throw BharatRuntimeError(
        "'+' sirf numbers ya strings ke saath kaam karta hai!", expr.op.line,
        expr.op.column);
  }

  // ── Arithmetic operators ───────────────────────────
  case TokenType::MINUS: {
    if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right))
      return std::get<int>(left) - std::get<int>(right);
    double l, r;
    if (asDouble(left, l) && asDouble(right, r))
      return l - r;
    throw BharatRuntimeError("'-' sirf numbers ke saath kaam karta hai!",
                             expr.op.line, expr.op.column);
  }

  case TokenType::STAR: {
    if (std::holds_alternative<int>(left) && std::holds_alternative<int>(right))
      return std::get<int>(left) * std::get<int>(right);
    double l, r;
    if (asDouble(left, l) && asDouble(right, r))
      return l * r;
    throw BharatRuntimeError("'*' sirf numbers ke saath kaam karta hai!",
                             expr.op.line, expr.op.column);
  }

  case TokenType::SLASH: {
    double l, r;
    if (!asDouble(left, l) || !asDouble(right, r)) {
      throw BharatRuntimeError("'/' sirf numbers ke saath kaam karta hai!",
                               expr.op.line, expr.op.column);
    }
    if (r == 0) {
      throw BharatRuntimeError("Zero se divide nahi kar sakte bhai! 💥",
                               expr.op.line, expr.op.column);
    }
    // int / int → int (integer division) if both are int
    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      return std::get<int>(left) / std::get<int>(right);
    }
    return l / r;
  }

  case TokenType::MODULO: {
    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      int r = std::get<int>(right);
      if (r == 0) {
        throw BharatRuntimeError("Zero se modulo nahi kar sakte!", expr.op.line,
                                 expr.op.column);
      }
      return std::get<int>(left) % r;
    }
    double l, r;
    if (asDouble(left, l) && asDouble(right, r)) {
      if (r == 0) {
        throw BharatRuntimeError("Zero se modulo nahi kar sakte!", expr.op.line,
                                 expr.op.column);
      }
      return std::fmod(l, r);
    }
    throw BharatRuntimeError("'%' sirf numbers ke saath kaam karta hai!",
                             expr.op.line, expr.op.column);
  }

  // ── Comparison operators ───────────────────────────
  case TokenType::LESS: {
    double l, r;
    if (asDouble(left, l) && asDouble(right, r))
      return l < r;
    if (std::holds_alternative<std::string>(left) &&
        std::holds_alternative<std::string>(right))
      return std::get<std::string>(left) < std::get<std::string>(right);
    throw BharatRuntimeError(
        "'<' sirf numbers ya strings compare kar sakta hai!", expr.op.line,
        expr.op.column);
  }

  case TokenType::LESS_EQUAL: {
    double l, r;
    if (asDouble(left, l) && asDouble(right, r))
      return l <= r;
    throw BharatRuntimeError("'<=' sirf numbers compare kar sakta hai!",
                             expr.op.line, expr.op.column);
  }

  case TokenType::GREATER: {
    double l, r;
    if (asDouble(left, l) && asDouble(right, r))
      return l > r;
    throw BharatRuntimeError("'>' sirf numbers compare kar sakta hai!",
                             expr.op.line, expr.op.column);
  }

  case TokenType::GREATER_EQUAL: {
    double l, r;
    if (asDouble(left, l) && asDouble(right, r))
      return l >= r;
    throw BharatRuntimeError("'>=' sirf numbers compare kar sakta hai!",
                             expr.op.line, expr.op.column);
  }

  // ── Equality ───────────────────────────────────────
  case TokenType::EQUAL:
    return isEqual(left, right);

  case TokenType::NOT_EQUAL:
    return !isEqual(left, right);

  default:
    break;
  }

  return std::monostate{};
}

/**
 * LogicalExpr — short-circuit logical operators.
 *   a aur b → if a is galat, return a; else evaluate & return b
 *   a ya  b → if a is sahi,  return a; else evaluate & return b
 */
Value Interpreter::visit(LogicalExpr &expr) {
  Value left = evaluate(expr.left);

  if (expr.op.type == TokenType::YA) {
    if (isTruthy(left))
      return left; // short-circuit
  } else {
    if (!isTruthy(left))
      return left; // short-circuit
  }

  return evaluate(expr.right);
}

/**
 * CallExpr — function call.
 * 1. Evaluate the callee (should resolve to a BharatCallable)
 * 2. Evaluate all arguments
 * 3. Check arity (argument count)
 * 4. Call the function
 */
Value Interpreter::visit(CallExpr &expr) {
  Value callee = evaluate(expr.callee);

  // Evaluate arguments
  std::vector<Value> arguments;
  for (auto &arg : expr.arguments) {
    arguments.push_back(evaluate(arg));
  }

  // Check that callee is a callable
  if (!std::holds_alternative<std::shared_ptr<BharatCallable>>(callee)) {
    throw BharatRuntimeError("Sirf functions ko call kar sakte ho!",
                             expr.paren.line, expr.paren.column);
  }

  auto function = std::get<std::shared_ptr<BharatCallable>>(callee);

  // Check arity
  if (static_cast<int>(arguments.size()) != function->arity()) {
    throw BharatRuntimeError("Function ko " +
                                 std::to_string(function->arity()) +
                                 " arguments chahiye, lekin " +
                                 std::to_string(arguments.size()) + " diye!",
                             expr.paren.line, expr.paren.column);
  }

  return function->call(*this, std::move(arguments));
}

/**
 * InputExpr — padho() reads a line from stdin.
 * Attempts to auto-convert to int or double.
 */
Value Interpreter::visit(InputExpr &expr) {
  // Print prompt if provided
  if (expr.prompt) {
    Value promptVal = evaluate(expr.prompt);
    std::cout << valueToString(promptVal);
    std::cout.flush();
  }

  // Read a line from stdin
  std::string input;
  std::getline(std::cin, input);

  // Try to auto-convert: int → double → string
  try {
    size_t pos;
    int i = std::stoi(input, &pos);
    if (pos == input.length())
      return i;
  } catch (...) {
  }

  try {
    size_t pos;
    double d = std::stod(input, &pos);
    if (pos == input.length())
      return d;
  } catch (...) {
  }

  return input;
}

// ═══════════════════════════════════════════════════════════
//  STATEMENT VISITORS
// ═══════════════════════════════════════════════════════════

/**
 * ExpressionStmt — evaluate expression, discard result.
 */
void Interpreter::visit(ExpressionStmt &stmt) { evaluate(stmt.expression); }

/**
 * PrintStmt — likho(...) prints values separated by spaces.
 */
void Interpreter::visit(PrintStmt &stmt) {
  for (size_t i = 0; i < stmt.expressions.size(); i++) {
    if (i > 0)
      std::cout << " ";
    Value val = evaluate(stmt.expressions[i]);
    std::cout << valueToString(val);
  }
  std::cout << std::endl;
}

/**
 * VarDeclStmt — rakho: declare a variable in current scope.
 */
void Interpreter::visit(VarDeclStmt &stmt) {
  Value value = std::monostate{}; // default: shunya
  if (stmt.initializer) {
    value = evaluate(stmt.initializer);
  }
  env_->define(stmt.name.lexeme, std::move(value));
}

/**
 * BlockStmt — execute statements in a new scope.
 */
void Interpreter::visit(BlockStmt &stmt) {
  auto blockEnv = std::make_shared<Environment>(env_);
  executeBlock(stmt.statements, blockEnv);
}

/**
 * IfStmt — agar / warna_agar / warna
 */
void Interpreter::visit(IfStmt &stmt) {
  // Check main condition
  if (isTruthy(evaluate(stmt.condition))) {
    execute(stmt.thenBranch);
    return;
  }

  // Check each warna_agar (else-if) branch
  for (size_t i = 0; i < stmt.elifConditions.size(); i++) {
    if (isTruthy(evaluate(stmt.elifConditions[i]))) {
      execute(stmt.elifBranches[i]);
      return;
    }
  }

  // Execute warna (else) branch if present
  if (stmt.elseBranch) {
    execute(stmt.elseBranch);
  }
}

/**
 * WhileStmt — jabtak loop.
 * Handles ruko (break) and agla (continue) signals.
 */
void Interpreter::visit(WhileStmt &stmt) {
  while (isTruthy(evaluate(stmt.condition))) {
    try {
      execute(stmt.body);
    } catch (BreakSignal &) {
      break;
    } catch (ContinueSignal &) {
      continue;
    }
  }
}

/**
 * ForStmt — har loop.
 * Desugared internally:
 *   har (init; condition; increment) { body }
 * becomes:
 *   { init; jabtak (condition) { body; increment; } }
 */
void Interpreter::visit(ForStmt &stmt) {
  // Create scope for the loop variable
  auto loopEnv = std::make_shared<Environment>(env_);
  auto previousEnv = env_;
  env_ = loopEnv;

  try {
    // Execute initializer
    if (stmt.initializer) {
      execute(stmt.initializer);
    }

    // Loop
    while (true) {
      // Check condition (if present)
      if (stmt.condition) {
        if (!isTruthy(evaluate(stmt.condition)))
          break;
      }

      // Execute body
      try {
        execute(stmt.body);
      } catch (BreakSignal &) {
        break;
      } catch (ContinueSignal &) {
        // Fall through to increment
      }

      // Execute increment
      if (stmt.increment) {
        evaluate(stmt.increment);
      }
    }
  } catch (...) {
    env_ = previousEnv;
    throw;
  }

  env_ = previousEnv;
}

/**
 * FunctionStmt — kaam: declare a function.
 * Creates a BharatFunction that captures the current environment
 * as its closure (for lexical scoping).
 */
void Interpreter::visit(FunctionStmt &stmt) {
  auto function = std::make_shared<BharatFunction>(&stmt, env_);
  env_->define(stmt.name.lexeme, std::move(function));
}

/**
 * ReturnStmt — lautao: throw a ReturnSignal to unwind.
 */
void Interpreter::visit(ReturnStmt &stmt) {
  Value value = std::monostate{};
  if (stmt.value) {
    value = evaluate(stmt.value);
  }
  throw ReturnSignal(std::move(value));
}

/**
 * BreakStmt — ruko: throw a BreakSignal.
 */
void Interpreter::visit(BreakStmt &) { throw BreakSignal(); }

/**
 * ContinueStmt — agla: throw a ContinueSignal.
 */
void Interpreter::visit(ContinueStmt &) { throw ContinueSignal(); }

// ═══════════════════════════════════════════════════════════
//  HELPERS
// ═══════════════════════════════════════════════════════════

void Interpreter::checkNumberOperand(const Token &op, const Value &val) {
  if (std::holds_alternative<int>(val) || std::holds_alternative<double>(val))
    return;
  throw BharatRuntimeError("'" + op.lexeme + "' ko number chahiye!", op.line,
                           op.column);
}

void Interpreter::checkNumberOperands(const Token &op, const Value &left,
                                      const Value &right) {
  double l, r;
  if (asDouble(left, l) && asDouble(right, r))
    return;
  throw BharatRuntimeError("'" + op.lexeme + "' ko dono taraf numbers chahiye!",
                           op.line, op.column);
}
