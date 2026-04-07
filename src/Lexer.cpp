/**
 * ============================================================
 *  Lexer.cpp — BharatLang Lexical Analyzer (Implementation)
 * ============================================================
 *  Scans source code character-by-character, producing tokens.
 *
 *  How it works:
 *  1. advance() through the source one character at a time
 *  2. Decide what token the character starts
 *  3. Consume all characters in that token
 *  4. Create a Token with type, lexeme, line, column, literal
 *  5. Repeat until EOF
 * ============================================================
 */

#include "Lexer.h"
#include <stdexcept>

// ═══════════════════════════════════════════════════════════
//  HINGLISH KEYWORD TABLE
//  Maps keyword strings to their TokenType.
// ═══════════════════════════════════════════════════════════
const std::unordered_map<std::string, TokenType> Lexer::keywords_ = {
    {"agar",       TokenType::AGAR},
    {"warna",      TokenType::WARNA},
    {"warna_agar", TokenType::WARNA_AGAR},
    {"jabtak",     TokenType::JABTAK},
    {"har",        TokenType::HAR},
    {"likho",      TokenType::LIKHO},
    {"padho",      TokenType::PADHO},
    {"ank",        TokenType::ANK},
    {"dasham",     TokenType::DASHAM},
    {"shabd",      TokenType::SHABD},
    {"haan_naa",   TokenType::HAAN_NAA},
    {"sahi",       TokenType::SAHI},
    {"galat",      TokenType::GALAT},
    {"kaam",       TokenType::KAAM},
    {"lautao",     TokenType::LAUTAO},
    {"ruko",       TokenType::RUKO},
    {"agla",       TokenType::AGLA},
    {"shunya",     TokenType::SHUNYA},
    {"aur",        TokenType::AUR},
    {"ya",         TokenType::YA},
    {"nahi",       TokenType::NAHI},
    {"rakho",      TokenType::RAKHO},
};

// ═══════════════════════════════════════════════════════════
//  CONSTRUCTOR
// ═══════════════════════════════════════════════════════════
Lexer::Lexer(const std::string& source, ErrorReporter& reporter)
    : source_(source), reporter_(reporter) {}

// ═══════════════════════════════════════════════════════════
//  MAIN TOKENIZE METHOD
// ═══════════════════════════════════════════════════════════
std::vector<Token> Lexer::tokenize() {
    while (!isAtEnd()) {
        // Mark the start of the next lexeme
        start_    = current_;
        startCol_ = column_;
        scanToken();
    }

    // Add the final EOF token
    tokens_.emplace_back(TokenType::EOF_TOKEN, "", line_, column_);
    return tokens_;
}

// ═══════════════════════════════════════════════════════════
//  SCAN ONE TOKEN
//  The heart of the lexer — reads one character and decides
//  what token type it starts.
// ═══════════════════════════════════════════════════════════
void Lexer::scanToken() {
    char c = advance();

    switch (c) {
        // ── Single-character tokens ────────────────────────
        case '(': addToken(TokenType::LPAREN);    break;
        case ')': addToken(TokenType::RPAREN);    break;
        case '{': addToken(TokenType::LBRACE);    break;
        case '}': addToken(TokenType::RBRACE);    break;
        case '[': addToken(TokenType::LBRACKET);  break;
        case ']': addToken(TokenType::RBRACKET);  break;
        case ',': addToken(TokenType::COMMA);     break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '%': addToken(TokenType::MODULO);    break;

        // ── Two-character tokens (look-ahead needed) ──────
        case '+':
            addToken(match('=') ? TokenType::PLUS_ASSIGN : TokenType::PLUS);
            break;
        case '-':
            addToken(match('=') ? TokenType::MINUS_ASSIGN : TokenType::MINUS);
            break;
        case '*':
            addToken(match('=') ? TokenType::STAR_ASSIGN : TokenType::STAR);
            break;
        case '!':
            addToken(match('=') ? TokenType::NOT_EQUAL : TokenType::NAHI);
            break;
        case '=':
            addToken(match('=') ? TokenType::EQUAL : TokenType::ASSIGN);
            break;
        case '<':
            addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS);
            break;
        case '>':
            addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER);
            break;

        // ── Slash: division or comment ─────────────────────
        case '/':
            if (match('/')) {
                skipLineComment();
            } else if (match('*')) {
                skipBlockComment();
            } else if (match('=')) {
                addToken(TokenType::SLASH_ASSIGN);
            } else {
                addToken(TokenType::SLASH);
            }
            break;

        // ── Whitespace (skip) ──────────────────────────────
        case ' ':
        case '\t':
        case '\r':
            break;

        // ── Newline ────────────────────────────────────────
        case '\n':
            line_++;
            column_ = 1;
            break;

        // ── String literal ─────────────────────────────────
        case '"': scanString(); break;

        // ── Default: number, identifier, or error ──────────
        default:
            if (isDigit(c)) {
                scanNumber();
            } else if (isAlpha(c)) {
                scanIdentifier();
            } else {
                reporter_.error(line_, startCol_,
                    "Yeh character samajh nahi aaya: '" +
                    std::string(1, c) + "'");
            }
            break;
    }
}

