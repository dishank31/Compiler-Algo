/**
 * ============================================================
 *  ErrorReporter.h — BharatLang Hinglish Error Reporting
 * ============================================================
 *  Centralized error handling with Hinglish error messages.
 *  All compiler phases report errors through this class.
 *
 *  OOP Concepts: Encapsulation, static utility class,
 *  exception hierarchy.
 * ============================================================
 */
#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <sstream>

#include "Value.h"

// ── Custom Exception Classes ───────────────────────────────

/**
 * LexerError — thrown when the lexer encounters invalid input.
 */
class LexerError : public std::runtime_error {
public:
    int line, column;
    LexerError(const std::string& msg, int line, int col)
        : std::runtime_error(msg), line(line), column(col) {}
};

/**
 * ParseError — thrown when the parser encounters invalid syntax.
 */
class ParseError : public std::runtime_error {
public:
    int line, column;
    ParseError(const std::string& msg, int line, int col)
        : std::runtime_error(msg), line(line), column(col) {}
};

/**
 * RuntimeError — thrown during interpretation.
 */
class BharatRuntimeError : public std::runtime_error {
public:
    int line, column;
    BharatRuntimeError(const std::string& msg, int line = 0, int col = 0)
        : std::runtime_error(msg), line(line), column(col) {}
};

/**
 * ReturnSignal — not a real error, used to unwind the call
 * stack when a lautao (return) statement is executed.
 * Demonstrates: exception-based control flow.
 */
class ReturnSignal : public std::exception {
public:
    Value value;
    explicit ReturnSignal(Value value) : value(std::move(value)) {}
};

/**
 * BreakSignal — used to break out of loops (ruko).
 */
class BreakSignal : public std::exception {};

/**
 * ContinueSignal — used to skip to next iteration (agla).
 */
class ContinueSignal : public std::exception {};

// ── ErrorReporter — Central error output ───────────────────

class ErrorReporter {
private:
    std::vector<std::string> errors_;
    std::vector<std::string> warnings_;
    bool hadError_ = false;

public:
    // Report an error with Hinglish message
    void error(int line, int col, const std::string& message) {
        std::ostringstream oss;
        oss << "\033[1;31m"   // bold red
            << "❌ Galti"
            << " [line " << line << ", column " << col << "]: "
            << "\033[0m"      // reset
            << message;
        errors_.push_back(oss.str());
        hadError_ = true;
        std::cerr << oss.str() << std::endl;
    }

    // Report a warning
    void warning(int line, int col, const std::string& message) {
        std::ostringstream oss;
        oss << "\033[1;33m"   // bold yellow
            << "⚠️  Chetavani"
            << " [line " << line << ", column " << col << "]: "
            << "\033[0m"
            << message;
        warnings_.push_back(oss.str());
        std::cerr << oss.str() << std::endl;
    }

    // Report a runtime error
    void runtimeError(const BharatRuntimeError& err) {
        std::ostringstream oss;
        oss << "\033[1;31m"
            << "💥 Runtime Galti";
        if (err.line > 0) {
            oss << " [line " << err.line << "]";
        }
        oss << ": \033[0m" << err.what();
        std::cerr << oss.str() << std::endl;
        hadError_ = true;
    }

    bool hadError() const { return hadError_; }

    void reset() {
        errors_.clear();
        warnings_.clear();
        hadError_ = false;
    }

    int errorCount()   const { return static_cast<int>(errors_.size()); }
    int warningCount() const { return static_cast<int>(warnings_.size()); }
};
