#ifndef UNIFIED_AST_VISITOR_H
#define UNIFIED_AST_VISITOR_H

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include <string>
#include <vector>

// Friend template structure moved from AddFriendConsumer.h
struct FriendTemplate {
  std::string templateText;
  
  std::string instantiate(const std::string& namespaceName, 
                         const std::string& className) const;
};

class UnifiedASTVisitor : public clang::RecursiveASTVisitor<UnifiedASTVisitor> {
public:
  UnifiedASTVisitor(clang::Rewriter &Rewrite, const std::string &BaseFolder,
                    bool EnableRemoveFinal, bool EnableMakeVirtual, 
                    bool EnableAddFriend, const std::vector<FriendTemplate> &CustomFriends);

  bool VisitCXXRecordDecl(clang::CXXRecordDecl *D);
  bool VisitCXXMethodDecl(clang::CXXMethodDecl *D);
  
  void setSourceManager(clang::SourceManager *SM) { this->SM = SM; }

private:
  clang::Rewriter &Rewrite;
  std::string BaseFolder;
  clang::SourceManager *SM;
  
  // Feature flags
  bool EnableRemoveFinal;
  bool EnableMakeVirtual;
  bool EnableAddFriend;
  std::vector<FriendTemplate> CustomFriends;
  
  // Helper methods
  bool isInBaseFolder(clang::SourceLocation Loc);
  
  // RemoveFinal feature methods
  void removeFinalFromRange(clang::SourceRange Range);
  void processRemoveFinalForClass(clang::CXXRecordDecl *D);
  void processRemoveFinalForMethod(clang::CXXMethodDecl *D);
  
  // MakeVirtual feature methods
  bool canBeMadeVirtual(clang::CXXMethodDecl *D);
  void makeMethodVirtual(clang::CXXMethodDecl *D);
  
  // AddFriend feature methods
  std::string getNamespaceString(const clang::DeclContext *DC);
  void addFriendDeclarations(clang::CXXRecordDecl *D);
};

class UnifiedASTConsumer : public clang::ASTConsumer {
public:
  UnifiedASTConsumer(clang::Rewriter &Rewrite, const std::string &BaseFolder,
                     bool EnableRemoveFinal, bool EnableMakeVirtual, 
                     bool EnableAddFriend, const std::vector<FriendTemplate> &CustomFriends);
  
  void HandleTranslationUnit(clang::ASTContext &Context) override;

private:
  UnifiedASTVisitor Visitor;
};

#endif // UNIFIED_AST_VISITOR_H