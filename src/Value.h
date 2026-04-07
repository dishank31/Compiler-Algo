/**
 * ============================================================
 *  Value.h — BharatLang Runtime Value System
 * ============================================================
 *  Defines the dynamic Value type used throughout the
 *  interpreter.  A Value can hold:
 *    - shunya  (null  / std::monostate)
 *    - ank     (int)
 *    - dasham  (double)
 *    - shabd   (string)
 *    - haan_naa(bool)
 *    - kaam    (shared_ptr<BharatCallable>  — a callable)
 *
 *  Also defines the abstract BharatCallable interface that
 *  user-defined and native functions implement.
 * ============================================================
 */
#pragma once

#include <variant>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <cmath>

// ── Forward declaration for the callable pointer in Value ───
class BharatCallable;

// ── The universal Value type ───────────────────────────────
//  Every variable, expression result, and function argument
//  in BharatLang is a Value.
using Value = std::variant<
    std::monostate,                     // shunya (null)
    int,                                // ank
    double,                             // dasham
    std::string,                        // shabd
    bool,                               // haan_naa (sahi / galat)
    std::shared_ptr<BharatCallable>     // kaam (function)
>;

// ── Overloaded visitor helper (C++17 pattern) ──────────────
template <class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
// C++17 deduction guide (automatic in C++20)
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// ── Forward-declare Interpreter for BharatCallable ─────────
class Interpreter;

// ── BharatCallable — Abstract base for all callables ───────
//  Both user-defined (kaam) and native functions inherit
//  from this class.  Demonstrates: Abstraction, Polymorphism.
class BharatCallable {
public:
    virtual ~BharatCallable() = default;

    // Number of parameters the function expects
    virtual int arity() const = 0;

    // Execute the function body with the given arguments
    virtual Value call(Interpreter& interpreter,
                       std::vector<Value> arguments) = 0;

    // Human-readable representation (for likho / debugging)
    virtual std::string toString() const = 0;
};

// ── Value Helper Functions ─────────────────────────────────

// Check if a Value is "truthy" (sahi) or "falsy" (galat)
//  - shunya (null) → galat
//  - galat         → galat
//  - everything else → sahi
inline bool isTruthy(const Value& val) {
    return std::visit(overloaded{
        [](std::monostate)                          { return false; },
        [](bool v)                                  { return v;     },
        [](int)                                     { return true;  },
        [](double)                                  { return true;  },
        [](const std::string&)                      { return true;  },
        [](const std::shared_ptr<BharatCallable>&)  { return true;  },
    }, val);
}

// Convert a Value to its string representation
inline std::string valueToString(const Value& val) {
    return std::visit(overloaded{
        [](std::monostate) -> std::string { return "shunya"; },
        [](int v)          -> std::string { return std::to_string(v); },
        [](double v)       -> std::string {
            // Remove trailing zeros: 3.140000 → 3.14
            std::ostringstream oss;
            oss << v;
            return oss.str();
        },
        [](const std::string& v) -> std::string { return v; },
        [](bool v)               -> std::string { return v ? "sahi" : "galat"; },
        [](const std::shared_ptr<BharatCallable>& v) -> std::string {
            return v ? v->toString() : "shunya";
        },
    }, val);
}

// Check if two Values are equal
inline bool isEqual(const Value& a, const Value& b) {
    // Different variant indices → not equal
    if (a.index() != b.index()) return false;

    return std::visit(overloaded{
        [&](std::monostate) { return true; },
        [&](int v)          { return v == std::get<int>(b); },
        [&](double v)       { return std::abs(v - std::get<double>(b)) < 1e-9; },
        [&](const std::string& v) { return v == std::get<std::string>(b); },
        [&](bool v)         { return v == std::get<bool>(b); },
        [&](const std::shared_ptr<BharatCallable>& v) {
            return v == std::get<std::shared_ptr<BharatCallable>>(b);
        },
    }, a);
}

// Extract a numeric value (int or double) as double
// Returns true if conversion succeeded
inline bool asDouble(const Value& val, double& out) {
    if (std::holds_alternative<int>(val)) {
        out = static_cast<double>(std::get<int>(val));
        return true;
    }
    if (std::holds_alternative<double>(val)) {
        out = std::get<double>(val);
        return true;
    }
    // Try converting string to number
    if (std::holds_alternative<std::string>(val)) {
        try {
            size_t pos;
            out = std::stod(std::get<std::string>(val), &pos);
            return pos == std::get<std::string>(val).length();
        } catch (...) {
            return false;
        }
    }
    return false;
}

// Get the type name in Hinglish
inline std::string typeName(const Value& val) {
    return std::visit(overloaded{
        [](std::monostate)                          -> std::string { return "shunya"; },
        [](int)                                     -> std::string { return "ank"; },
        [](double)                                  -> std::string { return "dasham"; },
        [](const std::string&)                      -> std::string { return "shabd"; },
        [](bool)                                    -> std::string { return "haan_naa"; },
        [](const std::shared_ptr<BharatCallable>&)  -> std::string { return "kaam"; },
    }, val);
}
