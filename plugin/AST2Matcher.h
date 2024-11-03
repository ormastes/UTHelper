#pragma once

#include <memory>
#include "llvm/ADT/StringRef.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "ASTNode.h"
#include "ASTMakeMatcherVisitor.h"
#include "ASTMatcherP.h"

template <typename T>
struct AST {
    const T* node;
    AST(T* node) : node(node) {}
};

extern ASTMakeMatcherVisitor ASTMakeMatcherVisitorInstance;

struct AbstractAST2Matcher {
    AbstractAST2Matcher() = default;
    virtual ~AbstractAST2Matcher() = default;
    virtual bool isType(MatchKind type);
};

template <typename NodeType>
struct AST2Matcher : AbstractAST2Matcher {
    virtual clang::ast_matchers::internal::Matcher<NodeType> getMatcher() const = 0;
};

template <typename NodeType>
struct PointcutDeclarationMatcher {
    llvm::StringRef name;

    // Constructor to set the name
    PointcutDeclarationMatcher(const llvm::StringRef& n) : name(n) {}
};

struct DeclMatcher : AST2Matcher<clang::Decl> {
    virtual bool isType(MatchKind type) override;
};

struct StmtMatcher : AST2Matcher<clang::Stmt> {
    virtual bool isType(MatchKind type) override;
};

struct CallMatcher : StmtMatcher, PointcutDeclarationMatcher<clang::Stmt>, AST<PointcutDeclaration> {
    std::unique_ptr<DeclMatcher> FuncMatcher;

    CallMatcher(PointcutDeclaration* node, std::unique_ptr<DeclMatcher> Matcher);

    clang::ast_matchers::internal::Matcher<clang::Stmt> getMatcher() const override;
};

struct RunMatcher : DeclMatcher, PointcutDeclarationMatcher<clang::Decl>, AST<PointcutDeclaration> {
    std::unique_ptr<DeclMatcher> FuncMatcher;

    RunMatcher(PointcutDeclaration* node, std::unique_ptr<DeclMatcher> Matcher);

    clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override;
};

struct FuncNameMatcher : DeclMatcher, AST<FuncExpression> {
    FuncNameMatcher(FuncExpression* node);

    clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override;
};

struct AndMatcher : DeclMatcher, AST<AndExpression> {
    std::unique_ptr<DeclMatcher> LHS;
    std::unique_ptr<DeclMatcher> RHS;

    AndMatcher(AndExpression* node, std::unique_ptr<DeclMatcher> Left, std::unique_ptr<DeclMatcher> Right);

    clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override;
};

struct OrMatcher : DeclMatcher, AST<OrExpression> {
    std::unique_ptr<DeclMatcher> LHS;
    std::unique_ptr<DeclMatcher> RHS;

    OrMatcher(OrExpression* node, std::unique_ptr<DeclMatcher> Left, std::unique_ptr<DeclMatcher> Right);

    clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override;
};

struct NotMatcher : DeclMatcher, AST<NotExpression> {
    std::unique_ptr<DeclMatcher> InnerMatcher;

    NotMatcher(NotExpression* node, std::unique_ptr<DeclMatcher> Matcher);

    clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override;
};

struct ParenthesizedMatcher : DeclMatcher, AST<ParenthesizedExpression> {
    std::unique_ptr<DeclMatcher> InnerMatcher;

    ParenthesizedMatcher(ParenthesizedExpression* node, std::unique_ptr<DeclMatcher> Matcher);

    clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override;
};

struct PragmaClangMatcher : DeclMatcher, AST<PragmaClangExprNode> {
    PragmaClangMatcher(PragmaClangExprNode* node);

    clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override;
};

struct NotationMatcher : DeclMatcher, AST<NotationExprNode> {
    NotationMatcher(NotationExprNode* node);

    clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override;
};

struct NotationAnalysisMatcher : DeclMatcher, AST<NotationAnalysisExprNode> {
    NotationAnalysisMatcher(NotationAnalysisExprNode* node);

    clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override;
};