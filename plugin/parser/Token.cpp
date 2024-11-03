#include "Token.h"

Token::Token(const char* s) : Token(llvm::StringRef(s)) {}
Token::Token(TokenKind k, llvm::StringRef t) : kind(k), text(t) {}

//make std::out printable with text if it is not empty
llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const Token &T) {
    OS << Token::toTwine(T.kind) << ": " << T.kind << " ";
    if (!T.text.empty()) {
        OS << " (" << T.text << ")";
    }
    return OS;
}

bool Token::operator==(TokenKind k) const { return kind == k; }

bool Token::operator==(llvm::StringRef t) const { return text == t; }

const bool Token::operator==(Token& other) const { return kind == other.kind && text == other.text; }

bool Token::operator<(const Token& other) const {
    return kind < other.kind || (kind == other.kind && text < other.text);
}
// cast to bool
Token::operator bool() const { return kind != TOK_EOF; }


auto Token::toStr(TokenKind type) { 
    switch (type) {
        #define TOKEN_DEF(NAME, STR) case NAME: return STR;
        #include "Token.def"
        #undef TOKEN_DEF
        default: 
            llvm_unreachable(("Unknown token kind " + std::to_string(type)).c_str());
    }
}

llvm::Twine Token::toTwine(TokenKind type) { 
    return toStr(type);
}

llvm::StringMap<TokenKind> Token::toTokenKind({
    #define TOKEN_DEF(NAME, STR) {STR, NAME},
    #include "Token.def"
    #undef TOKEN_DEF
});


Token::Token(TokenKind k) : kind(k), text(toStr(k)) {}
Token::Token(StringRef s) : kind(Token::toTokenKind.at(s)), text(s) {}

const Token EOF_TOKEN(TOK_EOF);

ParenOpenToken::ParenOpenToken(const char* cur, const char* pairParent) : Token(TOK_LPAREN, llvm::StringRef(cur, pairParent-cur)) {}

bool ParenOpenToken::operator==(TokenKind k) const { return kind == k; }
bool ParenOpenToken::operator==(const Token& other) const {
    return kind == other.kind && text == other.text;
}
bool ParenOpenToken::operator==(const ParenOpenToken& other) const {
    return kind == other.kind && text == other.text;
}

KeywordToken::KeywordToken(TokenKind k) : Token(k){}
KeywordToken::KeywordToken(llvm::StringRef t) : Token(keywords.lookup(t), t) {}
bool KeywordToken::isKeyword(llvm::StringRef text) {
    return keywords.count(text);
}
bool KeywordToken::operator==(TokenKind k) const { return kind == k; }
bool KeywordToken::operator==(const Token& other) const {
    return kind == other.kind && text == other.text;
}
bool KeywordToken::operator==(const KeywordToken& other) const {
    return kind == other.kind && text == other.text;
}


// _TokenKindToStr(TOK_POINTCUT) and TOK_POINTCUT map initialization
llvm::StringMap<TokenKind> KeywordToken::keywords({
    {toStr(TOK_POINTCUT), TOK_POINTCUT},
    {toStr(TOK_FUNC), TOK_FUNC},
    {toStr(TOK_WITHIN), TOK_WITHIN},
    {toStr(TOK_EXPORT), TOK_EXPORT},
    {toStr(TOK_PRAGMA_CLANG), TOK_PRAGMA_CLANG},
    {toStr(TOK_ANNOTATION), TOK_ANNOTATION},
    {toStr(TOK_ANNOTATION_ANALYSIS), TOK_ANNOTATION_ANALYSIS},
    {toStr(TOK_CONST), TOK_CONST},
    {toStr(TOK_STATIC), TOK_STATIC},
    {toStr(TOK_REGISTER), TOK_REGISTER},
    {toStr(TOK_VOLATILE), TOK_VOLATILE},
    {toStr(TOK_RESTRICT), TOK_RESTRICT},
    {toStr(TOK_RUN_POINTCUT), TOK_RUN_POINTCUT},
    {toStr(TOK_CALL_POINTCUT), TOK_CALL_POINTCUT}

});
/*
    {toStr(TOK_BSS), TOK_BSS},
    {toStr(TOK_DATA), TOK_DATA},
    {toStr(TOK_RELRO), TOK_RELRO},
    {toStr(TOK_RODATA), TOK_RODATA},
    {toStr(TOK_TEXT), TOK_TEXT},
*/
/*
TOKEN_DEF(TOK_BSS, "bss")
TOKEN_DEF(TOK_DATA, "data")
TOKEN_DEF(TOK_RELRO, "relro")
TOKEN_DEF(TOK_RODATA, "rodata")
TOKEN_DEF(TOK_TEXT, "text")
*/