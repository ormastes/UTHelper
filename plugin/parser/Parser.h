#include "ASTNode.h"
#include "CommonHeader.h"
#include "Lexer.h"
#include "Token.h"

#include <llvm/ADT/StringSwitch.h>

#include "Common.h"

// Parser class
class Parser {
public:
    Parser(Lexer &lexer) : lex(lexer), currentToken(TOK_NOT_INIT) {
    }

    std::vector<PointcutDeclarationPtr> parsePointcutList() {
        std::vector<PointcutDeclarationPtr> declarations;
        while (currentToken.kind != TOK_EOF) {
            lex.updateDeclarationEnd();
            if (currentToken == TOK_NOT_INIT) {
                nextToken();
            }

            auto decl = pointcut_declaration();
            CASSERT_MSG(decl, "Failed to parse pointcut declaration.");
            declarations.push_back(std::move(decl));
        }
        return declarations;
    }

    void printContext(llvm::raw_ostream &OS) {
        llvm::errs() << "Current token: " << currentToken<<"\t";
        lex.printContext(llvm::errs());
    }

private:
    Lexer &lex;
    Token currentToken;

    void nextToken() {
        currentToken = lex.next();
        // Skip comments
        while (currentToken.kind == TOK_COMMENT) {
            currentToken = lex.next();
        }
    }
    

    Token _c(Token expected, llvm::StringRef msg = "") {

        if (currentToken != expected.kind) {
            if (!msg.empty()) {
                llvm::errs() << "Expected " << msg << expected << ", got " << currentToken << "\n";
            }
            return EOF_TOKEN;
        } 
        DEBUG_PRINT("check, current token: " + Token::toTwine(currentToken.kind));
        auto token = currentToken;
        nextToken();
        return token;
    }

    Token _(Token expected, llvm::StringRef msg = "") {
        auto token = _c(expected, msg);
        CASSERT(token != EOF_TOKEN)
        return token;
    }

    PointcutDeclarationPtr pointcut_declaration() {
        auto result = _pointcut_declaration();
        DEBUG_LOG("parsePointcutDeclaration, current token: " + Token::toTwine(currentToken.kind) + ", result: " + (result ? "success" : "failure"));
        DEBUG_NODE_LOG(result);
        return result;
    }
    
    PointcutDeclarationPtr _pointcut_declaration() {
        a isExported = false;
        a matchKind = MATCH_NONE;

        if (_c("run_pointcut")) { 
            isExported = true;
            matchKind = MATCH_RUN;
        } else if (_c("call_pointcut")) {
            isExported =true;
            matchKind = MATCH_CALL;
        }

        a name = _(TOK_IDENTIFIER, "Pointcut name").text;

        _("=");
        auto expr = parseExpression();

        CASSERT_MSG(expr, "Failed to parse an expression. There is no expression to parse.");

        _(";");

        return std::make_unique<PointcutDeclaration>(isExported, name, matchKind, std::move(expr));
    }
    Token pragma_kind() {
        auto ident = currentToken.text;
        // Check for keywords using llvm::StringSwitch
        TokenKind token = llvm::StringSwitch<TokenKind>(ident)
            .Case("bss", TOK_BSS)
            .Case("data", TOK_DATA)
            .Case("relro", TOK_RELRO)
            .Case("rodata", TOK_RODATA)
            .Case("text", TOK_TEXT)
            .Default(TOK_IDENTIFIER);
        currentToken.kind = token; 
        if (token != TOK_IDENTIFIER) {
            auto kind = currentToken;
            nextToken();
            return kind;
        } 
        CASSERT_MSG(false, "Expected pragma kind (bss, data, relro, rodata, text): " << currentToken);
    }

    ASTNodePtr parseExpression() {
        return parseOrExpression();
    }

    ASTNodePtr parseOrExpression() {
        auto lhs = parseAndExpression();
        while (_c("||")) {
            auto rhs = parseAndExpression();
            lhs = std::make_unique<OrExpression>(std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    ASTNodePtr parseAndExpression() {
        auto lhs = parseUnaryExpression();
        while (_c("&&")) {
            auto rhs = parseUnaryExpression();
            lhs = std::make_unique<AndExpression>(std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    ASTNodePtr parseUnaryExpression() {
        if (_c("!")) {
            auto expr = parseUnaryExpression();
            return std::make_unique<NotExpression>(std::move(expr));
        } else {
            return parsePrimaryExpression();
        }
    }

    ASTNodePtr parsePrimaryExpression() {
        if (_c("(")) {
            auto expr = parseExpression();
            _(")");
            return std::make_unique<ParenthesizedExpression>(std::move(expr));
        } else {
            return parsePointcutPrimary();
        }
    }


    ASTNodePtr parsePointcutPrimary() {

        if (_c(TOK_FUNC)) {
            _("(");
            auto functionName = _(TOK_IDENTIFIER, "Funciton name").text;
            _(")");
            DEBUG_LOG("parsePointcutPrimaryghghjjgh, current token: " + Token::toTwine(currentToken.kind));
            return std::make_unique<FuncExpression>(functionName);
        }
        if (_c(TOK_PRAGMA_CLANG)) {

             _("(");

            auto pragmaKind = pragma_kind();
            _(",");
            auto sectionName = _(TOK_IDENTIFIER, "Section name").text;

            _(")");

            return std::make_unique<PragmaClangExprNode>(pragmaKind, sectionName);
        }
        if (_c(TOK_ANNOTATION)) {
            _("(");
            auto annotationName = _(TOK_IDENTIFIER, "Annotation name").text;
            _(")");
            return std::make_unique<NotationExprNode>(annotationName);
        }
        if (_c(TOK_ANNOTATION_ANALYSIS)) {
            _("(");
            auto annotationName = _(TOK_IDENTIFIER, "Annotation name").text;
            _(")");
            return std::make_unique<NotationAnalysisExprNode>(annotationName);
        }

        ASSERT_MSG(false, "Unknown expression starting with " << currentToken << "\n");
        return nullptr;
    }

    void skipToNextSemicolon() {
        while (currentToken.kind != TOK_SEMICOLON && currentToken.kind != TOK_EOF) {
            nextToken();
        }
        if (currentToken.kind == TOK_SEMICOLON) {
            nextToken();
        }
    }
};