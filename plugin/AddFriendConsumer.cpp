#include "AddFriendConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <sstream>
#include <regex>

std::string FriendTemplate::instantiate(const std::string& namespaceName, 
                                       const std::string& className) const {
  std::string result = templateText;
  
  // Replace {namespace} with actual namespace
  size_t pos = 0;
  while ((pos = result.find("{namespace}", pos)) != std::string::npos) {
    result.replace(pos, 11, namespaceName);
    pos += namespaceName.length();
  }
  
  // Replace {class-name} with actual class name
  pos = 0;
  while ((pos = result.find("{class-name}", pos)) != std::string::npos) {
    result.replace(pos, 12, className);
    pos += className.length();
  }
  
  return result;
}

AddFriendVisitor::AddFriendVisitor(clang::Rewriter &Rewrite, const std::string &BaseFolder,
                                   const std::vector<FriendTemplate> &CustomFriends)
    : Rewrite(Rewrite), BaseFolder(BaseFolder), CustomFriends(CustomFriends), SM(nullptr) {}

bool AddFriendVisitor::isInBaseFolder(clang::SourceLocation Loc) {
  if (BaseFolder.empty() || !SM) {
    return true; // No base folder specified, process all files
  }
  
  if (!Loc.isValid() || !SM->isInMainFile(Loc)) {
    return false;
  }
  
  clang::FullSourceLoc FullLoc(Loc, *SM);
  if (!FullLoc.isValid()) {
    return false;
  }
  
  std::string Filename = SM->getFilename(Loc).str();
  if (Filename.empty()) {
    return false;
  }
  
  // Convert to absolute path if needed
  llvm::SmallString<256> AbsPath(Filename);
  if (!llvm::sys::path::is_absolute(AbsPath)) {
    if (std::error_code EC = llvm::sys::fs::make_absolute(AbsPath)) {
      return false;
    }
  }
  
  // Check if the file is under the base folder
  return AbsPath.str().starts_with(BaseFolder);
}

std::string AddFriendVisitor::getNamespaceString(const clang::DeclContext *DC) {
  std::vector<std::string> namespaces;
  
  // Walk up the DeclContext chain to collect namespaces
  while (DC) {
    if (const auto *NS = llvm::dyn_cast<clang::NamespaceDecl>(DC)) {
      if (!NS->isAnonymousNamespace()) {
        namespaces.push_back(NS->getNameAsString());
      }
    }
    DC = DC->getParent();
  }
  
  // Build the namespace string (in reverse order since we walked up)
  std::string result;
  for (auto it = namespaces.rbegin(); it != namespaces.rend(); ++it) {
    if (!result.empty()) {
      result += "::";
    }
    result += *it;
  }
  
  return result;
}

void AddFriendVisitor::addFriendDeclarations(clang::CXXRecordDecl *D) {
  // Get the class name and namespace
  std::string className = D->getNameAsString();
  std::string namespaceName = getNamespaceString(D->getDeclContext());
  
  // Build the friend declarations
  std::string friendDecls;
  
  // Add default friend classes
  // 1. {ClassName}Test
  friendDecls += "  friend class " + className + "Test;\n";
  
  // 2. template<typename T> friend class {ClassName}Friend
  friendDecls += "  template<typename T> friend class " + className + "Friend;\n";
  
  // Add custom friend classes
  for (const auto &friendTemplate : CustomFriends) {
    std::string friendDecl = friendTemplate.instantiate(namespaceName, className);
    friendDecls += "  friend " + friendDecl + ";\n";
  }
  
  // Find where to insert the friend declarations
  // We'll insert them right after the opening brace of the class
  clang::SourceLocation InsertLoc = D->getBraceRange().getBegin();
  if (InsertLoc.isValid()) {
    // Move past the opening brace
    InsertLoc = InsertLoc.getLocWithOffset(1);
    
    // Insert a newline first if needed, then the friend declarations
    Rewrite.InsertTextAfter(InsertLoc, "\n" + friendDecls);
  }
}

bool AddFriendVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *D) {
  if (!D || !SM) {
    return true;
  }
  
  // Skip if not in base folder
  if (!isInBaseFolder(D->getLocation())) {
    return true;
  }
  
  // Only process class/struct definitions (not forward declarations)
  if (!D->isThisDeclarationADefinition()) {
    return true;
  }
  
  // Skip template specializations, process only primary templates or non-template classes
  if (D->getTemplateSpecializationKind() != clang::TSK_Undeclared &&
      D->getTemplateSpecializationKind() != clang::TSK_ExplicitSpecialization) {
    return true;
  }
  
  // Add friend declarations
  addFriendDeclarations(D);
  
  return true;
}

AddFriendConsumer::AddFriendConsumer(clang::Rewriter &Rewrite, const std::string &BaseFolder,
                                     const std::vector<FriendTemplate> &CustomFriends)
    : Visitor(Rewrite, BaseFolder, CustomFriends) {}

void AddFriendConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
  Visitor.SM = &Context.getSourceManager();
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}