#include "WrapFunctionCallback.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"

WrapFunctionCallback::WrapFunctionCallback(clang::Rewriter &Rewrite,
                                           llvm::StringRef Id)
    : Rewrite(Rewrite), Id(Id) {}

void WrapFunctionCallback::run(
    const clang::ast_matchers::MatchFinder::MatchResult &Result) {
  const clang::FunctionDecl *Func =
      Result.Nodes.getNodeAs<clang::FunctionDecl>(Id);
  if (!Func || Func->isImplicit())
    return;

  processFunction(Func, Result.Context);
}
void WrapFunctionCallback::processFunction(const clang::FunctionDecl *Func,
                                           clang::ASTContext *Context) {
  // Check if function is in base folder
  if (!isInBaseFolder(Func->getLocation(), Context->getSourceManager())) {
    return;
  }
  
  std::string OriginalName = Func->getNameAsString();
  std::string WrappedName = OriginalName + "__wrapped__";

  bool IsDefinition = Func->isThisDeclarationADefinition();

  // Determine if the function is a method, and gather method-specific info
  const clang::CXXMethodDecl *Method =
      llvm::dyn_cast<clang::CXXMethodDecl>(Func);
  bool IsMethod = (Method != nullptr);
  bool IsVirtual = false;
  bool IsConstMethod = false;
  bool IsStaticMethod = false;
  std::string ClassName;
  std::string QualifiedName = OriginalName;
  std::string QualifiedNameUnderbar = OriginalName;

  if (IsMethod) {
    IsVirtual = Method->isVirtual();
    IsConstMethod = Method->isConst();
    IsStaticMethod = Method->isStatic();

    const clang::CXXRecordDecl *ClassDecl = Method->getParent();
    ClassName = ClassDecl->getNameAsString();
    QualifiedName = ClassName + "::" + OriginalName;
    QualifiedNameUnderbar = ClassName + "____" + OriginalName;
  }

  // Rename the original function
  clang::SourceLocation ActualImpFuncNameLoc = Func->getLocation();
  unsigned NameLength = OriginalName.length();
  Rewrite.ReplaceText(ActualImpFuncNameLoc, NameLength, WrappedName);

  // Insert the wrapper function after the original function
  clang::SourceLocation InsertLoc;
  if (IsDefinition) {
    InsertLoc = Func->getSourceRange().getEnd().getLocWithOffset(1);
  } else {
    InsertLoc = Func->getSourceRange().getEnd();
  }

  // Remove 'virtual' keyword if necessary
  if (IsVirtual) {
    clang::SourceRange FuncRange = Func->getSourceRange();
    clang::StringRef FuncText = clang::Lexer::getSourceText(
        clang::CharSourceRange::getCharRange(FuncRange),
        Context->getSourceManager(), Context->getLangOpts());

    size_t VirtualPos = FuncText.find("virtual");
    if (VirtualPos != std::string::npos) {
      clang::SourceLocation VirtualLoc =
          FuncRange.getBegin().getLocWithOffset(VirtualPos);
      Rewrite.RemoveText(VirtualLoc, strlen("virtual "));
    }
  }

  // Insert inline assembly markers in the wrapped function
  if (IsDefinition && Func->hasBody()) {
    clang::SourceLocation FuncStart =
        Func->getBody()->getBeginLoc().getLocWithOffset(1);
    clang::SourceLocation FuncEnd = Func->getBody()->getEndLoc();
    std::string AsmStart = "asm volatile(\".global " + QualifiedNameUnderbar +
                           "_start_\\n" + QualifiedNameUnderbar +
                           "_start_:\");\n";
    Rewrite.InsertText(FuncStart, AsmStart, true, true);

    std::string AsmEnd = "asm volatile(\".global " + QualifiedNameUnderbar +
                         "_end_\\n" + QualifiedNameUnderbar + "_end_:\");\n";
    Rewrite.InsertText(FuncEnd, AsmEnd, true, true);
  }

  std::string CurrentFunctionSectionName = "";
  for (const auto *Attr : Func->attrs()) {
    if (const auto *SA =
            llvm::dyn_cast<clang::PragmaClangTextSectionAttr>(Attr)) {
      CurrentFunctionSectionName = SA->getName();
    }
  }

  // Wrap the function definition in #pragma clang section
  std::string PragmaStart, PragmaEnd;
  PragmaStart = "\n#pragma clang section text=\"advice\"\n";
  PragmaEnd =
      "#pragma clang section text=\"" + CurrentFunctionSectionName + "\"\n";

  bool QualifiedNameIsOriginal = true;

  if (IsMethod) {
    const clang::CXXRecordDecl *ClassDecl = Method->getParent();

    // Insert the declaration inside the class
    clang::SourceLocation ClassInsertLoc;
    bool Inserted = false;
    for (const clang::Decl *D : ClassDecl->decls()) {
      if (const clang::AccessSpecDecl *AS =
              llvm::dyn_cast<clang::AccessSpecDecl>(D)) {
        if (AS->getAccess() == Method->getAccess()) {
          ClassInsertLoc = AS->getEndLoc().getLocWithOffset(1);
          Inserted = true;
          break;
        }
      }
    }
    // Insert the wrapper function definition after the class definition
    clang::SourceLocation ClassEndLoc =
        ClassDecl->getBraceRange().getEnd().getLocWithOffset(0);
    // if ClassEndLoc is not valid or the ClassInsertLoc is after  ClassEndLoc
    bool isOutsideOfClass = InsertLoc > ClassEndLoc;

    if (!Inserted) {
      ClassInsertLoc = ClassDecl->getBraceRange().getEnd();

      // Build the function declaration
      std::string WrapperDecl;
      llvm::raw_string_ostream WrapperDeclStream(WrapperDecl);

      if (IsVirtual)
        WrapperDecl += "virtual ";
      WrapperDecl +=
          Func->getReturnType().getAsString() + " " + WrappedName + "(";
      for (unsigned i = 0; i < Func->getNumParams(); ++i) {
        if (i > 0)
          WrapperDecl += ", ";
        WrapperDecl += Func->getParamDecl(i)->getType().getAsString();
        WrapperDecl += " " + Func->getParamDecl(i)->getNameAsString();
      }
      WrapperDecl += ")";
      if (IsConstMethod)
        WrapperDecl += " const";
      if (Method->isPureVirtual())
        WrapperDecl += " = 0";
      WrapperDecl += ";\n";

      if (isOutsideOfClass) {
        Rewrite.InsertText(ClassInsertLoc, WrapperDecl, true, true);
        QualifiedNameIsOriginal = false;
      } else {
      }
    }

    // If the method is pure virtual, no need to define it
    if (Method->isPureVirtual())
      return;
  }
  QualifiedName = (QualifiedNameIsOriginal) ? OriginalName : QualifiedName;
  // Build the wrapper function code
  std::string NewFunc = buildWrapperFunction(
      Func, OriginalName, WrappedName, QualifiedName, QualifiedNameUnderbar, IsMethod, IsVirtual,
      IsConstMethod, IsStaticMethod, ClassName, Id);
  std::string WrapperFuncCode = PragmaStart + NewFunc + PragmaEnd;
  Rewrite.InsertTextAfter(InsertLoc, WrapperFuncCode);
}

