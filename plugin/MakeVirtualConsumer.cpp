#include "MakeVirtualConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

MakeVirtualVisitor::MakeVirtualVisitor(clang::Rewriter &Rewrite, const std::string &BaseFolder)
    : Rewrite(Rewrite), BaseFolder(BaseFolder), SM(nullptr) {}

bool MakeVirtualVisitor::isInBaseFolder(clang::SourceLocation Loc) {
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

bool MakeVirtualVisitor::canBeMadeVirtual(clang::CXXMethodDecl *D) {
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

void MakeVirtualVisitor::makeMethodVirtual(clang::CXXMethodDecl *D) {
  // Get the source location of the method
  clang::SourceLocation StartLoc = D->getBeginLoc();
  
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

bool MakeVirtualVisitor::VisitCXXMethodDecl(clang::CXXMethodDecl *D) {
  if (!D || !SM) {
    return true;
  }
  
  // Skip if not in base folder
  if (!isInBaseFolder(D->getLocation())) {
    return true;
  }
  
  // Skip template instantiations, process only the template pattern or non-template methods
  if (D->isTemplateInstantiation()) {
    return true;
  }
  
  // Check if this method can be made virtual
  if (canBeMadeVirtual(D)) {
    makeMethodVirtual(D);
  }
  
  return true;
}

MakeVirtualConsumer::MakeVirtualConsumer(clang::Rewriter &Rewrite, const std::string &BaseFolder)
    : Visitor(Rewrite, BaseFolder) {}

void MakeVirtualConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
  Visitor.SM = &Context.getSourceManager();
  Visitor.TraverseDecl(Context.getTranslationUnitDecl());
}