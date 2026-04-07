/**
 * ============================================================
 *  main.cpp — BharatLang Compiler Entry Point
 * ============================================================
 *  Supports two modes:
 *    1. File mode:   bharatlang program.bl
 *    2. REPL mode:   bharatlang  (interactive shell)
 *
 *  Debug flags:
 *    --tokens   Dump the token list
 *    --ast      Dump the AST (pretty-printed)
 *    --ir       Dump Three-Address Code
 *
 *  Compilation Pipeline:
 *    Source Code → Lexer → Tokens → Parser → AST
 *      → Interpreter (execute)
 *      → IRGenerator (optional TAC output)
 * ============================================================
 */

#include "Lexer.h"
#include "Parser.h"
#include "Interpreter.h"
#include "IRGenerator.h"
#include "ErrorReporter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// ── ANSI Color Helpers ─────────────────────────────────────
const std::string RESET   = "\033[0m";
const std::string BOLD    = "\033[1m";
const std::string DIM     = "\033[2m";
const std::string CYAN    = "\033[36m";
const std::string GREEN   = "\033[32m";
const std::string YELLOW  = "\033[33m";
const std::string MAGENTA = "\033[35m";
const std::string RED     = "\033[31m";

// ── Print the BharatLang banner ────────────────────────────
void printBanner() {
    std::cout << BOLD << CYAN;
    std::cout << R"(
  ____  _                     _   _                       
 | __ )| |__   __ _ _ __ __ _| |_| |    __ _ _ __   __ _ 
 |  _ \| '_ \ / _` | '__/ _` | __| |   / _` | '_ \ / _` |
 | |_) | | | | (_| | | | (_| | |_| |__| (_| | | | | (_| |
 |____/|_| |_|\__,_|_|  \__,_|\__|_____\__,_|_| |_|\__, |
                                                     |___/ 
)" << RESET;
    std::cout << BOLD << GREEN
              << "  🇮🇳 Hinglish Programming Language Compiler v1.0"
              << RESET << "\n";
    std::cout << DIM
              << "  Type 'niklo' to exit, 'madad' for help.\n"
              << RESET << std::endl;
}

// ── Print help ─────────────────────────────────────────────
void printHelp() {
    std::cout << BOLD << "\n📖 BharatLang Madad (Help):\n\n" << RESET;
    std::cout << "  " << YELLOW << "Usage:" << RESET << "\n";
    std::cout << "    bharatlang                    → REPL mode\n";
    std::cout << "    bharatlang <file.bl>           → Run a file\n";
    std::cout << "    bharatlang <file.bl> --tokens  → Show tokens\n";
    std::cout << "    bharatlang <file.bl> --ir      → Show IR code\n\n";
    std::cout << "  " << YELLOW << "Keywords:" << RESET << "\n";
    std::cout << "    rakho   → variable declare   agar    → if\n";
    std::cout << "    warna   → else               jabtak  → while\n";
    std::cout << "    har     → for                kaam    → function\n";
    std::cout << "    likho   → print              padho   → input\n";
    std::cout << "    lautao  → return             ruko    → break\n";
    std::cout << "    agla    → continue           sahi    → true\n";
    std::cout << "    galat   → false              shunya  → null\n";
    std::cout << "    aur     → and                ya      → or\n";
    std::cout << "    nahi    → not\n\n";
    std::cout << "  " << YELLOW << "Types:" << RESET << "\n";
    std::cout << "    ank     → int                dasham  → float\n";
    std::cout << "    shabd   → string             haan_naa→ bool\n\n";
    std::cout << "  " << YELLOW << "Example:" << RESET << "\n";
    std::cout << "    rakho naam = padho(\"Naam batao: \");\n";
    std::cout << "    likho(\"Namaste, \" + naam + \"!\");\n\n";
}

// ── Dump tokens ────────────────────────────────────────────
void dumpTokens(const std::vector<Token>& tokens) {
    std::cout << BOLD << "\n🔤 Token List:\n" << RESET;
    std::cout << "  " << DIM
              << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
              << RESET;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::EOF_TOKEN) continue;
        std::cout << "  " << CYAN << tokenTypeName(tok.type) << RESET
                  << "\t'" << GREEN << tok.lexeme << RESET << "'"
                  << "\t" << DIM << "L:" << tok.line
                  << " C:" << tok.column << RESET << "\n";
    }
    std::cout << "  " << DIM
              << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
              << RESET << std::endl;
}

