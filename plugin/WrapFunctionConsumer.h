#pragma once
#include "WrapFunctionCallback.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include <memory>
#include <fstream>

#include "AST2Matcher.h"
#include "Lexer.h"
#include "Parser.h"

// Custom matcher to check for an AnnotateAttr with a specific annotation value
AST_MATCHER_P(clang::FunctionDecl, _hasAnnotateAttrWithValue, std::string,
              AnnotationValue) {
  for (const auto *Attr : Node.attrs()) {
    if (const auto *Annotate = llvm::dyn_cast<clang::AnnotateAttr>(Attr)) {
      if (Annotate->getAnnotation() == AnnotationValue) {
        return true;
      }
    }
  }
  return false;
}

// Custom matcher to check for a SectionAttr with a specific name
AST_MATCHER_P(clang::FunctionDecl, _hasSectionAttrWithValue, std::string,
              SectionName) {
  // from node get PragmaClangTextSectionAttr and check if it has the correct
  // name
  for (const auto *Attr : Node.attrs()) {
    if (const auto *SA =
            llvm::dyn_cast<clang::PragmaClangTextSectionAttr>(Attr)) {
      return SA->getName() == SectionName;
    }
  }
  return false;
}

class WrapFunctionConsumer : public clang::ASTConsumer {
public:
#if 0
  WrapFunctionConsumer(clang::Rewriter &R) : Handler(R) {
    std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;
    auto wrap = clang::ast_matchers::functionDecl(_hasAnnotateAttrWithValue("wrap"));
    auto data = clang::ast_matchers::functionDecl(_hasSectionAttrWithValue("data"));
    clang::ast_matchers::internal::Matcher<clang::Decl> matchAny =
        clang::ast_matchers::anyOf(wrap, data);
    auto mainSpace = clang::ast_matchers::isExpansionInMainFile();
    auto funcDecl = clang::ast_matchers::functionDecl(mainSpace, matchAny);
    clang::ast_matchers::internal::Matcher<clang::Decl> funcDeclBind =
        funcDecl.bind("funcDecl");
    matcher_decl.push_back(funcDeclBind);
    for (const auto &matcher : matcher_decl) {
      Matcher.addMatcher(matcher, &Handler);
    }
  }
  WrapFunctionConsumer(clang::Rewriter &R, std::string PointcutTextFile): Handler(R) {
    std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;

    // check file exist
    std::ifstream f(PointcutTextFile);
    if (!f.good()) {
      llvm::errs() << "File not found: " << PointcutTextFile << "\n";
      assert(false);
    }

    // read file
    std::ifstream file(PointcutTextFile);
    std::string PointcutText((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

    // string assert
    llvm::StringRef PointcutTextExpect = "run_pointcut funcDecl = pragma_clang(text, data) || annotation(wrap);";
    assert(PointcutText == PointcutTextExpect);

    // string to stringbuf
    std::stringbuf buffer(PointcutText);

    //std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;

     PointcutText = "run_pointcut funcDecl = pragma_clang(text, data) || annotation(wrap);";
    Lexer lexer4(PointcutText);
    Parser parser4(lexer4);
    auto ast = parser4.parsePointcutList();
    ASTMakeMatcherVisitor visitor;
    for (const auto &pointcut : ast) {
      pointcut->accept(visitor);
      auto matcher = visitor.getMatcher();
      if (matcher->isType(MATCH_RUN)) {
        RunMatcher* runMatcher = (RunMatcher*)matcher.get();
        clang::ast_matchers::DeclarationMatcher functionMatcher = runMatcher->getMatcher();
        Matcher.addMatcher(functionMatcher, &Handler);
      } else {
        assert(false);
      }
    }
  }
#else
  WrapFunctionConsumer(clang::Rewriter &R) : Handler(R) {
    std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;

    llvm::StringRef PointcutText = "run_pointcut funcDecl = pragma_clang(text, data) || annotation(wrap);";
    Lexer lexer4(PointcutText);
    Parser parser4(lexer4);
    auto ast = parser4.parsePointcutList();
    ASTMakeMatcherVisitor visitor;
    for (const auto &pointcut : ast) {
      pointcut->accept(visitor);
      auto matcher = visitor.getMatcher();
      if (matcher->isType(MATCH_RUN)) {
        RunMatcher* runMatcher = (RunMatcher*)matcher.get();
        clang::ast_matchers::DeclarationMatcher functionMatcher = runMatcher->getMatcher();
        Matcher.addMatcher(functionMatcher, &Handler);
      } else {
        assert(false);
      }
    }
  }
#endif
  WrapFunctionConsumer(clang::Rewriter &R, std::string PointcutTextFile): Handler(R) {
    // Read the file into a MemoryBuffer
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> FileOrErr =
        llvm::MemoryBuffer::getFile(PointcutTextFile);

    if (!FileOrErr) {
        llvm::errs() << "Error reading file: " << PointcutTextFile << "\n";
        llvm::errs() << "Error: " << FileOrErr.getError().message() << "\n";
        assert(false && "File not found or cannot be read");
    }

    std::unique_ptr<llvm::MemoryBuffer> &Buffer = FileOrErr.get();

    // Get the contents as a StringRef
    llvm::StringRef PointcutStringRead = Buffer->getBuffer();

    // Check for UTF-8 BOM and remove it if present
    if (PointcutStringRead.startswith("\xEF\xBB\xBF")) {
        // Remove the first three bytes (UTF-8 BOM)
        PointcutStringRead = PointcutStringRead.drop_front(3);
    }

    // Expected string for verification
    llvm::StringRef PointcutTextExpect = "run_pointcut funcDecl = pragma_clang(text, data) || annotation(wrap);";

    // Verify the content matches the expected string
    if (PointcutStringRead.trim() != PointcutTextExpect) {
        llvm::errs() << "Pointcut text does not match expected value.\n";
        llvm::errs() << "Expected: " << PointcutTextExpect << "\n";
        llvm::errs() << "Got: " << PointcutStringRead << "\n";
        assert(false && "Pointcut text does not match");
    }

    // Now process PointcutStringRead
    // For example, create a Lexer
    Lexer lexer(PointcutStringRead);
    Parser parser4(lexer);
    auto ast = parser4.parsePointcutList();
    ASTMakeMatcherVisitor visitor;
    for (const auto &pointcut : ast) {
      pointcut->accept(visitor);
      auto matcher = visitor.getMatcher();
      if (matcher->isType(MATCH_RUN)) {
        RunMatcher* runMatcher = (RunMatcher*)matcher.get();
        clang::ast_matchers::DeclarationMatcher functionMatcher = runMatcher->getMatcher();
        Matcher.addMatcher(functionMatcher, &Handler);
      } else {
        assert(false);
      }
    }
  }

  void HandleTranslationUnit(clang::ASTContext &Context) override {
    Matcher.matchAST(Context);
  }
  std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;

private:
  WrapFunctionCallback Handler;
  clang::ast_matchers::MatchFinder Matcher;
};
