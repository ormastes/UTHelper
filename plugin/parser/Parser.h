#include "ASTNode.h"
#include "Lexer.h"
#include "Token.h"
#include <vector>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/raw_ostream.h>


// Parser class
class Parser {
public:
    Parser(Lexer &lexer);

    std::vector<PointcutDeclarationPtr> parsePointcutList();

    void printContext(llvm::raw_ostream &OS);

private:
    Lexer &lex;
    Token currentToken;

    void nextToken();

    Token _c(TokenKind expectedKind, llvm::StringRef msg = "");
    Token _(TokenKind expectedKind, llvm::StringRef msg = "");

    PointcutDeclarationPtr pointcut_declaration();
    PointcutDeclarationPtr _pointcut_declaration();

    Token pragma_kind();

    ASTNodePtr parseExpression();
    ASTNodePtr parseOrExpression();
    ASTNodePtr parseAndExpression();
    ASTNodePtr parseUnaryExpression();
    ASTNodePtr parsePrimaryExpression();
    ASTNodePtr parsePointcutPrimary();

    void skipToNextSemicolon();
};