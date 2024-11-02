#pragma once

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Rewrite/Core/Rewriter.h"

class WrapFunctionCallback : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  WrapFunctionCallback(clang::Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {
    const clang::FunctionDecl *Func = Result.Nodes.getNodeAs<clang::FunctionDecl>("funcDecl");
    if (!Func || Func->isImplicit())
      return;
    //llvm::outs() << "Processing function: " << Func->getNameAsString() << "\n";
    // Check for 'wrap' attribute
    processFunction(Func, Result.Context);
  }

private:
  clang::Rewriter &Rewrite;

  void processFunction(const clang::FunctionDecl *Func, clang::ASTContext *Context) {
    std::string OriginalName = Func->getNameAsString();
    std::string WrappedName = OriginalName + "__wrapped__";

    bool IsDefinition = Func->isThisDeclarationADefinition();

    // Check if the function is virtual
    bool IsVirtual = false;
    if (const clang::CXXMethodDecl *Method = llvm::dyn_cast<clang::CXXMethodDecl>(Func)) {
      IsVirtual = Method->isVirtual();
    }

    // Rename the original function in its declaration
    clang::SourceLocation NameLoc = Func->getLocation();
    unsigned NameLength = OriginalName.length();
    Rewrite.ReplaceText(NameLoc, NameLength, WrappedName);

    // Remove the 'virtual' keyword from the original function
    if (IsVirtual) {
      // Find the 'virtual' keyword in the source code
      clang::SourceRange FuncRange = Func->getSourceRange();
      clang::StringRef FuncText = clang::Lexer::getSourceText(
          clang::CharSourceRange::getCharRange(FuncRange),
          Context->getSourceManager(), Context->getLangOpts());

      size_t VirtualPos = FuncText.find("virtual");
      if (VirtualPos != std::string::npos) {
        clang::SourceLocation VirtualLoc = FuncRange.getBegin().getLocWithOffset(VirtualPos);
        Rewrite.RemoveText(VirtualLoc, strlen("virtual "));
      }
    }

    if (IsDefinition && Func->hasBody()) {
      // Insert inline assembly markers in the wrapped function
      clang::SourceLocation FuncStart = Func->getBody()->getBeginLoc().getLocWithOffset(1);
      clang::SourceLocation FuncEnd = Func->getBody()->getEndLoc();

      std::string AsmStart = "asm volatile(\".global " + WrappedName +
                             "_start_\\n" + WrappedName + "_start_:\");\n";
      Rewrite.InsertText(FuncStart, AsmStart, true, true);

      std::string AsmEnd = "asm volatile(\".global " + WrappedName + "_end_\\n" +
                           WrappedName + "_end_:\");\n";
      Rewrite.InsertText(FuncEnd, AsmEnd, true, true);
    }

    // Build the new wrapper function
    std::string NewFunc;
    llvm::raw_string_ostream OS(NewFunc); // Fully qualified 'llvm::raw_string_ostream'

    if (const clang::CXXMethodDecl *Method = llvm::dyn_cast<clang::CXXMethodDecl>(Func)) {
      // For class methods, declare the wrapper inside the class
      const clang::CXXRecordDecl *ClassDecl = Method->getParent();
      std::string ClassName = ClassDecl->getNameAsString();
      std::string QualifiedName = ClassName + "::" + OriginalName;

      // Insert the declaration inside the class
      clang::SourceLocation InsertLoc;
      bool Inserted = false;
      for (const clang::Decl *D : ClassDecl->decls()) {
        if (const clang::AccessSpecDecl *AS = llvm::dyn_cast<clang::AccessSpecDecl>(D)) {
          if (AS->getAccess() == Method->getAccess()) {
            InsertLoc = AS->getEndLoc().getLocWithOffset(1);
            Inserted = true;
            break;
          }
        }
      }
      if (!Inserted) {
        // If no matching access specifier found, insert at the end
        InsertLoc = ClassDecl->getBraceRange().getEnd();
      }

      // Build the function declaration
      std::string WrapperDecl;
      if (IsVirtual)
        WrapperDecl += "virtual ";
      WrapperDecl += Func->getReturnType().getAsString() + " " +
                     OriginalName + "(";
      for (unsigned i = 0; i < Func->getNumParams(); ++i) {
        if (i > 0)
          WrapperDecl += ", ";
        WrapperDecl += Func->getParamDecl(i)->getType().getAsString();
        WrapperDecl += " " + Func->getParamDecl(i)->getNameAsString();
      }
      WrapperDecl += ")";
      if (Method->isConst())
        WrapperDecl += " const";
      if (Method->isPureVirtual()) // Updated to 'isPureVirtual()'
        WrapperDecl += " = 0";
      WrapperDecl += ";\n";

      Rewrite.InsertText(InsertLoc, WrapperDecl, true, true);

      // If the method is pure virtual, we don't need to define it
      if (Method->isPureVirtual()) // Updated to 'isPureVirtual()'
        return;

      // Now, define the wrapper function outside the class
      OS << Func->getReturnType().getAsString() << " " << QualifiedName << "(";
      for (unsigned i = 0; i < Func->getNumParams(); ++i) {
        if (i > 0)
          OS << ", ";
        OS << Func->getParamDecl(i)->getType().getAsString();
        OS << " " << Func->getParamDecl(i)->getNameAsString();
      }
      OS << ")";
      if (Method->isConst())
        OS << " const";
      OS << " {\n";

      // Declare external symbols
      OS << "    extern char " << WrappedName << "_start_;\n";
      OS << "    extern char " << WrappedName << "_end_;\n"; 
 
      // Create Pointcut based on whether it's a member function
      OS << "    auto pc = ";
      if (Method->isStatic()) {
        // Static member function (treated like free functions)
        OS << "createPointcut(" << "&" << ClassName << "::" << WrappedName << ", &"
           << WrappedName << "_start_, &" << WrappedName << "_end_);\n";
      } else {
        // Non-static member function
        OS << "createPointcut(&" << ClassName << "::" << WrappedName << ", this, &"
           << WrappedName << "_start_, &" << WrappedName << "_end_);\n";
      }

      // Call around function
      OS << "    ";
      if (!Func->getReturnType()->isVoidType()) {
        OS << "return ";
      }
      OS << "pc.around(";
      for (unsigned i = 0; i < Func->getNumParams(); ++i) {
        if (i > 0)
          OS << ", ";
        OS << "std::forward<"
           << Func->getParamDecl(i)->getType().getAsString() << ">("
           << Func->getParamDecl(i)->getNameAsString() << ")";
      }
      OS << ");\n";

      OS << "}\n";

      // Insert the new function definition after the class definition
      clang::SourceLocation ClassEndLoc = ClassDecl->getBraceRange().getEnd().getLocWithOffset(1);
      Rewrite.InsertTextAfter(ClassEndLoc, OS.str());

    } else {
      // Non-method function
      // New function signature
      OS << Func->getReturnType().getAsString() << " " << OriginalName << "(";

      // Parameters
      for (unsigned i = 0; i < Func->getNumParams(); ++i) {
        if (i > 0)
          OS << ", ";
        OS << Func->getParamDecl(i)->getType().getAsString();
        OS << " " << Func->getParamDecl(i)->getNameAsString();
      }
      OS << ") {\n";
      // Declare external symbols
      OS << "    extern char " << WrappedName << "_start_;\n";
      OS << "    extern char " << WrappedName << "_end_;\n";
      // Create Pointcut
      OS << "    auto pc = createPointcut(&" << WrappedName << ", &"
         << WrappedName << "_start_, &" << WrappedName << "_end_);\n";

      // Call around function
      OS << "    ";
      if (!Func->getReturnType()->isVoidType()) {
        OS << "return ";
      }
      OS << "pc.around(";
      for (unsigned i = 0; i < Func->getNumParams(); ++i) {
        if (i > 0)
          OS << ", ";
        OS << "std::forward<"
           << Func->getParamDecl(i)->getType().getAsString() << ">("
           << Func->getParamDecl(i)->getNameAsString() << ")";
      }
      OS << ");\n";

      OS << "}\n";

      // Insert the new function after the original function
      clang::SourceLocation InsertLoc;
      if (IsDefinition) {
        InsertLoc = Func->getSourceRange().getEnd().getLocWithOffset(1);
      } else {
        InsertLoc = Func->getSourceRange().getEnd();
      }
      Rewrite.InsertTextAfter(InsertLoc, OS.str());
    }
  }
};
