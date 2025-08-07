#include "UnifiedASTVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <sstream>
#include <regex>

// FriendTemplate implementation
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

UnifiedASTVisitor::UnifiedASTVisitor(clang::Rewriter &Rewrite, const std::string &BaseFolder,
                                     bool EnableRemoveFinal, bool EnableMakeVirtual, 
                                     bool EnableAddFriend, const std::vector<FriendTemplate> &CustomFriends)
    : Rewrite(Rewrite), BaseFolder(BaseFolder), SM(nullptr),
      EnableRemoveFinal(EnableRemoveFinal), EnableMakeVirtual(EnableMakeVirtual),
      EnableAddFriend(EnableAddFriend), CustomFriends(CustomFriends) {}

bool UnifiedASTVisitor::isInBaseFolder(clang::SourceLocation Loc) {
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

bool UnifiedASTVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *D) {
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
  
  // Process RemoveFinal feature
  if (EnableRemoveFinal) {
    processRemoveFinalForClass(D);
  }
  
  // Process AddFriend feature
  if (EnableAddFriend) {
    // Skip template specializations, process only primary templates or non-template classes
    if (D->getTemplateSpecializationKind() == clang::TSK_Undeclared ||
        D->getTemplateSpecializationKind() == clang::TSK_ExplicitSpecialization) {
      addFriendDeclarations(D);
    }
  }
  
  return true;
}

bool UnifiedASTVisitor::VisitCXXMethodDecl(clang::CXXMethodDecl *D) {
  if (!D || !SM) {
    return true;
  }
  
  // Skip if not in base folder
  if (!isInBaseFolder(D->getLocation())) {
    return true;
  }
  
  // Process RemoveFinal feature
  if (EnableRemoveFinal && D->isVirtual()) {
    processRemoveFinalForMethod(D);
  }
  
  // Process MakeVirtual feature
  if (EnableMakeVirtual) {
    // Skip template instantiations, process only the template pattern or non-template methods
    if (!D->isTemplateInstantiation() && canBeMadeVirtual(D)) {
      makeMethodVirtual(D);
    }
  }
  
  return true;
}

// RemoveFinal feature methods
void UnifiedASTVisitor::removeFinalFromRange(clang::SourceRange Range) {
  if (!Range.isValid()) {
    return;
  }
  
  // Get the source text for the range
  clang::StringRef Text = clang::Lexer::getSourceText(
      clang::CharSourceRange::getTokenRange(Range),
      *SM, clang::LangOptions());
  
  // Search for "final" keyword
  size_t FinalPos = Text.find("final");
  if (FinalPos != std::string::npos) {
    // Calculate the actual location of "final"
    clang::SourceLocation FinalLoc = Range.getBegin().getLocWithOffset(FinalPos);
    
    // Remove "final" and any surrounding whitespace
    size_t RemoveStart = FinalPos;
    size_t RemoveLen = 5; // "final" is 5 characters
    
    // Check for whitespace before "final"
    while (RemoveStart > 0 && std::isspace(Text[RemoveStart - 1])) {
      RemoveStart--;
      RemoveLen++;
    }
    
    // Check for whitespace after "final"
    size_t EndPos = FinalPos + 5;
    while (EndPos < Text.size() && std::isspace(Text[EndPos])) {
      RemoveLen++;
      EndPos++;
    }
    
    clang::SourceLocation RemoveLoc = Range.getBegin().getLocWithOffset(RemoveStart);
    Rewrite.RemoveText(RemoveLoc, RemoveLen);
  }
}

void UnifiedASTVisitor::processRemoveFinalForClass(clang::CXXRecordDecl *D) {
  // For classes, we need to search for final keyword in the declaration
  // Get the range from "class" keyword to the opening brace
  clang::SourceLocation ClassLoc = D->getBeginLoc();
  clang::SourceLocation BraceLoc = D->getBraceRange().getBegin();
  
  if (ClassLoc.isValid() && BraceLoc.isValid()) {
    clang::SourceRange SearchRange(ClassLoc, BraceLoc);
    removeFinalFromRange(SearchRange);
  }
}

void UnifiedASTVisitor::processRemoveFinalForMethod(clang::CXXMethodDecl *D) {
  // For virtual functions, we need to check for the final keyword
  // The final keyword typically appears after the function declaration
  clang::SourceRange DeclRange = D->getSourceRange();
  
  // For methods, final usually appears after the parameter list and qualifiers
  // We need to search from the end of parameters to the end of the declaration
  if (D->doesThisDeclarationHaveABody()) {
    // If there's a body, search up to the opening brace
    clang::SourceLocation BodyStart = D->getBody()->getBeginLoc();
    clang::SourceRange SearchRange(D->getLocation(), BodyStart);
    removeFinalFromRange(SearchRange);
  } else {
    // For pure virtual or declarations without body
    removeFinalFromRange(DeclRange);
  }
}

