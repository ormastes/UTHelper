#pragma once

#include <llvm-18/llvm/ADT/StringRef.h>
#include <memory>
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
  virtual bool isType(MatchKind type) { return false; }
};
template <typename NodeType>
struct AST2Matcher : AbstractAST2Matcher {
  virtual clang::ast_matchers::internal::Matcher<NodeType>  getMatcher() const = 0;
};

template<typename NodeType>
struct PointcutDeclarationMatcher  {
  StringRef name;

  // Constructor to set the name
  PointcutDeclarationMatcher(const StringRef& n) : name(n) {}
};

struct DeclMatcher : AST2Matcher<clang::Decl> {
  // Potential common functionality for declaration matchers
  virtual bool isType(MatchKind type) { return MATCH_RUN==type; }
};

struct StmtMatcher : AST2Matcher<clang::Stmt> {
  // Potential common functionality for statement matchers
  virtual bool isType(MatchKind type) { return MATCH_CALL==type; }
};

struct CallMatcher : StmtMatcher, PointcutDeclarationMatcher<clang::Stmt> , AST<PointcutDeclaration> {
  std::unique_ptr<DeclMatcher> FuncMatcher;

  CallMatcher(PointcutDeclaration* node, std::unique_ptr<DeclMatcher> Matcher)
      : FuncMatcher(std::move(Matcher)), PointcutDeclarationMatcher(node->name), AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Stmt> getMatcher() const override {
    return clang::ast_matchers::callExpr(
        clang::ast_matchers::callee(FuncMatcher->getMatcher())
    ).bind(name);
  }
};


struct ReadMatcher : StmtMatcher, PointcutDeclarationMatcher<clang::Stmt> , AST<PointcutDeclaration> {
  std::unique_ptr<DeclMatcher> VarMatcher;

  ReadMatcher(PointcutDeclaration* node, std::unique_ptr<DeclMatcher> Matcher)
      : VarMatcher(std::move(Matcher)), PointcutDeclarationMatcher(node->name),  AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Stmt> getMatcher() const override {
    return clang::ast_matchers::expr(
        clang::ast_matchers::ignoringParenImpCasts(
            clang::ast_matchers::declRefExpr(
                clang::ast_matchers::to(VarMatcher->getMatcher())
            )
        )
    ).bind(name);
  }
};

struct WriteMatcher :StmtMatcher,  PointcutDeclarationMatcher<clang::Stmt> , AST<PointcutDeclaration> {
  std::unique_ptr<DeclMatcher> VarMatcher;

  WriteMatcher(PointcutDeclaration* node, std::unique_ptr<DeclMatcher> Matcher)
      : VarMatcher(std::move(Matcher)), PointcutDeclarationMatcher(node->name),  AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Stmt> getMatcher() const override {
    return clang::ast_matchers::binaryOperator(
        clang::ast_matchers::hasLHS(
            clang::ast_matchers::ignoringParenImpCasts(
                clang::ast_matchers::declRefExpr(
                    clang::ast_matchers::to(VarMatcher->getMatcher())
                )
            )
        ),
        clang::ast_matchers::isAssignmentOperator()
    ).bind(name);
  }
};

struct RunMatcher : DeclMatcher, PointcutDeclarationMatcher<clang::Decl> , AST<PointcutDeclaration> {
  std::unique_ptr<DeclMatcher> FuncMatcher;

  RunMatcher(PointcutDeclaration* node, std::unique_ptr<DeclMatcher> Matcher)
      : FuncMatcher(std::move(Matcher)), PointcutDeclarationMatcher(node->name), AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    return clang::ast_matchers::functionDecl(FuncMatcher->getMatcher()).bind(name);
  }
};



struct IsMemberType : DeclMatcher, AST<PointcutDeclaration> {
  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    assert(false);
  }
};

struct IsMemberChildType : DeclMatcher, AST<PointcutDeclaration> {
  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    assert(false);
  }
};