// ═══════════════════════════════════════════════════════════
//  STRING SCANNER
//  Scans everything between "..." including escape sequences.
// ═══════════════════════════════════════════════════════════
void Lexer::scanString() {
    std::string value;

    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            line_++;
            column_ = 0;   // will be incremented by advance()
        }

        // Handle escape sequences: \n, \t, \\, \"
        if (peek() == '\\' && !isAtEnd()) {
            advance();   // consume '\'
            char escaped = advance();
            switch (escaped) {
                case 'n':  value += '\n'; break;
                case 't':  value += '\t'; break;
                case '\\': value += '\\'; break;
                case '"':  value += '"';  break;
                default:
                    value += '\\';
                    value += escaped;
                    break;
            }
        } else {
            value += advance();
        }
    }

    if (isAtEnd()) {
        reporter_.error(line_, startCol_,
            "String band karna bhool gaye! '\"' lagao end mein.");
        return;
    }

    advance();   // consume closing '"'
    addToken(TokenType::STRING_LIT, value);
}

// ═══════════════════════════════════════════════════════════
//  NUMBER SCANNER
//  Scans integer literals (42) and float literals (3.14).
// ═══════════════════════════════════════════════════════════
void Lexer::scanNumber() {
    // Consume all digits before the decimal point
    while (!isAtEnd() && isDigit(peek())) {
        advance();
    }

    // Check for decimal point (must be followed by a digit)
    bool isFloat = false;
    if (!isAtEnd() && peek() == '.' && isDigit(peekNext())) {
        isFloat = true;
        advance();   // consume '.'

        // Consume digits after decimal point
        while (!isAtEnd() && isDigit(peek())) {
            advance();
        }
    }

    std::string numStr = currentLexeme();
    if (isFloat) {
        addToken(TokenType::FLOAT_LIT, std::stod(numStr));
    } else {
        addToken(TokenType::INTEGER_LIT, std::stoi(numStr));
    }
}

// ═══════════════════════════════════════════════════════════
//  IDENTIFIER / KEYWORD SCANNER
//  Scans a word and checks if it's a reserved keyword.
//  Allows underscores in identifiers (e.g., warna_agar).
// ═══════════════════════════════════════════════════════════
void Lexer::scanIdentifier() {
    while (!isAtEnd() && isAlphaNumeric(peek())) {
        advance();
    }

    std::string text = currentLexeme();

    // Check keyword table
    auto it = keywords_.find(text);
    if (it != keywords_.end()) {
        TokenType kwType = it->second;

        // Special literals get actual literal values
        if (kwType == TokenType::SAHI) {
            addToken(kwType, true);
        } else if (kwType == TokenType::GALAT) {
            addToken(kwType, false);
        } else if (kwType == TokenType::SHUNYA) {
            addToken(kwType, std::monostate{});
        } else {
            addToken(kwType);
        }
    } else {
        addToken(TokenType::IDENTIFIER);
    }
}

// ═══════════════════════════════════════════════════════════
//  COMMENT SKIPPERS
// ═══════════════════════════════════════════════════════════
void Lexer::skipLineComment() {
    // Consume until end of line
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

void Lexer::skipBlockComment() {
    int depth = 1;   // support nested block comments

    while (!isAtEnd() && depth > 0) {
        if (peek() == '/' && peekNext() == '*') {
            advance(); advance();
            depth++;
        } else if (peek() == '*' && peekNext() == '/') {
            advance(); advance();
            depth--;
        } else {
            if (peek() == '\n') {
                line_++;
                column_ = 0;
            }
            advance();
        }
    }

    if (depth > 0) {
        reporter_.error(line_, startCol_,
            "Block comment band karna bhool gaye! '*/' lagao.");
    }
}

// ═══════════════════════════════════════════════════════════
//  HELPER METHODS
// ═══════════════════════════════════════════════════════════

char Lexer::advance() {
    char c = source_[current_++];
    column_++;
    return c;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= static_cast<int>(source_.size())) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[current_] != expected) return false;
    current_++;
    column_++;
    return true;
}

bool Lexer::isAtEnd() const {
    return current_ >= static_cast<int>(source_.size());
}

std::string Lexer::currentLexeme() const {
    return source_.substr(start_, current_ - start_);
}

void Lexer::addToken(TokenType type) {
    tokens_.emplace_back(type, currentLexeme(), line_, startCol_);
}

void Lexer::addToken(TokenType type,
                     std::variant<std::monostate, int, double, std::string, bool> literal) {
    tokens_.emplace_back(type, currentLexeme(), line_, startCol_, std::move(literal));
}

bool Lexer::isDigit(char c)        { return c >= '0' && c <= '9'; }
bool Lexer::isAlpha(char c)        { return (c >= 'a' && c <= 'z') ||
                                            (c >= 'A' && c <= 'Z') || c == '_'; }
bool Lexer::isAlphaNumeric(char c) { return isAlpha(c) || isDigit(c); }
