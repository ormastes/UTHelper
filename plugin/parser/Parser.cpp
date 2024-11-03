#include "Parser.h"
#include "Common.h"

Parser::Parser(Lexer &lexer) : lex(lexer), currentToken(TOK_NOT_INIT) {
}

std::vector<PointcutDeclarationPtr> Parser::parsePointcutList() {
    std::vector<PointcutDeclarationPtr> declarations;
    while (currentToken.kind != TOK_EOF) {
        lex.updateDeclarationEnd();
        if (currentToken.kind == TOK_NOT_INIT) {
            nextToken();
        }

        auto decl = pointcut_declaration();
        CASSERT_MSG(decl, "Failed to parse pointcut declaration.");
        declarations.push_back(std::move(decl));
    }
    return declarations;
}

void Parser::printContext(llvm::raw_ostream &OS) {
    llvm::errs() << "Current token: " << currentToken << "\t";
    lex.printContext(llvm::errs());
}

void Parser::nextToken() {
    currentToken = lex.next();
    // Skip comments
    while (currentToken.kind == TOK_COMMENT) {
        currentToken = lex.next();
    }
}

Token Parser::_c(TokenKind expectedKind, llvm::StringRef msg) {
    if (currentToken.kind != expectedKind) {
        if (!msg.empty()) {
            llvm::errs() << "Expected " << msg << Token(expectedKind) << ", got " << currentToken << "\n";
        }
        return Token(TOK_EOF);
    }
    DEBUG_PRINT("check, current token: " + Token::toTwine(currentToken.kind));
    auto token = currentToken;
    nextToken();
    return token;
}

Token Parser::_(TokenKind expectedKind, llvm::StringRef msg) {
    auto token = _c(expectedKind, msg);
    CASSERT(token.kind != TOK_EOF);
    return token;
}

PointcutDeclarationPtr Parser::pointcut_declaration() {
    auto result = _pointcut_declaration();
    DEBUG_LOG("parsePointcutDeclaration, current token: " + Token::toTwine(currentToken.kind) + ", result: " + (result ? "success" : "failure"));
    DEBUG_NODE_LOG(result);
    return result;
}

PointcutDeclarationPtr Parser::_pointcut_declaration() {
    bool isExported = false;
    MatchKind matchKind = MATCH_NONE;
    llvm::StringRef name;

    if (_c(TOK_RUN_POINTCUT).kind != TOK_EOF) {
        isExported = true;
        matchKind = MATCH_RUN;
    } else if (_c(TOK_CALL_POINTCUT).kind != TOK_EOF) {
        isExported = true;
        matchKind = MATCH_CALL;
    }

    name = _(TOK_IDENTIFIER, "Pointcut name").text;

    _(TOK_EQUAL);
    auto expr = parseExpression();

    CASSERT_MSG(expr, "Failed to parse an expression. There is no expression to parse.");

    _(TOK_SEMICOLON);

    return std::make_unique<PointcutDeclaration>(isExported, name, matchKind, std::move(expr));
}

Token Parser::pragma_kind() {
    llvm::StringRef ident = currentToken.text;
    // Check for keywords using llvm::StringSwitch
    TokenKind tokenKind = llvm::StringSwitch<TokenKind>(ident)
        .Case("bss", TOK_BSS)
        .Case("data", TOK_DATA)
        .Case("relro", TOK_RELRO)
        .Case("rodata", TOK_RODATA)
        .Case("text", TOK_TEXT)
        .Default(TOK_IDENTIFIER);
    currentToken.kind = tokenKind;
    if (tokenKind != TOK_IDENTIFIER) {
        auto kind = currentToken;
        nextToken();
        return kind;
    }
    CASSERT_MSG(false, "Expected pragma kind (bss, data, relro, rodata, text): " << currentToken);
    return Token(TOK_EOF);
}

ASTNodePtr Parser::parseExpression() {
    return parseOrExpression();
}

ASTNodePtr Parser::parseOrExpression() {
    auto lhs = parseAndExpression();
    while (_c(TOK_PIPEPIPE).kind != TOK_EOF) {
        auto rhs = parseAndExpression();
        lhs = std::make_unique<OrExpression>(std::move(lhs), std::move(rhs));
    }
    return lhs;
}

ASTNodePtr Parser::parseAndExpression() {
    auto lhs = parseUnaryExpression();
    while (_c(TOK_AMPAMP).kind != TOK_EOF) {
        auto rhs = parseUnaryExpression();
        lhs = std::make_unique<AndExpression>(std::move(lhs), std::move(rhs));
    }
    return lhs;
}

ASTNodePtr Parser::parseUnaryExpression() {
    if (_c(TOK_EXCLAMATION).kind != TOK_EOF) {
        auto expr = parseUnaryExpression();
        return std::make_unique<NotExpression>(std::move(expr));
    } else {
        return parsePrimaryExpression();
    }
}

ASTNodePtr Parser::parsePrimaryExpression() {
    if (_c(TOK_LPAREN).kind != TOK_EOF) {
        auto expr = parseExpression();
        _(TOK_RPAREN);
        return std::make_unique<ParenthesizedExpression>(std::move(expr));
    } else {
        return parsePointcutPrimary();
    }
}

ASTNodePtr Parser::parsePointcutPrimary() {
    if (_c(TOK_FUNC).kind != TOK_EOF) {
        _(TOK_LPAREN);
        auto functionName = _(TOK_IDENTIFIER, "Function name").text;
        _(TOK_RPAREN);
        DEBUG_LOG("parsePointcutPrimary, current token: " + Token::toTwine(currentToken.kind));
        return std::make_unique<FuncExpression>(functionName);
    }
    if (_c(TOK_PRAGMA_CLANG).kind != TOK_EOF) {
        _(TOK_LPAREN);
        auto pragmaKind = pragma_kind();
        _(TOK_COMMA);
        auto sectionName = _(TOK_IDENTIFIER, "Section name").text;
        _(TOK_RPAREN);
        return std::make_unique<PragmaClangExprNode>(pragmaKind, sectionName);
    }
    if (_c(TOK_ANNOTATION).kind != TOK_EOF) {
        _(TOK_LPAREN);
        auto annotationName = _(TOK_IDENTIFIER, "Annotation name").text;
        _(TOK_RPAREN);
        return std::make_unique<NotationExprNode>(annotationName);
    }
    if (_c(TOK_ANNOTATION_ANALYSIS).kind != TOK_EOF) {
        _(TOK_LPAREN);
        auto annotationName = _(TOK_IDENTIFIER, "Annotation name").text;
        _(TOK_RPAREN);
        return std::make_unique<NotationAnalysisExprNode>(annotationName);
    }

    ASSERT_MSG(false, "Unknown expression starting with " << currentToken << "\n");
    return nullptr;
}

void Parser::skipToNextSemicolon() {
    while (currentToken.kind != TOK_SEMICOLON && currentToken.kind != TOK_EOF) {
        nextToken();
    }
    if (currentToken.kind == TOK_SEMICOLON) {
        nextToken();
    }
}