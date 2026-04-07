/**
 * ============================================================
 *  Environment.h — BharatLang Runtime Environment
 * ============================================================
 *  Implements a scope-chained variable storage system.
 *
 *  Each Environment holds a map of (name → Value) and an
 *  optional pointer to its enclosing (parent) scope.
 *
 *  Scope chain: inner → outer → outer → ... → global
 *
 *  When looking up a variable, we search the current scope
 *  first, then walk up the chain. This gives us lexical
 *  scoping (block scope).
 *
 *  OOP Concepts:
 *    - Encapsulation (private variable map)
 *    - Composition (enclosing pointer)
 *    - Scope Chain pattern (linked list of scopes)
 *    - shared_ptr for shared ownership
 * ============================================================
 */
#pragma once

#include "Value.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

class Environment : public std::enable_shared_from_this<Environment> {
public:
    /**
     * Create a global environment (no parent).
     */
    Environment() : enclosing_(nullptr) {}

    /**
     * Create a new scope enclosed by the given parent.
     * Variables not found here will be looked up in the parent.
     */
    explicit Environment(std::shared_ptr<Environment> enclosing)
        : enclosing_(std::move(enclosing)) {}

    /**
     * Define a new variable in the current scope.
     * Overwrites if already exists in this scope.
     */
    void define(const std::string& name, Value value) {
        values_[name] = std::move(value);
    }

    /**
     * Get the value of a variable.
     * Searches up the scope chain.
     * Throws if not found.
     */
    Value get(const std::string& name) const {
        auto it = values_.find(name);
        if (it != values_.end()) {
            return it->second;
        }

        // Walk up the scope chain
        if (enclosing_) {
            return enclosing_->get(name);
        }

        throw std::runtime_error(
            "'" + name + "' naam ka variable milta nahi! "
            "Pehle 'rakho' se declare karo.");
    }

    /**
     * Assign a new value to an existing variable.
     * Searches up the scope chain.
     * Throws if the variable was never declared.
     */
    void assign(const std::string& name, Value value) {
        auto it = values_.find(name);
        if (it != values_.end()) {
            it->second = std::move(value);
            return;
        }

        if (enclosing_) {
            enclosing_->assign(name, std::move(value));
            return;
        }

        throw std::runtime_error(
            "'" + name + "' ko assign nahi kar sakte — "
            "pehle 'rakho' se declare karo!");
    }

    /**
     * Check if a variable exists in the current scope (not parent).
     */
    bool existsInCurrentScope(const std::string& name) const {
        return values_.find(name) != values_.end();
    }

    /**
     * Get the enclosing (parent) environment.
     */
    std::shared_ptr<Environment> enclosing() const {
        return enclosing_;
    }

private:
    std::unordered_map<std::string, Value> values_;
    std::shared_ptr<Environment>           enclosing_;
};
