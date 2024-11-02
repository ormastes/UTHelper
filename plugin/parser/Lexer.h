#pragma once

#include "CommonHeader.h"
#include "Token.h"

// Lexer class
class Lexer {
public:
    Lexer(llvm::StringRef input) : source(input), ptr(input.begin()) {}

    Token next() {
        while (ptr != source.end()) {
            char c = *ptr;

            CASSERT_LE(ptr, cur_decl_end);

            // Skip whitespace
            if (isWhitespace(c)) {
                ++ptr;
                continue;
            }

            // Handle comments
            if (c == '#') {
                // skip comments
                lexComment();
                continue;
            }

            // Handle identifiers and keywords
            if (isLetter(c) || c == '_' || c == '.') {
                DEBUG_LOG("lexIdentifierOrKeyword");
                return lexIdentifierOrKeyword();
            }

            // Handle numbers
            if (isDigit(c)) {
                DEBUG_LOG("lexNumber");
                return lexNumber();
            }

            // Handle string literals
            if (c == '"') {
                DEBUG_LOG("lexStringLiteral");
                return lexStringLiteral();
            }

            DEBUG_LOG("lexOperator: " << c);
            // Handle operators and punctuation
            switch (c) {
                case '(': ++ptr; CASSERT_LT(paren_open_idx, paren_pairloc_stack.size()); return ParenOpenToken(ptr, paren_pairloc_stack[paren_open_idx++]);
                case ')': ++ptr; return Token(TOK_RPAREN);
                case ',': ++ptr; return Token(TOK_COMMA);
                case ';': ++ptr; return Token(TOK_SEMICOLON);
                case '!': ++ptr; return Token(TOK_EXCLAMATION);
                case '=': if (peekNext() == '>') { ++ptr; ++ptr; return Token(TOK_ARROW); } else { ++ptr; return Token(TOK_EQUAL); }
                case '&': if (peekNext() == '&') { ++ptr; ++ptr; return Token(TOK_AMPAMP); } break;
                case '|': if (peekNext() == '|') { ++ptr; ++ptr; return Token(TOK_PIPEPIPE); } break;
                default: break;
            }

            // Unknown character
            CASSERT_MSG(false, "Unknown symbol '" << c << "'");
            //llvm::SmallString<1> unknownChar;
            //unknownChar.push_back(c);
            //++ptr;
            //return Token(TOK_UNKNOWN);
        }

        return Token(TOK_EOF);
    }
    const char* updateDeclarationEnd() {
        const char* org_loc = getCurrent();
        //llvm stack<cons char*> stack;
        paren_pairloc_stack.clear();
        paren_open_idx = 0;

        llvm::SmallVector<size_t, 10> temp_open_paren_loc_stack;


        const char* result = nullptr;
        while (ptr != source.end()) {
            char c = *ptr;

            // Handle comments
            if (c == '#') {
                // skip comments
                lexComment();
                continue;
            }
            size_t idx;
            switch (c) {
                case '(': 
                    temp_open_paren_loc_stack.push_back(paren_pairloc_stack.size());
                    paren_pairloc_stack.push_back(ptr);
                    break;
                case ')': 
                    idx = temp_open_paren_loc_stack.back();
                    temp_open_paren_loc_stack.pop_back();
                    CASSERT_LT(idx , paren_pairloc_stack.size());
                    paren_pairloc_stack[idx] = ptr;
                    break;
                case ';': 
                    CASSERT_MSG(temp_open_paren_loc_stack.empty(), "Unbalanced parenthesis");
                    result = ptr;
                    cur_decl_end = ptr;
                    restore(org_loc);
                    return result;
                default: break;
            }
            ++ptr;
        }
        restore(org_loc);
        return result;
    }
    const char* getCurrent() const { return ptr; }
    void restore(const char* pos) { ptr = pos; }

    void printContext(llvm::raw_ostream &OS) {
        OS << llvm::StringRef(((ptr - CONTENT_PRINT_SIZE) > source.data() ? (ptr - CONTENT_PRINT_SIZE) : source.data()), ptr - ((ptr - CONTENT_PRINT_SIZE) > source.data() ? (ptr - CONTENT_PRINT_SIZE) : source.data()));
        OS << "<<==";
        if (ptr >= source.end()) OS << llvm::StringRef(ptr + 1, ((ptr + CONTENT_PRINT_SIZE) < source.end()) ? CONTENT_PRINT_SIZE : (source.end() - ptr - 1));
    }

private:
    llvm::StringRef source;
    const char *ptr;
    llvm::SmallVector<const char*, 10> paren_pairloc_stack;
    size_t paren_open_idx = 0;
    const char *cur_decl_end = nullptr;


    char peekNext(size_t offset = 1) const {
        if ((ptr + offset) < source.end()) {
            return *(ptr + offset);
        } else {
            return '\0';
        }
    }

    bool isWhitespace(char c) const {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    bool isLetter(char c) const {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ':';
    }

    bool isDigit(char c) const {
        return c >= '0' && c <= '9';
    }

    Token lexIdentifierOrKeyword() {
        const char *start = ptr;
        while (ptr != source.end() && (isLetter(*ptr) || isDigit(*ptr) || *ptr == '_' || *ptr == '.')) {
            ++ptr;
        }
        llvm::StringRef text(start, ptr - start);

        // Check for keywords
        if (KeywordToken::isKeyword(text)) {
            DEBUG_LOG("lexIdentifierOrKeyword(KeywordToken): " << text);
            return KeywordToken(text);
        }
        DEBUG_LOG("lexIdentifierOrKeyword(TOK_IDENTIFIER): " << text);
        return Token(TOK_IDENTIFIER, text);
    }

    Token lexNumber() {
        const char *start = ptr;
        while (ptr != source.end() && isDigit(*ptr)) {
            ++ptr;
        }
        llvm::StringRef text(start, ptr - start);
        return Token(TOK_NUMBER, text);
    }

    Token lexStringLiteral() {
        const char *start = ptr;
        ++ptr; // Skip opening quote
        while (ptr != source.end() && *ptr != '"') {
            ++ptr;
        }
        if (ptr != source.end()) {
            ++ptr; // Skip closing quote
        }
        llvm::StringRef text(start, ptr - start);
        return Token(TOK_STRING_LITERAL, text);
    }

    Token lexComment() {
        const char *start = ptr;
        ++ptr; // Skip '#'
        while (ptr != source.end() && *ptr != '\n' && *ptr != '\r') {
            ++ptr;
        }
        llvm::StringRef text(start, ptr - start);
        return Token(TOK_COMMENT, text);
    }
};