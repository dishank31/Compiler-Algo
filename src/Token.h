/**
 * ============================================================
 *  Token.h — BharatLang Token Definitions
 * ============================================================
 *  Defines every token type the lexer can produce and the
 *  Token struct that carries lexeme text, source location,
 *  and an optional literal value.
 * ============================================================
 */
#pragma once

#include <string>
#include <variant>
#include <ostream>

// ── Token Types ────────────────────────────────────────────
enum class TokenType {
    // ── Literals ───────────────────────────────────────────
    INTEGER_LIT,        // 42
    FLOAT_LIT,          // 3.14
    STRING_LIT,         // "namaste"

    // ── Identifier ─────────────────────────────────────────
    IDENTIFIER,         // user-defined names

    // ── Hinglish Keywords ──────────────────────────────────
    AGAR,               // if
    WARNA,              // else
    WARNA_AGAR,         // else if
    JABTAK,             // while
    HAR,                // for
    LIKHO,              // print
    PADHO,              // input / read
    ANK,                // int   type
    DASHAM,             // float type
    SHABD,              // string type
    HAAN_NAA,           // bool  type
    SAHI,               // true
    GALAT,              // false
    KAAM,               // function
    LAUTAO,             // return
    RUKO,               // break
    AGLA,               // continue
    SHUNYA,             // null / void
    AUR,                // && (logical AND)
    YA,                 // || (logical OR)
    NAHI,               // !  (logical NOT)
    RAKHO,              // let / var

    // ── Operators ──────────────────────────────────────────
    PLUS,               // +
    MINUS,              // -
    STAR,               // *
    SLASH,              // /
    MODULO,             // %

    ASSIGN,             // =
    EQUAL,              // ==
    NOT_EQUAL,          // !=

    LESS,               // <
    LESS_EQUAL,         // <=
    GREATER,            // >
    GREATER_EQUAL,      // >=

    PLUS_ASSIGN,        // +=
    MINUS_ASSIGN,       // -=
    STAR_ASSIGN,        // *=
    SLASH_ASSIGN,       // /=

    // ── Punctuation ────────────────────────────────────────
    LPAREN,             // (
    RPAREN,             // )
    LBRACE,             // {
    RBRACE,             // }
    LBRACKET,           // [
    RBRACKET,           // ]
    COMMA,              // ,
    SEMICOLON,          // ;

    // ── Special ────────────────────────────────────────────
    EOF_TOKEN,          // end of file
    ERROR_TOKEN         // lexer error
};

// ── Human-readable name for each token (debugging) ─────────
inline const char* tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::INTEGER_LIT:    return "INTEGER_LIT";
        case TokenType::FLOAT_LIT:      return "FLOAT_LIT";
        case TokenType::STRING_LIT:     return "STRING_LIT";
        case TokenType::IDENTIFIER:     return "IDENTIFIER";
        case TokenType::AGAR:           return "AGAR";
        case TokenType::WARNA:          return "WARNA";
        case TokenType::WARNA_AGAR:     return "WARNA_AGAR";
        case TokenType::JABTAK:         return "JABTAK";
        case TokenType::HAR:            return "HAR";
        case TokenType::LIKHO:          return "LIKHO";
        case TokenType::PADHO:          return "PADHO";
        case TokenType::ANK:            return "ANK";
        case TokenType::DASHAM:         return "DASHAM";
        case TokenType::SHABD:          return "SHABD";
        case TokenType::HAAN_NAA:       return "HAAN_NAA";
        case TokenType::SAHI:           return "SAHI";
        case TokenType::GALAT:          return "GALAT";
        case TokenType::KAAM:           return "KAAM";
        case TokenType::LAUTAO:         return "LAUTAO";
        case TokenType::RUKO:           return "RUKO";
        case TokenType::AGLA:           return "AGLA";
        case TokenType::SHUNYA:         return "SHUNYA";
        case TokenType::AUR:            return "AUR";
        case TokenType::YA:             return "YA";
        case TokenType::NAHI:           return "NAHI";
        case TokenType::RAKHO:          return "RAKHO";
        case TokenType::PLUS:           return "PLUS";
        case TokenType::MINUS:          return "MINUS";
        case TokenType::STAR:           return "STAR";
        case TokenType::SLASH:          return "SLASH";
        case TokenType::MODULO:         return "MODULO";
        case TokenType::ASSIGN:         return "ASSIGN";
        case TokenType::EQUAL:          return "EQUAL";
        case TokenType::NOT_EQUAL:      return "NOT_EQUAL";
        case TokenType::LESS:           return "LESS";
        case TokenType::LESS_EQUAL:     return "LESS_EQUAL";
        case TokenType::GREATER:        return "GREATER";
        case TokenType::GREATER_EQUAL:  return "GREATER_EQUAL";
        case TokenType::PLUS_ASSIGN:    return "PLUS_ASSIGN";
        case TokenType::MINUS_ASSIGN:   return "MINUS_ASSIGN";
        case TokenType::STAR_ASSIGN:    return "STAR_ASSIGN";
        case TokenType::SLASH_ASSIGN:   return "SLASH_ASSIGN";
        case TokenType::LPAREN:         return "LPAREN";
        case TokenType::RPAREN:         return "RPAREN";
        case TokenType::LBRACE:         return "LBRACE";
        case TokenType::RBRACE:         return "RBRACE";
        case TokenType::LBRACKET:       return "LBRACKET";
        case TokenType::RBRACKET:       return "RBRACKET";
        case TokenType::COMMA:          return "COMMA";
        case TokenType::SEMICOLON:      return "SEMICOLON";
        case TokenType::EOF_TOKEN:      return "EOF";
        case TokenType::ERROR_TOKEN:    return "ERROR";
    }
    return "UNKNOWN";
}

// ── Token Struct ───────────────────────────────────────────
//  Holds one lexical token produced by the Lexer.
//  `literal` stores the parsed value for numbers and strings.
struct Token {
    TokenType   type;
    std::string lexeme;     // raw text from source
    int         line;       // 1-based line number
    int         column;     // 1-based column number

    // Literal value: monostate means "no literal"
    std::variant<std::monostate, int, double, std::string, bool> literal;

    // Constructor without literal
    Token(TokenType type, std::string lexeme, int line, int column)
        : type(type), lexeme(std::move(lexeme)),
          line(line), column(column), literal(std::monostate{}) {}

    // Constructor with literal value
    Token(TokenType type, std::string lexeme, int line, int column,
          std::variant<std::monostate, int, double, std::string, bool> literal)
        : type(type), lexeme(std::move(lexeme)),
          line(line), column(column), literal(std::move(literal)) {}
};

// ── Stream operator for debugging ──────────────────────────
inline std::ostream& operator<<(std::ostream& os, const Token& tok) {
    os << "[" << tokenTypeName(tok.type) << " '" << tok.lexeme
       << "' L:" << tok.line << " C:" << tok.column << "]";
    return os;
}
