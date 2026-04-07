/**
 * ============================================================
 *  Parser.cpp — BharatLang Recursive Descent Parser
 * ============================================================
 *  Implements a top-down recursive descent parser with
 *  precedence climbing for expressions.
 *
 *  Each grammar rule becomes a method:
 *    program()    → loops over declaration()
 *    expression() → calls assignment() → ... → primary()
 *
 *  Error recovery: on a ParseError, synchronize() skips
 *  tokens until a likely statement boundary, then resumes.
 * ============================================================
 */

#include "Parser.h"
#include <iostream>

// ═══════════════════════════════════════════════════════════
//  CONSTRUCTOR
// ═══════════════════════════════════════════════════════════
Parser::Parser(const std::vector<Token>& tokens, ErrorReporter& reporter)
    : tokens_(tokens), reporter_(reporter) {}

// ═══════════════════════════════════════════════════════════
//  PARSE — Entry Point
// ═══════════════════════════════════════════════════════════
std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;

    while (!isAtEnd()) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }

    return statements;
}

// ═══════════════════════════════════════════════════════════
//  DECLARATIONS
// ═══════════════════════════════════════════════════════════

StmtPtr Parser::declaration() {
    try {
        if (match(TokenType::RAKHO))  return varDeclaration();
        if (match(TokenType::KAAM))   return functionDeclaration();
        return statement();
    } catch (const ParseError&) {
        synchronize();
        return nullptr;
    }
}

/**
 * varDecl → "rakho" IDENTIFIER ("=" expression)? ";"
 * Example: rakho x = 42;
 */
StmtPtr Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER,
        "Variable ka naam dena bhool gaye 'rakho' ke baad!");

    ExprPtr initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON,
        "';' lagana bhool gaye variable declaration ke baad!");
    return std::make_unique<VarDeclStmt>(std::move(name), std::move(initializer));
}

/**
 * funDecl → "kaam" IDENTIFIER "(" parameters? ")" "{" body "}"
 * Example: kaam factorial(n) { lautao n * factorial(n-1); }
 */
StmtPtr Parser::functionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER,
        "Function ka naam dena bhool gaye 'kaam' ke baad!");

    consume(TokenType::LPAREN,
        "'(' chahiye function name ke baad!");

    // Parse parameter list
    std::vector<Token> params;
    if (!check(TokenType::RPAREN)) {
        do {
            if (params.size() >= 255) {
                error(peek(), "255 se zyada parameters nahi ho sakte bhai!");
            }
            params.push_back(
                consume(TokenType::IDENTIFIER, "Parameter ka naam do!")
            );
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "')' lagao parameters ke baad!");
    consume(TokenType::LBRACE, "'{' se function body shuru karo!");

    // Parse the function body
    std::vector<StmtPtr> body;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = declaration();
        if (stmt) body.push_back(std::move(stmt));
    }

    consume(TokenType::RBRACE, "'}' se function body band karo!");

    return std::make_unique<FunctionStmt>(
        std::move(name), std::move(params), std::move(body));
}

// ═══════════════════════════════════════════════════════════
//  STATEMENTS
// ═══════════════════════════════════════════════════════════

StmtPtr Parser::statement() {
    if (match(TokenType::LIKHO))   return printStatement();
    if (match(TokenType::AGAR))    return ifStatement();
    if (match(TokenType::JABTAK))  return whileStatement();
    if (match(TokenType::HAR))     return forStatement();
    if (match(TokenType::LAUTAO))  return returnStatement();
    if (match(TokenType::RUKO))    return breakStatement();
    if (match(TokenType::AGLA))    return continueStatement();
    if (match(TokenType::LBRACE))  return blockStatement();
    return expressionStatement();
}

/**
 * printStmt → "likho" "(" expression ("," expression)* ")" ";"
 * Example: likho("Hello", naam, umar);
 */
