/**
 * ============================================================
 *  Lexer.h — BharatLang Lexical Analyzer (Declaration)
 * ============================================================
 *  The Lexer (Tokenizer) is the FIRST phase of compilation.
 *  It reads raw source code characters and groups them into
 *  meaningful Tokens — keywords, identifiers, numbers,
 *  strings, operators, and punctuation.
 *
 *  OOP Concepts:
 *    - Encapsulation (private scanning state)
 *    - Single Responsibility Principle
 *    - Composition (uses ErrorReporter)
 * ============================================================
 */
#pragma once

#include "Token.h"
#include "ErrorReporter.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
public:
    /**
     * Construct a Lexer for the given source code.
     * @param source  The raw BharatLang source text
     * @param reporter  Shared error reporter for Hinglish messages
     */
    Lexer(const std::string& source, ErrorReporter& reporter);

    /**
     * Scan the entire source and produce a vector of Tokens.
     * The final token is always EOF_TOKEN.
     */
    std::vector<Token> tokenize();

private:
    // ── Source & Position Tracking ─────────────────────────
    std::string    source_;
    ErrorReporter& reporter_;
    std::vector<Token> tokens_;

    int start_   = 0;    // start of current lexeme
    int current_ = 0;    // current character position
    int line_    = 1;    // current line number
    int column_  = 1;    // current column number
    int startCol_= 1;    // column at start of current lexeme

    // ── Hinglish Keyword Table ─────────────────────────────
    static const std::unordered_map<std::string, TokenType> keywords_;

    // ── Core Scanning Methods ──────────────────────────────
    void scanToken();           // scan one token
    char advance();             // consume & return current char
    char peek() const;          // look at current char (no consume)
    char peekNext() const;      // look one ahead (no consume)
    bool match(char expected);  // consume if current == expected
    bool isAtEnd() const;       // are we done?

    // ── Specialized Scanners ───────────────────────────────
    void scanString();          // scan a "string literal"
    void scanNumber();          // scan integer or float literal
    void scanIdentifier();      // scan identifier or keyword
    void skipLineComment();     // skip // comment
    void skipBlockComment();    // skip /* ... */ comment

    // ── Helpers ────────────────────────────────────────────
    void addToken(TokenType type);
    void addToken(TokenType type,
                  std::variant<std::monostate, int, double, std::string, bool> literal);
    std::string currentLexeme() const;

    static bool isDigit(char c);
    static bool isAlpha(char c);
    static bool isAlphaNumeric(char c);
};
