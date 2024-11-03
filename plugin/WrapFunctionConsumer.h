#pragma once

#include "WrapFunctionCallback.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include <vector>

// Forward declarations
namespace clang {
    class ASTContext;
}

class WrapFunctionConsumer : public clang::ASTConsumer {
public:
    WrapFunctionConsumer(clang::Rewriter &R, const std::string &PointcutTextFile);

    void HandleTranslationUnit(clang::ASTContext &Context) override;

private:
    WrapFunctionCallback Handler;
    clang::ast_matchers::MatchFinder Matcher;
    std::vector<clang::ast_matchers::DeclarationMatcher> matcher_decl;
};