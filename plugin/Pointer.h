#pragma once

#include <memory>

#include "clang/AST/Attr.h"
#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using RunMatcherPtr = clang::ast_matchers::internal::Matcher<clang::Decl>;

using CallMatcherPtr = clang::ast_matchers::internal::Matcher<clang::Stmt>;

struct AbstractAST2Matcher;
using AbstractAST2MatcherPtr = std::unique_ptr<AbstractAST2Matcher>;
