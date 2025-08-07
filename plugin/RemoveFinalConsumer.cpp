#include "RemoveFinalConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

RemoveFinalVisitor::RemoveFinalVisitor(clang::Rewriter &Rewrite, const std::string &BaseFolder)
    : Rewrite(Rewrite), BaseFolder(BaseFolder), SM(nullptr) {}

bool RemoveFinalVisitor::isInBaseFolder(clang::SourceLocation Loc) {
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

void RemoveFinalVisitor::removeFinalFromRange(clang::SourceRange Range) {
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

bool RemoveFinalVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl *D) {
  if (!D || !SM) {
    return true;
  }
  
  // Skip if not in base folder
  if (!isInBaseFolder(D->getLocation())) {
    return true;
  }
  
  // Only process class definitions (not forward declarations)
  if (!D->isThisDeclarationADefinition()) {
    return true;
  }
  
  // For classes, we need to search for final keyword in the declaration
  // Get the range from "class" keyword to the opening brace
  clang::SourceLocation ClassLoc = D->getBeginLoc();
  clang::SourceLocation BraceLoc = D->getBraceRange().getBegin();
  
  if (ClassLoc.isValid() && BraceLoc.isValid()) {
    clang::SourceRange SearchRange(ClassLoc, BraceLoc);
    removeFinalFromRange(SearchRange);
  }
  
  return true;
}

bool RemoveFinalVisitor::VisitCXXMethodDecl(clang::CXXMethodDecl *D) {
  if (!D || !SM) {
    return true;
  }
  
  // Skip if not in base folder
  if (!isInBaseFolder(D->getLocation())) {
    return true;
  }
  
  // Check if method is virtual (final only applies to virtual methods)
  if (D->isVirtual()) {
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
  
  return true;
}

RemoveFinalConsumer::RemoveFinalConsumer(clang::Rewriter &Rewrite, const std::string &BaseFolder)
    : Visitor(Rewrite, BaseFolder) {}

void RemoveFinalConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
  Visitor.SM = &Context.getSourceManager();
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}