StmtPtr Parser::printStatement() {
    consume(TokenType::LPAREN, "'(' chahiye 'likho' ke baad!");

    std::vector<ExprPtr> expressions;
    if (!check(TokenType::RPAREN)) {
        do {
            expressions.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "')' lagao 'likho' ke baad!");
    consume(TokenType::SEMICOLON, "';' lagao 'likho(...)' ke baad!");

    return std::make_unique<PrintStmt>(std::move(expressions));
}

/**
 * ifStmt → "agar" "(" expr ")" block
 *          ("warna_agar" "(" expr ")" block)*
 *          ("warna" block)?
 */
StmtPtr Parser::ifStatement() {
    consume(TokenType::LPAREN, "'(' chahiye 'agar' ke baad!");
    ExprPtr condition = expression();
    consume(TokenType::RPAREN, "')' lagao condition ke baad!");

    consume(TokenType::LBRACE, "'{' se agar block shuru karo!");
    StmtPtr thenBranch = blockStatement();

    // Parse warna_agar (else-if) chains
    std::vector<ExprPtr> elifConditions;
    std::vector<StmtPtr> elifBranches;

    while (match(TokenType::WARNA_AGAR)) {
        consume(TokenType::LPAREN, "'(' chahiye 'warna_agar' ke baad!");
        elifConditions.push_back(expression());
        consume(TokenType::RPAREN, "')' lagao condition ke baad!");
        consume(TokenType::LBRACE, "'{' se warna_agar block shuru karo!");
        elifBranches.push_back(blockStatement());
    }

    // Parse warna (else)
    StmtPtr elseBranch = nullptr;
    if (match(TokenType::WARNA)) {
        consume(TokenType::LBRACE, "'{' se warna block shuru karo!");
        elseBranch = blockStatement();
    }

    return std::make_unique<IfStmt>(
        std::move(condition), std::move(thenBranch),
        std::move(elifConditions), std::move(elifBranches),
        std::move(elseBranch));
}

/**
 * whileStmt → "jabtak" "(" expression ")" block
 * Example: jabtak (x < 10) { x = x + 1; }
 */
StmtPtr Parser::whileStatement() {
    consume(TokenType::LPAREN, "'(' chahiye 'jabtak' ke baad!");
    ExprPtr condition = expression();
    consume(TokenType::RPAREN, "')' lagao condition ke baad!");

    consume(TokenType::LBRACE, "'{' se jabtak loop body shuru karo!");
    StmtPtr body = blockStatement();

    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

/**
 * forStmt → "har" "(" (varDecl | exprStmt | ";")
 *                      expression? ";"
 *                      expression? ")" block
 * Example: har (rakho i = 0; i < 10; i = i + 1) { ... }
 */
StmtPtr Parser::forStatement() {
    consume(TokenType::LPAREN, "'(' chahiye 'har' ke baad!");

    // Initializer
    StmtPtr initializer;
    if (match(TokenType::SEMICOLON)) {
        initializer = nullptr;
    } else if (match(TokenType::RAKHO)) {
        initializer = varDeclaration();
    } else {
        initializer = expressionStatement();
    }

    // Condition
    ExprPtr condition = nullptr;
    if (!check(TokenType::SEMICOLON)) {
        condition = expression();
    }
    consume(TokenType::SEMICOLON, "';' chahiye loop condition ke baad!");

    // Increment
    ExprPtr increment = nullptr;
    if (!check(TokenType::RPAREN)) {
        increment = expression();
    }
    consume(TokenType::RPAREN, "')' lagao 'har' ke baad!");

    consume(TokenType::LBRACE, "'{' se har loop body shuru karo!");
    StmtPtr body = blockStatement();

    return std::make_unique<ForStmt>(
        std::move(initializer), std::move(condition),
        std::move(increment), std::move(body));
}

/**
 * returnStmt → "lautao" expression? ";"
 */
StmtPtr Parser::returnStatement() {
    Token keyword = previous();
    ExprPtr value = nullptr;

    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }

    consume(TokenType::SEMICOLON, "';' lagao 'lautao' ke baad!");
    return std::make_unique<ReturnStmt>(std::move(keyword), std::move(value));
}

/**
 * breakStmt → "ruko" ";"
 */
StmtPtr Parser::breakStatement() {
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "';' lagao 'ruko' ke baad!");
    return std::make_unique<BreakStmt>(std::move(keyword));
}

/**
 * continueStmt → "agla" ";"
 */
StmtPtr Parser::continueStatement() {
    Token keyword = previous();
    consume(TokenType::SEMICOLON, "';' lagao 'agla' ke baad!");
    return std::make_unique<ContinueStmt>(std::move(keyword));
}

/**
 * block → "{" declaration* "}"
 * The opening "{" is already consumed by the caller.
 */
StmtPtr Parser::blockStatement() {
    std::vector<StmtPtr> statements;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto stmt = declaration();
        if (stmt) statements.push_back(std::move(stmt));
    }

    consume(TokenType::RBRACE, "'}' se block band karo!");
    return std::make_unique<BlockStmt>(std::move(statements));
}

/**
 * exprStmt → expression ";"
 */
