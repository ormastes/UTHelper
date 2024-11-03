#include "WrapFunctionConsumer.h"
#include "WrapFunctionCallback.h"

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/ASTContext.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>

#include "AST2Matcher.h"
#include "Lexer.h"
#include "Parser.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace llvm;

namespace {

    // Custom matcher to check for an AnnotateAttr with a specific annotation value
    AST_MATCHER_P(FunctionDecl, _hasAnnotateAttrWithValue, std::string, AnnotationValue) {
        for (const auto *Attr : Node.attrs()) {
            if (const auto *Annotate = llvm::dyn_cast<AnnotateAttr>(Attr)) {
                if (Annotate->getAnnotation() == AnnotationValue) {
                    return true;
                }
            }
        }
        return false;
    }

    // Custom matcher to check for a SectionAttr with a specific name
    AST_MATCHER_P(FunctionDecl, _hasSectionAttrWithValue, std::string, SectionName) {
        // Check if the function has a section attribute with the given name
        for (const auto *Attr : Node.attrs()) {
            if (const auto *SA = llvm::dyn_cast<SectionAttr>(Attr)) {
                if (SA->getName() == SectionName) {
                    return true;
                }
            }
        }
        return false;
    }

}

WrapFunctionConsumer::WrapFunctionConsumer(clang::Rewriter &R, const std::string &PointcutTextFile)
    : Handler(R) {
    // Read the file into a MemoryBuffer
    ErrorOr<std::unique_ptr<MemoryBuffer>> FileOrErr =
        MemoryBuffer::getFile(PointcutTextFile);

    if (!FileOrErr) {
        errs() << "Error reading file: " << PointcutTextFile << "\n";
        errs() << "Error: " << FileOrErr.getError().message() << "\n";
        llvm_unreachable("File not found or cannot be read");
    }

    std::unique_ptr<MemoryBuffer> &Buffer = FileOrErr.get();

    // Get the contents as a StringRef
    StringRef PointcutStringRead = Buffer->getBuffer();

    // Check for UTF-8 BOM and remove it if present
    if (PointcutStringRead.starts_with("\xEF\xBB\xBF")) {
        // Remove the first three bytes (UTF-8 BOM)
        PointcutStringRead = PointcutStringRead.drop_front(3);
    }

    // Expected string for verification
    StringRef PointcutTextExpect = "run_pointcut funcDecl = pragma_clang(text, data) || annotation(wrap);";

    // Verify the content matches the expected string
    if (PointcutStringRead.trim() != PointcutTextExpect) {
        errs() << "Pointcut text does not match expected value.\n";
        errs() << "Expected: " << PointcutTextExpect << "\n";
        errs() << "Got: " << PointcutStringRead << "\n";
        llvm_unreachable("Pointcut text does not match");
    }

    // Now process PointcutStringRead
    // Create a Lexer and Parser to parse the pointcut string
    ::Lexer lexer(PointcutStringRead);
    Parser parser(lexer);
    auto ast = parser.parsePointcutList();
    ASTMakeMatcherVisitor visitor;

    for (const auto &pointcut : ast) {
        pointcut->accept(visitor);
        auto matcher = visitor.getMatcher();
        if (matcher->isType(MATCH_RUN)) {
            auto *runMatcher = dynamic_cast<RunMatcher*>(matcher.get());
            assert(runMatcher && "Failed to cast to RunMatcher");
            DeclarationMatcher functionMatcher = runMatcher->getMatcher();
            Matcher.addMatcher(functionMatcher, &Handler);
        } else {
            llvm_unreachable("Matcher type not supported");
        }
    }
}

void WrapFunctionConsumer::HandleTranslationUnit(ASTContext &Context) {
    Matcher.matchAST(Context);
}