// ── Run source code through the pipeline ───────────────────
void run(const std::string& source, Interpreter& interpreter,
         ErrorReporter& reporter, bool showTokens, bool showIR) {

    // ── Phase 1: Lexical Analysis ──────────────────────────
    Lexer lexer(source, reporter);
    std::vector<Token> tokens = lexer.tokenize();

    if (showTokens) {
        dumpTokens(tokens);
    }

    if (reporter.hadError()) return;

    // ── Phase 2: Parsing (Syntax Analysis) ─────────────────
    Parser parser(tokens, reporter);
    std::vector<StmtPtr> statements = parser.parse();

    if (reporter.hadError()) return;

    // ── Phase 3 (optional): IR Code Generation ─────────────
    if (showIR) {
        IRGenerator irGen;
        irGen.generate(statements);
        std::cout << MAGENTA << irGen.toString() << RESET;
    }

    // ── Phase 4: Interpretation (Execution) ────────────────
    interpreter.interpret(statements);
}

// ── Run a .bl file ─────────────────────────────────────────
int runFile(const std::string& filename, bool showTokens, bool showIR) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << RED << "❌ File nahi khul rahi: '" << filename << "'"
                  << RESET << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    ErrorReporter reporter;
    Interpreter interpreter(reporter);

    std::cout << BOLD << GREEN << "🚀 Running: " << RESET
              << filename << "\n" << DIM
              << "───────────────────────────────────────\n"
              << RESET;

    run(source, interpreter, reporter, showTokens, showIR);

    if (reporter.hadError()) {
        std::cout << "\n" << RED << BOLD
                  << "❌ " << reporter.errorCount()
                  << " galti(yan) mili!" << RESET << std::endl;
        return 1;
    }

    std::cout << DIM
              << "───────────────────────────────────────\n"
              << RESET << GREEN << "✅ Program safal!" << RESET
              << std::endl;
    return 0;
}

// ── Interactive REPL ───────────────────────────────────────
void runREPL() {
    printBanner();

    ErrorReporter reporter;
    Interpreter interpreter(reporter);
    std::string line;

    while (true) {
        std::cout << BOLD << CYAN << "भारत>>> " << RESET;

        if (!std::getline(std::cin, line)) {
            std::cout << std::endl;
            break;
        }

        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);

        // Special REPL commands
        if (line == "niklo" || line == "exit" || line == "quit") {
            std::cout << GREEN << "👋 Alvida! Phir milenge!\n"
                      << RESET;
            break;
        }
        if (line == "madad" || line == "help") {
            printHelp();
            continue;
        }

        reporter.reset();
        run(line, interpreter, reporter, false, false);
    }
}

// ═══════════════════════════════════════════════════════════
//  MAIN
// ═══════════════════════════════════════════════════════════
int main(int argc, char* argv[]) {
    // No arguments → REPL mode
    if (argc == 1) {
        runREPL();
        return 0;
    }

    // Parse command-line arguments
    std::string filename;
    bool showTokens = false;
    bool showIR     = false;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--tokens") {
            showTokens = true;
        } else if (arg == "--ir") {
            showIR = true;
        } else if (arg == "--help" || arg == "-h" || arg == "madad") {
            printHelp();
            return 0;
        } else {
            filename = arg;
        }
    }

    if (filename.empty()) {
        std::cerr << RED << "❌ Koi file nahi di! Use: bharatlang <file.bl>"
                  << RESET << std::endl;
        return 1;
    }

    return runFile(filename, showTokens, showIR);
}