StmtPtr Parser::expressionStatement() {
    ExprPtr expr = expression();
    consume(TokenType::SEMICOLON,
        "';' lagana bhool gaye statement ke end mein!");
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

// ═══════════════════════════════════════════════════════════
//  EXPRESSION PARSERS (Precedence Climbing)
//  Lowest precedence at top, highest at bottom.
// ═══════════════════════════════════════════════════════════

ExprPtr Parser::expression() {
    return assignment();
}

/**
 * assignment → IDENTIFIER "=" assignment | logicOr
 * Right-to-left associative.
 */
ExprPtr Parser::assignment() {
    ExprPtr expr = logicOr();

    if (match(TokenType::ASSIGN)) {
        Token equals = previous();
        ExprPtr value = assignment();   // right-to-left recursion

        // Check that the left side is a valid assignment target
        if (auto* varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = varExpr->name;
            return std::make_unique<AssignExpr>(std::move(name), std::move(value));
        }

        error(equals, "Yeh koi valid assignment target nahi hai!");
    }

    // Handle compound assignment: +=, -=, *=, /=
    if (match({TokenType::PLUS_ASSIGN, TokenType::MINUS_ASSIGN,
               TokenType::STAR_ASSIGN, TokenType::SLASH_ASSIGN})) {
        Token op = previous();
        ExprPtr value = assignment();

        if (auto* varExpr = dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = varExpr->name;

            // Determine the base operator
            TokenType baseOp;
            switch (op.type) {
                case TokenType::PLUS_ASSIGN:  baseOp = TokenType::PLUS;  break;
                case TokenType::MINUS_ASSIGN: baseOp = TokenType::MINUS; break;
                case TokenType::STAR_ASSIGN:  baseOp = TokenType::STAR;  break;
                case TokenType::SLASH_ASSIGN: baseOp = TokenType::SLASH; break;
                default: baseOp = TokenType::PLUS; break;
            }

            // x += 5  →  x = x + 5  (desugar)
            Token opToken(baseOp, op.lexeme.substr(0, 1), op.line, op.column);
            auto varRef = std::make_unique<VariableExpr>(name);
            auto binExpr = std::make_unique<BinaryExpr>(
                std::move(varRef), std::move(opToken), std::move(value));

            return std::make_unique<AssignExpr>(std::move(name), std::move(binExpr));
        }

        error(op, "Yeh koi valid assignment target nahi hai!");
    }

    return expr;
}

/**
 * logicOr → logicAnd ("ya" logicAnd)*
 * Short-circuit: if left is sahi, skip right.
 */
ExprPtr Parser::logicOr() {
    ExprPtr expr = logicAnd();

    while (match(TokenType::YA)) {
        Token op    = previous();
        ExprPtr right = logicAnd();
        expr = std::make_unique<LogicalExpr>(
            std::move(expr), std::move(op), std::move(right));
    }

    return expr;
}

/**
 * logicAnd → equality ("aur" equality)*
 * Short-circuit: if left is galat, skip right.
 */
ExprPtr Parser::logicAnd() {
    ExprPtr expr = equality();

    while (match(TokenType::AUR)) {
        Token op    = previous();
        ExprPtr right = equality();
        expr = std::make_unique<LogicalExpr>(
            std::move(expr), std::move(op), std::move(right));
    }

    return expr;
}

/**
 * equality → comparison (("==" | "!=") comparison)*
 */
ExprPtr Parser::equality() {
    ExprPtr expr = comparison();

    while (match({TokenType::EQUAL, TokenType::NOT_EQUAL})) {
        Token op    = previous();
        ExprPtr right = comparison();
        expr = std::make_unique<BinaryExpr>(
            std::move(expr), std::move(op), std::move(right));
    }

    return expr;
}

/**
 * comparison → term (("<" | "<=" | ">" | ">=") term)*
 */
ExprPtr Parser::comparison() {
    ExprPtr expr = term();

    while (match({TokenType::LESS, TokenType::LESS_EQUAL,
                  TokenType::GREATER, TokenType::GREATER_EQUAL})) {
        Token op    = previous();
        ExprPtr right = term();
        expr = std::make_unique<BinaryExpr>(
            std::move(expr), std::move(op), std::move(right));
    }

    return expr;
}

/**
 * term → factor (("+" | "-") factor)*
 */
ExprPtr Parser::term() {
    ExprPtr expr = factor();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        Token op    = previous();
        ExprPtr right = factor();
        expr = std::make_unique<BinaryExpr>(
            std::move(expr), std::move(op), std::move(right));
    }

    return expr;
}

/**
 * factor → unary (("*" | "/" | "%") unary)*
 */
