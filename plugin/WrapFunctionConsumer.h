#pragma once
#include "WrapFunctionCallback.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include <memory>

#include "AST2Matcher.h"
#include "Lexer.h"
#include "Parser.h"

// Custom matcher to check for an AnnotateAttr with a specific annotation value
AST_MATCHER_P(clang::FunctionDecl, _hasAnnotateAttrWithValue, std::string,
              AnnotationValue) {
  for (const auto *Attr : Node.attrs()) {
    if (const auto *Annotate = llvm::dyn_cast<clang::AnnotateAttr>(Attr)) {
      if (Annotate->getAnnotation() == AnnotationValue) {
        return true;
      }
    }
  }
  return false;
}

// Custom matcher to check for a SectionAttr with a specific name
AST_MATCHER_P(clang::FunctionDecl, _hasSectionAttrWithValue, std::string,
              SectionName) {
  // from node get PragmaClangTextSectionAttr and check if it has the correct
  // name
  for (const auto *Attr : Node.attrs()) {
    if (const auto *SA =
            llvm::dyn_cast<clang::PragmaClangTextSectionAttr>(Attr)) {
      return SA->getName() == SectionName;
    }
  }
  return false;
}

class WrapFunctionConsumer : public clang::ASTConsumer {
  const clang::ast_matchers::DeclarationMatcher
  generateMatcher(std::string PointcutText) {
    // llvm::StringRef testInput4 = "run_pointcut myPointcut =
    // pragma_clang(text, data) || annotation(wrap);";
    Lexer lexer4(PointcutText);
    Parser parser4(lexer4);
    auto ast = parser4.parsePointcutList();
    return clang::ast_matchers::functionDecl(
               clang::ast_matchers::isExpansionInMainFile(),
               clang::ast_matchers::anyOf(_hasAnnotateAttrWithValue("wrap"),
                                          _hasSectionAttrWithValue("data")))
        .bind("funcDecl");
  }

public:
#if 0
  WrapFunctionConsumer(clang::Rewriter &R) : Handler(R) {
    std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;
    auto wrap = clang::ast_matchers::functionDecl(_hasAnnotateAttrWithValue("wrap"));
    auto data = clang::ast_matchers::functionDecl(_hasSectionAttrWithValue("data"));
    clang::ast_matchers::internal::Matcher<clang::Decl> matchAny =
        clang::ast_matchers::anyOf(wrap, data);
    auto mainSpace = clang::ast_matchers::isExpansionInMainFile();
    auto funcDecl = clang::ast_matchers::functionDecl(mainSpace, matchAny);
    clang::ast_matchers::internal::Matcher<clang::Decl> funcDeclBind =
        funcDecl.bind("funcDecl");
    matcher_decl.push_back(funcDeclBind);
    for (const auto &matcher : matcher_decl) {
      Matcher.addMatcher(matcher, &Handler);
    }
  }
#else

  WrapFunctionConsumer(clang::Rewriter &R) : Handler(R) {
    std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;

    llvm::StringRef PointcutText = "run_pointcut funcDecl = pragma_clang(text, "
                                 "data) || annotation(wrap);";
    Lexer lexer4(PointcutText);
    Parser parser4(lexer4);
    auto ast = parser4.parsePointcutList();
    ASTMakeMatcherVisitor visitor;
    for (const auto &pointcut : ast) {
      pointcut->accept(visitor);
      auto matcher = visitor.getMatcher();
      if (matcher->isType(MATCH_RUN)) {
        RunMatcher* runMatcher = (RunMatcher*)matcher.get();
        clang::ast_matchers::DeclarationMatcher functionMatcher = runMatcher->getMatcher();
        Matcher.addMatcher(functionMatcher, &Handler);
      } else {
        assert(false);
      }
    }
  }
#endif
  WrapFunctionConsumer(clang::Rewriter &R, std::string PointcutText)
      : Handler(R) {
    std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;

    llvm::StringRef testInput4 = "run_pointcut myPointcut = pragma_clang(text, "
                                 "data) || annotation(wrap);";
    Lexer lexer4(PointcutText);
    Parser parser4(lexer4);
    auto ast = parser4.parsePointcutList();
    ASTMakeMatcherVisitor visitor;
    for (const auto &pointcut : ast) {
      pointcut->accept(visitor);
      auto matcher = visitor.getMatcher();
      if (matcher->isType(MATCH_RUN)) {
        RunMatcher* runMatcher = (RunMatcher*)matcher.get();
        clang::ast_matchers::internal::Matcher<clang::Decl> functionMatcher = runMatcher->getMatcher();
        Matcher.addMatcher(functionMatcher, &Handler);
      }
    }
  }

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Matcher.matchAST(Context);
  }
  std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;

private:
  WrapFunctionCallback Handler;
  clang::ast_matchers::MatchFinder Matcher;
};