struct FuncNameMatcher : DeclMatcher, AST<FuncExpression> {
  FuncNameMatcher(FuncExpression* node) 
    : AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    return clang::ast_matchers::functionDecl(
        clang::ast_matchers::hasName(node->id)
    );
  }
};
/*
struct VarNameMatcher : DeclMatcher, AST<PointcutDeclaration> {
  std::string Name;

  VarNameMatcher(const std::string &VarName) : Name(VarName) {}

  clang::ast_matchers::internal::Matcher<clang::Stmt> getMatcher() const override {
    return clang::ast_matchers::anyOf(
        clang::ast_matchers::varDecl(clang::ast_matchers::hasName(Name)),
        clang::ast_matchers::fieldDecl(clang::ast_matchers::hasName(Name))
    );
  }
};
*/
struct AndMatcher : DeclMatcher, AST<AndExpression> {
  std::unique_ptr<DeclMatcher> LHS;
  std::unique_ptr<DeclMatcher> RHS;

  AndMatcher(AndExpression* node, std::unique_ptr<DeclMatcher> Left, std::unique_ptr<DeclMatcher> Right)
      : LHS(std::move(Left)), RHS(std::move(Right)), AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    return clang::ast_matchers::allOf(
        LHS->getMatcher(), RHS->getMatcher()
    );
  }
};

struct OrMatcher : DeclMatcher, AST<OrExpression> {
  std::unique_ptr<DeclMatcher> LHS;
  std::unique_ptr<DeclMatcher> RHS;

  OrMatcher(OrExpression* node, std::unique_ptr<DeclMatcher> Left, std::unique_ptr<DeclMatcher> Right)
      : LHS(std::move(Left)), RHS(std::move(Right)), AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    return clang::ast_matchers::anyOf(
        LHS->getMatcher(), RHS->getMatcher()
    );
  }
};

struct NotMatcher : DeclMatcher, AST<NotExpression> {
  std::unique_ptr<DeclMatcher> InnerMatcher;

  NotMatcher(NotExpression* node, std::unique_ptr<DeclMatcher> Matcher)
      : InnerMatcher(std::move(Matcher)), AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    return clang::ast_matchers::unless(
        InnerMatcher->getMatcher()
    );
  }
};

struct ParenthesizedMatcher: DeclMatcher, AST<ParenthesizedExpression> {
  std::unique_ptr<DeclMatcher> InnerMatcher;

  ParenthesizedMatcher(ParenthesizedExpression* node, std::unique_ptr<DeclMatcher> Matcher)
      : InnerMatcher(std::move(Matcher)), AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    return InnerMatcher->getMatcher();
  }
};

struct PragmaClangMatcher : DeclMatcher, AST<PragmaClangExprNode> {

  PragmaClangMatcher(PragmaClangExprNode* node) : AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
      switch (node->pragmaKind.kind) {
    case TOK_BSS:
      return clang::ast_matchers::functionDecl(hasPragmaClangBSSSectionAttr(node->sectionName));
    case TOK_DATA:
      return clang::ast_matchers::functionDecl(hasPragmaClangDataSectionAttr(node->sectionName));
    case TOK_RELRO:
      return clang::ast_matchers::functionDecl(hasPragmaClangRelroSectionAttr(node->sectionName));
    case TOK_RODATA:
      return clang::ast_matchers::functionDecl(hasPragmaClangRodataSectionAttr(node->sectionName));
    case TOK_TEXT:
      return clang::ast_matchers::functionDecl(hasPragmaClangTextSectionAttr(node->sectionName));
    default:
      llvm::errs() << "Unknown pragma kind " << node->pragmaKind << "\n";
      assert(false);
    }
  }
};

struct NotationMatcher : DeclMatcher, AST<NotationExprNode> {

  NotationMatcher(NotationExprNode* node) : AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    return clang::ast_matchers::functionDecl(hasAnnotateAttrWithValue(node->id));
  }
};

struct NotationAnalysisMatcher : DeclMatcher, AST<NotationAnalysisExprNode> {

  NotationAnalysisMatcher(NotationAnalysisExprNode* node) :  AST(node) {}

  clang::ast_matchers::internal::Matcher<clang::Decl> getMatcher() const override {
    return clang::ast_matchers::functionDecl(hasAnnotateTypeAttrWithValue(node->id));
  }
};

