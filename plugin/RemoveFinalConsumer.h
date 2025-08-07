#ifndef REMOVE_FINAL_CONSUMER_H
#define REMOVE_FINAL_CONSUMER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include <string>

class RemoveFinalVisitor : public clang::RecursiveASTVisitor<RemoveFinalVisitor> {
public:
  RemoveFinalVisitor(clang::Rewriter &Rewrite, const std::string &BaseFolder);
  
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *D);
  bool VisitCXXMethodDecl(clang::CXXMethodDecl *D);
  
public:
  clang::SourceManager *SM;
  
private:
  bool isInBaseFolder(clang::SourceLocation Loc);
  void removeFinalFromRange(clang::SourceRange Range);
  
  clang::Rewriter &Rewrite;
  std::string BaseFolder;
};

class RemoveFinalConsumer : public clang::ASTConsumer {
public:
  RemoveFinalConsumer(clang::Rewriter &Rewrite, const std::string &BaseFolder);
  
  void HandleTranslationUnit(clang::ASTContext &Context) override;
  
private:
  RemoveFinalVisitor Visitor;
};

#endif // REMOVE_FINAL_CONSUMER_H