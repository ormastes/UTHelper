#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/StringRef.h"
#include <cfloat>

class WrapFunctionCallback
    : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  WrapFunctionCallback(clang::Rewriter &Rewrite,
                       llvm::StringRef Id = "funcDecl");

  void
  run(const clang::ast_matchers::MatchFinder::MatchResult &Result) override;
  
  void setBaseFolder(const std::string &BaseFolder);

private:
  void processFunction(const clang::FunctionDecl *Func,
                       clang::ASTContext *Context);
  std::string buildWrapperFunction(const clang::FunctionDecl *Func,
                                   const std::string &OriginalName,
                                   const std::string &WrappedName,
                                   const std::string &QualifiedName,
                                   const std::string &QualifiedNameUnderbar,
                                   bool IsMethod, bool IsVirtual,
                                   bool IsConstMethod, bool IsStaticMethod,
                                   const std::string &ClassName, llvm::StringRef Id);

  bool isInBaseFolder(clang::SourceLocation Loc, clang::SourceManager &SM);
  
  clang::Rewriter &Rewrite;
  llvm::StringRef Id;
  std::string BaseFolder;
};