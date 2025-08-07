#ifndef MAKE_VIRTUAL_CONSUMER_H
#define MAKE_VIRTUAL_CONSUMER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include <string>

class MakeVirtualVisitor : public clang::RecursiveASTVisitor<MakeVirtualVisitor> {
public:
  MakeVirtualVisitor(clang::Rewriter &Rewrite, const std::string &BaseFolder);
  
  bool VisitCXXMethodDecl(clang::CXXMethodDecl *D);
  
  clang::SourceManager *SM;
  
private:
  bool isInBaseFolder(clang::SourceLocation Loc);
  bool canBeMadeVirtual(clang::CXXMethodDecl *D);
  void makeMethodVirtual(clang::CXXMethodDecl *D);
  
  clang::Rewriter &Rewrite;
  std::string BaseFolder;
};

class MakeVirtualConsumer : public clang::ASTConsumer {
public:
  MakeVirtualConsumer(clang::Rewriter &Rewrite, const std::string &BaseFolder);
  
  void HandleTranslationUnit(clang::ASTContext &Context) override;
  
private:
  MakeVirtualVisitor Visitor;
};

#endif // MAKE_VIRTUAL_CONSUMER_H