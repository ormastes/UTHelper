#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/StringRef.h"

class WrapFunctionCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  WrapFunctionCallback(clang::Rewriter &Rewrite, llvm::StringRef Id = "funcDecl");

  void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;

private:
  void processFunction(const clang::FunctionDecl *Func, clang::ASTContext *Context);

  clang::Rewriter &Rewrite;
  llvm::StringRef Id;
};