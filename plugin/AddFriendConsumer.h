#ifndef ADD_FRIEND_CONSUMER_H
#define ADD_FRIEND_CONSUMER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include <string>
#include <vector>

struct FriendTemplate {
  std::string templateText;
  
  // Replace {namespace} and {class-name} with actual values
  std::string instantiate(const std::string& namespaceName, const std::string& className) const;
};

class AddFriendVisitor : public clang::RecursiveASTVisitor<AddFriendVisitor> {
public:
  AddFriendVisitor(clang::Rewriter &Rewrite, const std::string &BaseFolder,
                   const std::vector<FriendTemplate> &CustomFriends);
  
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *D);
  
  clang::SourceManager *SM;
  
private:
  bool isInBaseFolder(clang::SourceLocation Loc);
  void addFriendDeclarations(clang::CXXRecordDecl *D);
  std::string getNamespaceString(const clang::DeclContext *DC);
  
  clang::Rewriter &Rewrite;
  std::string BaseFolder;
  std::vector<FriendTemplate> CustomFriends;
};

class AddFriendConsumer : public clang::ASTConsumer {
public:
  AddFriendConsumer(clang::Rewriter &Rewrite, const std::string &BaseFolder,
                    const std::vector<FriendTemplate> &CustomFriends);
  
  void HandleTranslationUnit(clang::ASTContext &Context) override;
  
private:
  AddFriendVisitor Visitor;
};

#endif // ADD_FRIEND_CONSUMER_H