ExprPtr Parser::factor() {
    ExprPtr expr = unary();

    while (match({TokenType::STAR, TokenType::SLASH, TokenType::MODULO})) {
        Token op    = previous();
        ExprPtr right = unary();
        expr = std::make_unique<BinaryExpr>(
            std::move(expr), std::move(op), std::move(right));
    }

    return expr;
}

/**
 * unary → ("-" | "nahi") unary | call
 */
ExprPtr Parser::unary() {
    if (match({TokenType::MINUS, TokenType::NAHI})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(std::move(op), std::move(right));
    }

    return call();
}

/**
 * call → primary ("(" arguments? ")")*
 * Handles chained function calls: f(a)(b)
 */
ExprPtr Parser::call() {
    ExprPtr expr = primary();

    while (true) {
        if (match(TokenType::LPAREN)) {
            expr = finishCall(std::move(expr));
        } else {
            break;
        }
    }

    return expr;
}

ExprPtr Parser::finishCall(ExprPtr callee) {
    std::vector<ExprPtr> arguments;

    if (!check(TokenType::RPAREN)) {
        do {
            if (arguments.size() >= 255) {
                error(peek(), "255 se zyada arguments nahi ho sakte!");
            }
            arguments.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    Token paren = consume(TokenType::RPAREN,
        "')' lagao function call ke baad!");

    return std::make_unique<CallExpr>(
        std::move(callee), std::move(paren), std::move(arguments));
}

/**
 * primary → INTEGER | FLOAT | STRING | "sahi" | "galat"
 *         | "shunya" | IDENTIFIER | "(" expression ")"
 *         | "padho" "(" expression? ")"
 */
ExprPtr Parser::primary() {
    // Integer literal
    if (match(TokenType::INTEGER_LIT)) {
        return std::make_unique<LiteralExpr>(
            std::get<int>(previous().literal));
    }

    // Float literal
    if (match(TokenType::FLOAT_LIT)) {
        return std::make_unique<LiteralExpr>(
            std::get<double>(previous().literal));
    }

    // String literal
    if (match(TokenType::STRING_LIT)) {
        return std::make_unique<LiteralExpr>(
            std::get<std::string>(previous().literal));
    }

    // Boolean literals
    if (match(TokenType::SAHI)) {
        return std::make_unique<LiteralExpr>(true);
    }
    if (match(TokenType::GALAT)) {
        return std::make_unique<LiteralExpr>(false);
    }

    // Null literal
    if (match(TokenType::SHUNYA)) {
        return std::make_unique<LiteralExpr>(std::monostate{});
    }

    // padho() input expression
    if (match(TokenType::PADHO)) {
        consume(TokenType::LPAREN, "'(' chahiye 'padho' ke baad!");

        ExprPtr prompt = nullptr;
        if (!check(TokenType::RPAREN)) {
            prompt = expression();
        }

        consume(TokenType::RPAREN, "')' lagao 'padho(...)' ke baad!");
        return std::make_unique<InputExpr>(std::move(prompt));
    }

    // Identifier (variable reference)
    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<VariableExpr>(previous());
    }

    // Grouping: ( expression )
    if (match(TokenType::LPAREN)) {
        ExprPtr expr = expression();
        consume(TokenType::RPAREN,
            "')' lagana bhool gaye expression ke baad!");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    throw error(peek(),
        "Yeh expression samajh nahi aaya: '" + peek().lexeme + "'");
}

// ═══════════════════════════════════════════════════════════
//  TOKEN NAVIGATION
// ═══════════════════════════════════════════════════════════

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

const Token& Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::EOF_TOKEN;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

// ═══════════════════════════════════════════════════════════
//  ERROR HANDLING & RECOVERY
// ═══════════════════════════════════════════════════════════

ParseError Parser::error(const Token& token, const std::string& message) {
    reporter_.error(token.line, token.column, message);
    return ParseError(message, token.line, token.column);
}

/**
 * Panic-mode error recovery.
 * Discards tokens until we find something that looks like
 * the start of a new statement (a "synchronization point").
 */
void Parser::synchronize() {
    advance();

    while (!isAtEnd()) {
        // Semicolons end statements — resume after one
        if (previous().type == TokenType::SEMICOLON) return;

        // These keywords start new statements
        switch (peek().type) {
            case TokenType::KAAM:
            case TokenType::RAKHO:
            case TokenType::AGAR:
            case TokenType::JABTAK:
            case TokenType::HAR:
            case TokenType::LIKHO:
            case TokenType::LAUTAO:
            case TokenType::RUKO:
            case TokenType::AGLA:
                return;
            default:
                break;
        }

        advance();
    }
}
