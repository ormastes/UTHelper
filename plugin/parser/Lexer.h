#pragma once

#include "Token.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

// Lexer class
class Lexer {
public:
    Lexer(llvm::StringRef input);

    Token next();
    const char* updateDeclarationEnd();
    const char* getCurrent() const;
    void restore(const char* pos);
    void printContext(llvm::raw_ostream &OS);

private:
    llvm::StringRef source;
    const char *ptr;
    llvm::SmallVector<const char*, 10> paren_pairloc_stack;
    size_t paren_open_idx = 0;
    const char *cur_decl_end = nullptr;

    char peekNext(size_t offset = 1) const;
    bool isWhitespace(char c) const;
    bool isLetter(char c) const;
    bool isDigit(char c) const;

    Token lexIdentifierOrKeyword();
    Token lexNumber();
    Token lexStringLiteral();
    void lexComment();
};