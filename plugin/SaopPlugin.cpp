#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

class WrapFunctionCallback : public MatchFinder::MatchCallback {
public:
  WrapFunctionCallback(Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    const FunctionDecl *Func = Result.Nodes.getNodeAs<FunctionDecl>("funcDecl");
    if (!Func || Func->isImplicit() || !Func->hasAttr<AnnotateAttr>())
      return;

    // Check for 'wrap' attribute
    for (auto *Attr : Func->getAttrs()) {
      if (const auto *Annotate = dyn_cast<AnnotateAttr>(Attr)) {
        if (Annotate->getAnnotation() == "wrap") {
          processFunction(Func, Result.Context);
          break;
        }
      }
    }
  }

private:
  Rewriter &Rewrite;

  void processFunction(const FunctionDecl *Func, ASTContext *Context) {
    std::string OriginalName = Func->getNameAsString();
    std::string WrappedName = OriginalName + "__wrapped__";

    // Rename the original function
    SourceLocation NameLoc = Func->getNameInfo().getLoc();
    Rewrite.ReplaceText(NameLoc, OriginalName.length(), WrappedName);

    // Insert inline assembly markers in the wrapped function
    SourceLocation FuncStart =
        Func->getBody()->getBeginLoc().getLocWithOffset(1);
    SourceLocation FuncEnd = Func->getBody()->getEndLoc();

    std::string AsmStart = "asm volatile(\".global " + WrappedName +
                           "_start_\\n" + WrappedName + "_start_:\");\n";
    Rewrite.InsertText(FuncStart, AsmStart, true, true);

    std::string AsmEnd = "asm volatile(\".global " + WrappedName + "_end_\\n" +
                         WrappedName + "_end_:\");\n";
    Rewrite.InsertText(FuncEnd, AsmEnd, true, true);

    // Build the new function declaration
    std::string NewFunc;
    llvm::raw_string_ostream OS(NewFunc);

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
    OS << "    auto pc = createPointcut(" << WrappedName << ", &"
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
      // example : std::forward<int>(x)
      OS << "std::forward<" << Func->getParamDecl(i)->getType().getAsString() << ">(" << Func->getParamDecl(i)->getNameAsString() << ")";
    }
    OS << ");\n";

    OS << "}\n";

    // Insert the new function after the original function
    SourceLocation InsertLoc =
        Func->getSourceRange().getEnd().getLocWithOffset(1);
    Rewrite.InsertTextAfter(InsertLoc, OS.str());
  }
};

class WrapFunctionConsumer : public ASTConsumer {
public:
  WrapFunctionConsumer(Rewriter &R) : Handler(R) {
    Matcher.addMatcher(functionDecl(isExpansionInMainFile()).bind("funcDecl"),
                       &Handler);
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    Matcher.matchAST(Context);
  }

private:
  WrapFunctionCallback Handler;
  MatchFinder Matcher;
};

class WrapFunctionAction : public PluginASTAction {
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) override {
    Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<WrapFunctionConsumer>(Rewrite);
  }

  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &) override {
    return true;
  }

  void EndSourceFileAction() override {
    SourceManager &SM = Rewrite.getSourceMgr();
    llvm::outs() << std::string(
        Rewrite.getRewriteBufferFor(SM.getMainFileID())->begin(),
        Rewrite.getRewriteBufferFor(SM.getMainFileID())->end());
  }

private:
  Rewriter Rewrite;
};

static FrontendPluginRegistry::Add<WrapFunctionAction>
    X("saop-plugin",
      "Rename functions with 'wrap' attribute and create wrapper functions");
