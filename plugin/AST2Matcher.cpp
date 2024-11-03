
#include <memory>

#include "AST2Matcher.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "ASTMatcherP.h"

ASTMakeMatcherVisitor ASTMakeMatcherVisitorInstance;


// Implementation of AbstractAST2Matcher
bool AbstractAST2Matcher::isType(MatchKind type) {
    return false;
}

// Implementation of DeclMatcher
bool DeclMatcher::isType(MatchKind type) {
    return MATCH_RUN == type;
}

// Implementation of StmtMatcher
bool StmtMatcher::isType(MatchKind type) {
    return MATCH_CALL == type;
}

// Implementation of CallMatcher
CallMatcher::CallMatcher(PointcutDeclaration* node, std::unique_ptr<DeclMatcher> Matcher)
    : FuncMatcher(std::move(Matcher)), PointcutDeclarationMatcher<clang::Stmt>(node->name), AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Stmt> CallMatcher::getMatcher() const {
    return clang::ast_matchers::callExpr(
        clang::ast_matchers::callee(FuncMatcher->getMatcher())
    ).bind(name);
}

// Implementation of RunMatcher
RunMatcher::RunMatcher(PointcutDeclaration* node, std::unique_ptr<DeclMatcher> Matcher)
    : FuncMatcher(std::move(Matcher)), PointcutDeclarationMatcher<clang::Decl>(node->name), AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Decl> RunMatcher::getMatcher() const {
    return clang::ast_matchers::functionDecl(FuncMatcher->getMatcher()).bind(name);
}

// Implementation of FuncNameMatcher
FuncNameMatcher::FuncNameMatcher(FuncExpression* node) : AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Decl> FuncNameMatcher::getMatcher() const {
    return clang::ast_matchers::functionDecl(
        clang::ast_matchers::hasName(node->id)
    );
}

// Implementation of AndMatcher
AndMatcher::AndMatcher(AndExpression* node, std::unique_ptr<DeclMatcher> Left, std::unique_ptr<DeclMatcher> Right)
    : LHS(std::move(Left)), RHS(std::move(Right)), AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Decl> AndMatcher::getMatcher() const {
    return clang::ast_matchers::allOf(
        LHS->getMatcher(), RHS->getMatcher()
    );
}

// Implementation of OrMatcher
OrMatcher::OrMatcher(OrExpression* node, std::unique_ptr<DeclMatcher> Left, std::unique_ptr<DeclMatcher> Right)
    : LHS(std::move(Left)), RHS(std::move(Right)), AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Decl> OrMatcher::getMatcher() const {
    return clang::ast_matchers::anyOf(
        LHS->getMatcher(), RHS->getMatcher()
    );
}

// Implementation of NotMatcher
NotMatcher::NotMatcher(NotExpression* node, std::unique_ptr<DeclMatcher> Matcher)
    : InnerMatcher(std::move(Matcher)), AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Decl> NotMatcher::getMatcher() const {
    return clang::ast_matchers::unless(
        InnerMatcher->getMatcher()
    );
}

// Implementation of ParenthesizedMatcher
ParenthesizedMatcher::ParenthesizedMatcher(ParenthesizedExpression* node, std::unique_ptr<DeclMatcher> Matcher)
    : InnerMatcher(std::move(Matcher)), AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Decl> ParenthesizedMatcher::getMatcher() const {
    return InnerMatcher->getMatcher();
}

// Implementation of PragmaClangMatcher
PragmaClangMatcher::PragmaClangMatcher(PragmaClangExprNode* node) : AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Decl> PragmaClangMatcher::getMatcher() const {
    using namespace clang::ast_matchers;
    switch (node->pragmaKind.kind) {
        case TOK_BSS:
            return functionDecl(hasPragmaClangBSSSectionAttr(node->sectionName));
        case TOK_DATA:
            return functionDecl(hasPragmaClangDataSectionAttr(node->sectionName));
        case TOK_RELRO:
            return functionDecl(hasPragmaClangRelroSectionAttr(node->sectionName));
        case TOK_RODATA:
            return functionDecl(hasPragmaClangRodataSectionAttr(node->sectionName));
        case TOK_TEXT:
            return functionDecl(hasPragmaClangTextSectionAttr(node->sectionName));
        default:
            llvm::errs() << "Unknown pragma kind " << node->pragmaKind << "\n";
            assert(false);
            return clang::ast_matchers::functionDecl(); // To satisfy return type
    }
}

// Implementation of NotationMatcher
NotationMatcher::NotationMatcher(NotationExprNode* node) : AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Decl> NotationMatcher::getMatcher() const {
    return clang::ast_matchers::functionDecl(hasAnnotateAttrWithValue(node->id));
}

// Implementation of NotationAnalysisMatcher
NotationAnalysisMatcher::NotationAnalysisMatcher(NotationAnalysisExprNode* node) : AST(node) {}

clang::ast_matchers::internal::Matcher<clang::Decl> NotationAnalysisMatcher::getMatcher() const {
    return clang::ast_matchers::functionDecl(hasAnnotateTypeAttrWithValue(node->id));
}