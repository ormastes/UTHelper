#include "Token.h"


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