// MakeVirtual feature methods
bool UnifiedASTVisitor::canBeMadeVirtual(clang::CXXMethodDecl *D) {
  // Already virtual
  if (D->isVirtual()) {
    return false;
  }
  
  // Static methods cannot be virtual
  if (D->isStatic()) {
    return false;
  }
  
  // Constructors cannot be virtual
  if (llvm::isa<clang::CXXConstructorDecl>(D)) {
    return false;
  }
  
  // Destructors are handled separately (they can be virtual but have special syntax)
  if (llvm::isa<clang::CXXDestructorDecl>(D)) {
    return true; // We'll handle destructors specially
  }
  
  // Friend functions cannot be virtual
  if (D->getFriendObjectKind() != clang::Decl::FOK_None) {
    return false;
  }
  
  // Template methods have restrictions but can generally be made virtual
  // if they're not template themselves (only the class is template)
  if (D->getTemplatedKind() == clang::FunctionDecl::TK_FunctionTemplate) {
    return false;
  }
  
  // Deleted or defaulted functions can be virtual
  // Inline functions can be virtual
  // Const/non-const methods can be virtual
  
  return true;
}

void UnifiedASTVisitor::makeMethodVirtual(clang::CXXMethodDecl *D) {
  // Get the source location of the method
  clang::SourceLocation StartLoc = D->getBeginLoc();
  
  // Check if "virtual" keyword already exists by examining the source text
  clang::SourceRange DeclRange(StartLoc, D->getLocation());
  clang::StringRef DeclText = clang::Lexer::getSourceText(
      clang::CharSourceRange::getTokenRange(DeclRange),
      *SM, clang::LangOptions());
  
  // If "virtual" is already present in the declaration text, skip
  if (DeclText.contains("virtual")) {
    return;
  }
  
  // For destructors, we need to find the ~ symbol
  if (llvm::isa<clang::CXXDestructorDecl>(D)) {
    // Get the method name location (this should be at the destructor name)
    clang::SourceLocation NameLoc = D->getLocation();
    
    // We need to insert "virtual " before the destructor
    // First, we need to find where to insert it
    clang::SourceLocation InsertLoc = StartLoc;
    
    // Check if there are any specifiers before the destructor
    if (D->isInlineSpecified()) {
      // If inline is specified, we might need to find it
      // For now, just insert at the beginning
    }
    
    Rewrite.InsertTextBefore(InsertLoc, "virtual ");
    return;
  }
  
  // For regular methods, insert "virtual " at the beginning
  // We need to handle various cases like:
  // - inline void foo()
  // - static void foo() // This shouldn't happen due to canBeMadeVirtual
  // - void foo()
  // - explicit operator bool()
  // - etc.
  
  // Find the location to insert "virtual"
  clang::SourceLocation InsertLoc = StartLoc;
  
  // If the method has explicit inline specifier, we want to insert before it
  if (D->isInlineSpecified()) {
    // The inline keyword should be at or near the start
    // For simplicity, we'll just insert at the beginning
  }
  
  // Handle explicit specifier for conversion operators
  // Note: In older Clang versions, we need to check if it's a conversion function
  if (llvm::isa<clang::CXXConversionDecl>(D)) {
    auto *ConvDecl = llvm::cast<clang::CXXConversionDecl>(D);
    if (ConvDecl->isExplicit()) {
      // Similar handling - insert at the beginning
    }
  }
  
  // Insert "virtual " at the beginning of the declaration
  Rewrite.InsertTextBefore(InsertLoc, "virtual ");
}

// AddFriend feature methods
std::string UnifiedASTVisitor::getNamespaceString(const clang::DeclContext *DC) {
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

void UnifiedASTVisitor::addFriendDeclarations(clang::CXXRecordDecl *D) {
  // Check if the class already has friend declarations (to avoid duplicates)
  // This is a simple check - in a real implementation, we'd parse existing friends
  for (auto *Friend : D->friends()) {
    // If the class already has friends, skip adding default ones
    // This is a simplified check - could be more sophisticated
    return;
  }
  
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

// UnifiedASTConsumer implementation
UnifiedASTConsumer::UnifiedASTConsumer(clang::Rewriter &Rewrite, const std::string &BaseFolder,
                                       bool EnableRemoveFinal, bool EnableMakeVirtual, 
                                       bool EnableAddFriend, const std::vector<FriendTemplate> &CustomFriends)
    : Visitor(Rewrite, BaseFolder, EnableRemoveFinal, EnableMakeVirtual, 
              EnableAddFriend, CustomFriends) {}

void UnifiedASTConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
  Visitor.setSourceManager(&Context.getSourceManager());
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}