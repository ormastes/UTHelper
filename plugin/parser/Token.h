#pragma once

#include "CommonHeader.h"

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
    Token(StringRef s) ;
    Token(const char* s) : Token(llvm::StringRef(s)) {}
    Token(TokenKind k, llvm::StringRef t) : kind(k), text(t) {}
    
    //make std::out printable with text if it is not empty
    friend llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Token &T) {
        OS << toTwine(T.kind) << ": " << T.kind << " ";
        if (!T.text.empty()) {
            OS << " (" << T.text << ")";
        }
        return OS;
    }

    bool operator==(TokenKind k) const { return kind == k; }

    bool operator==(llvm::StringRef t) const { return text == t; }

    const bool operator==(Token& other) const { return kind == other.kind && text == other.text; }

    bool operator<(const Token& other) const {
        return kind < other.kind || (kind == other.kind && text < other.text);
    }
    // cast to bool
    operator bool() const { return kind != TOK_EOF; }
};


extern const Token EOF_TOKEN;


struct ParenOpenToken : public Token {
    ParenOpenToken(const char* cur, const char* pairParent) : Token(TOK_LPAREN, llvm::StringRef(cur, pairParent-cur)) {}

    bool operator==(TokenKind k) const { return kind == k; }
    bool operator==(const Token& other) const {
        return kind == other.kind && text == other.text;
    }
    bool operator==(const ParenOpenToken& other) const {
        return kind == other.kind && text == other.text;
    }

};


struct KeywordToken : public Token {
private:
    // Use llvm::StringSet for better performance and simplicity
    static llvm::StringMap<TokenKind> keywords;
public:
    KeywordToken(TokenKind k) : Token(k){}
    KeywordToken(llvm::StringRef t) : Token(keywords.lookup(t), t) {}
    static bool isKeyword(llvm::StringRef text) {
        return keywords.count(text);
    }
    bool operator==(TokenKind k) const { return kind == k; }
    bool operator==(const Token& other) const {
        return kind == other.kind && text == other.text;
    }
    bool operator==(const KeywordToken& other) const {
        return kind == other.kind && text == other.text;
    }
};

