/**
 * ============================================================
 *  Parser.h — BharatLang Recursive Descent Parser
 * ============================================================
 *  The Parser is the SECOND phase of compilation.
 *  It takes a flat list of Tokens from the Lexer and builds
 *  a hierarchical Abstract Syntax Tree (AST).
 *
 *  Grammar (simplified):
 *    program    → declaration* EOF
 *    declaration→ varDecl | funDecl | statement
 *    varDecl    → "rakho" IDENTIFIER ("=" expression)? ";"
 *    funDecl    → "kaam" IDENTIFIER "(" params? ")" block
 *    statement  → printStmt | ifStmt | whileStmt | forStmt
 *               | returnStmt | breakStmt | continueStmt
 *               | block | exprStmt
 *
 *  Expression precedence (lowest → highest):
 *    assignment → or → and → equality → comparison
 *    → term → factor → unary → call → primary
 *
 *  OOP Concepts:
 *    - Encapsulation (parsing state is private)
 *    - Composition (owns token list, uses ErrorReporter)
 *    - Factory pattern (creates AST nodes via make_unique)
 * ============================================================
 */
#pragma once

#include "Token.h"
#include "AST.h"
#include "ErrorReporter.h"
#include <vector>
#include <memory>

class Parser {
public:
    /**
     * Construct a Parser.
     * @param tokens   Token list from the Lexer
     * @param reporter Shared error reporter
     */
    Parser(const std::vector<Token>& tokens, ErrorReporter& reporter);

    /**
     * Parse the token stream into a list of statements (the program).
     * Returns the complete AST.
     */
    std::vector<StmtPtr> parse();

private:
    // ── State ──────────────────────────────────────────────
    const std::vector<Token>& tokens_;
    ErrorReporter&            reporter_;
    int                       current_ = 0;

    // ── Declaration Parsers ────────────────────────────────
    StmtPtr declaration();
    StmtPtr varDeclaration();
    StmtPtr functionDeclaration();

    // ── Statement Parsers ──────────────────────────────────
    StmtPtr statement();
    StmtPtr printStatement();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    StmtPtr forStatement();
    StmtPtr returnStatement();
    StmtPtr breakStatement();
    StmtPtr continueStatement();
    StmtPtr blockStatement();
    StmtPtr expressionStatement();

    // ── Expression Parsers (precedence climbing) ───────────
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr logicOr();
    ExprPtr logicAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call();
    ExprPtr primary();

    // ── Helper: finish parsing a call expression ───────────
    ExprPtr finishCall(ExprPtr callee);

    // ── Token Navigation ───────────────────────────────────
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool  isAtEnd() const;
    bool  check(TokenType type) const;
    bool  match(TokenType type);
    bool  match(std::initializer_list<TokenType> types);
    const Token& consume(TokenType type, const std::string& message);

    // ── Error Recovery ─────────────────────────────────────
    ParseError error(const Token& token, const std::string& message);
    void synchronize();
};