std::string WrapFunctionCallback::buildWrapperFunction(
    const clang::FunctionDecl *Func, const std::string &OriginalName,
    const std::string &WrappedName, const std::string &QualifiedName, const std::string &QualifiedNameUnderbar,
    bool IsMethod, bool IsVirtual, bool IsConstMethod, bool IsStaticMethod,
    const std::string &ClassName, const llvm::StringRef Id) {
  std::string NewFunc;
  llvm::raw_string_ostream OS(NewFunc);

  // Build the function signature
  std::string ReturnType = Func->getReturnType().getAsString();
  OS << ReturnType << " ";

  if (IsMethod) {
    OS << QualifiedName;
  } else {
    OS << OriginalName;
  }

  OS << "(";

  // Parameters
  for (unsigned i = 0; i < Func->getNumParams(); ++i) {
    if (i > 0)
      OS << ", ";
    OS << Func->getParamDecl(i)->getType().getAsString();
    OS << " " << Func->getParamDecl(i)->getNameAsString();
  }
  OS << ")";

  if (IsMethod && IsConstMethod)
    OS << " const";

  OS << " {\n";

  // Declare external symbols
  OS << "    extern char " << QualifiedNameUnderbar << "_start_;\n";
  OS << "    extern char " << QualifiedNameUnderbar << "_end_;\n";

  // Create Pointcut
  OS << "    auto pc = ";
  if (IsMethod) {
    if (IsStaticMethod) {
      // Static member function
      OS << "createPointcut(&" << ClassName << "::" << WrappedName << ", &"
         << QualifiedNameUnderbar << "_start_, &" << QualifiedNameUnderbar << "_end_);\n";
    } else {
      // Non-static member function
      OS << "createPointcut(&" << ClassName << "::" << WrappedName
         << ", this, &" << QualifiedNameUnderbar << "_start_, &" << QualifiedNameUnderbar
         << "_end_);\n";
    }
  } else {
    // Non-method function
    OS << "createPointcut(&" << WrappedName << ", &" << QualifiedNameUnderbar
       << "_start_, &" << QualifiedNameUnderbar << "_end_);\n";
  }

  // Call around function
  OS << "    ";
  if (!Func->getReturnType()->isVoidType()) {
    OS << "return ";
  }
  OS << "pc.template around<PointcutName::" << Id << ">(";
  for (unsigned i = 0; i < Func->getNumParams(); ++i) {
    if (i > 0)
      OS << ", ";
    OS << "std::forward<" << Func->getParamDecl(i)->getType().getAsString()
       << ">(" << Func->getParamDecl(i)->getNameAsString() << ")";
  }
  OS << ");\n";

  OS << "}\n";

  return OS.str();
}

void WrapFunctionCallback::setBaseFolder(const std::string &Folder) {
  BaseFolder = Folder;
}

bool WrapFunctionCallback::isInBaseFolder(clang::SourceLocation Loc, clang::SourceManager &SM) {
  if (BaseFolder.empty()) {
    return true; // No base folder specified, process all files
  }
  
  if (!Loc.isValid() || !SM.isInMainFile(Loc)) {
    return false;
  }
  
  std::string Filename = SM.getFilename(Loc).str();
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
