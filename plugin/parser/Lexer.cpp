#include "Lexer.h"

#include "Common.h"

#undef CONTENT_PRINT_SIZE
#define CONTENT_PRINT_SIZE 50

Lexer::Lexer(llvm::StringRef input)
    : source(input), ptr(input.begin()) {}

Token Lexer::next() {
    while (ptr != source.end()) {
        char c = *ptr;

        // Skip whitespace
        if (isWhitespace(c)) {
            ++ptr;
            continue;
        }

        // Handle comments
        if (c == '#') {
            // Skip comments
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
            case '(':
                ++ptr;
                if (paren_open_idx < paren_pairloc_stack.size()) {
                    return Token(TOK_LPAREN);
                }
                break;
            case ')':
                ++ptr;
                return Token(TOK_RPAREN);
            case ',':
                ++ptr;
                return Token(TOK_COMMA);
            case ';':
                ++ptr;
                return Token(TOK_SEMICOLON);
            case '!':
                ++ptr;
                return Token(TOK_EXCLAMATION);
            case '=':
                if (peekNext() == '>') {
                    ++ptr;
                    ++ptr;
                    return Token(TOK_ARROW);
                } else {
                    ++ptr;
                    return Token(TOK_EQUAL);
                }
            case '&':
                if (peekNext() == '&') {
                    ++ptr;
                    ++ptr;
                    return Token(TOK_AMPAMP);
                }
                break;
            case '|':
                if (peekNext() == '|') {
                    ++ptr;
                    ++ptr;
                    return Token(TOK_PIPEPIPE);
                }
                break;
            default:
                break;
        }

        // Unknown character
        DEBUG_LOG("Unknown symbol '" << c << "'");
        ++ptr;
        return Token(TOK_UNKNOWN, llvm::StringRef(&c, 1));
    }

    return Token(TOK_EOF);
}

const char* Lexer::updateDeclarationEnd() {
    const char* org_loc = getCurrent();
    paren_pairloc_stack.clear();
    paren_open_idx = 0;

    llvm::SmallVector<size_t, 10> temp_open_paren_loc_stack;
    const char* result = nullptr;
    while (ptr != source.end()) {
        char c = *ptr;

        // Handle comments
        if (c == '#') {
            // Skip comments
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
                if (!temp_open_paren_loc_stack.empty()) {
                    idx = temp_open_paren_loc_stack.back();
                    temp_open_paren_loc_stack.pop_back();
                    if (idx < paren_pairloc_stack.size()) {
                        paren_pairloc_stack[idx] = ptr;
                    }
                }
                break;
            case ';':
                if (temp_open_paren_loc_stack.empty()) {
                    result = ptr;
                    cur_decl_end = ptr;
                    restore(org_loc);
                    return result;
                }
                break;
            default:
                break;
        }
        ++ptr;
    }
    restore(org_loc);
    return result;
}

const char* Lexer::getCurrent() const {
    return ptr;
}

void Lexer::restore(const char* pos) {
    ptr = pos;
}

void Lexer::printContext(llvm::raw_ostream &OS) {
    const char* start = (ptr - CONTENT_PRINT_SIZE) > source.data() ? (ptr - CONTENT_PRINT_SIZE) : source.data();
    OS << llvm::StringRef(start, ptr - start);
    OS << "<<==";
    if (ptr < source.end()) {
        const char* end = (ptr + CONTENT_PRINT_SIZE) < source.end() ? (ptr + CONTENT_PRINT_SIZE) : source.end();
        OS << llvm::StringRef(ptr + 1, end - ptr - 1);
    }
}

char Lexer::peekNext(size_t offset) const {
    if ((ptr + offset) < source.end()) {
        return *(ptr + offset);
    } else {
        return '\0';
    }
}

bool Lexer::isWhitespace(char c) const {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool Lexer::isLetter(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ':';
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

Token Lexer::lexIdentifierOrKeyword() {
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

Token Lexer::lexNumber() {
    const char *start = ptr;
    while (ptr != source.end() && isDigit(*ptr)) {
        ++ptr;
    }
    llvm::StringRef text(start, ptr - start);
    return Token(TOK_NUMBER, text);
}

Token Lexer::lexStringLiteral() {
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

void Lexer::lexComment() {
    ++ptr; // Skip '#'
    while (ptr != source.end() && *ptr != '\n' && *ptr != '\r') {
        ++ptr;
    }
}