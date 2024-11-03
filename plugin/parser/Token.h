#pragma once

#include "Common.h"

// Token kinds enumeration
enum TokenKind {
    #define TOKEN_DEF(NAME, X) NAME,
    #include "Token.def"
    #undef TOKEN_DEF
};


//impl std::less<T> for TokenKind and then text
// Token structure
struct Token : public std::less<Token> {
    static llvm::StringMap<TokenKind> toTokenKind;
    static auto toStr(TokenKind type);
    static llvm::Twine toTwine(TokenKind type);

    TokenKind kind;
    llvm::StringRef text;
    
    Token(TokenKind k);
    Token(StringRef s);
    Token(const char* s);
    Token(TokenKind k, llvm::StringRef t);
    
    //make std::out printable with text if it is not empty
    friend llvm::raw_ostream& operator<<(llvm::raw_ostream &OS, const Token &T);
    bool operator==(TokenKind k) const;
    bool operator==(llvm::StringRef t) const;
    const bool operator==(Token& other) const;
    bool operator<(const Token& other) const ;
    // cast to bool
    operator bool() const;
};

extern const Token EOF_TOKEN;

struct ParenOpenToken : public Token {
    ParenOpenToken(const char* cur, const char* pairParent);
    bool operator==(TokenKind k) const ;
    bool operator==(const Token& other) const ;
    bool operator==(const ParenOpenToken& other) const ;
};


struct KeywordToken : public Token {
private:
    // Use llvm::StringSet for better performance and simplicity
    static llvm::StringMap<TokenKind> keywords;
public:
    KeywordToken(TokenKind k);
    KeywordToken(llvm::StringRef t);
    static bool isKeyword(llvm::StringRef text);
    bool operator==(TokenKind k) const;
    bool operator==(const Token& other) const;
    bool operator==(const KeywordToken& other) const;
};

