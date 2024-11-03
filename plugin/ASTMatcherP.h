#pragma once

#include "clang/ASTMatchers/ASTMatchersMacros.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Attr.h"


AST_MATCHER_P(clang::FunctionDecl, hasPragmaClangBSSSectionAttr,
              llvm::StringRef, SectionName) {
  // from node get PragmaClangTextSectionAttr and check if it has the correct
  // name
  for (const auto *Attr : Node.attrs()) {
    if (const auto *SA =
            llvm::dyn_cast<clang::PragmaClangBSSSectionAttr>(Attr)) {
      return SA->getName() == SectionName;
    }
  }
  return false;
}

AST_MATCHER_P(clang::FunctionDecl, hasPragmaClangDataSectionAttr,
              llvm::StringRef, SectionName) {
  // from node get PragmaClangTextSectionAttr and check if it has the correct
  // name
  for (const auto *Attr : Node.attrs()) {
    if (const auto *SA =
            llvm::dyn_cast<clang::PragmaClangDataSectionAttr>(Attr)) {
      return SA->getName() == SectionName;
    }
  }
  return false;
}

AST_MATCHER_P(clang::FunctionDecl, hasPragmaClangRelroSectionAttr,
              llvm::StringRef, SectionName) {
  // from node get PragmaClangTextSectionAttr and check if it has the correct
  // name
  for (const auto *Attr : Node.attrs()) {
    if (const auto *SA =
            llvm::dyn_cast<clang::PragmaClangRelroSectionAttr>(Attr)) {
      return SA->getName() == SectionName;
    }
  }
  return false;
}

AST_MATCHER_P(clang::FunctionDecl, hasPragmaClangRodataSectionAttr,
              llvm::StringRef, SectionName) {
  // from node get PragmaClangTextSectionAttr and check if it has the correct
  // name
  for (const auto *Attr : Node.attrs()) {
    if (const auto *SA =
            llvm::dyn_cast<clang::PragmaClangRodataSectionAttr>(Attr)) {
      return SA->getName() == SectionName;
    }
  }
  return false;
}

AST_MATCHER_P(clang::FunctionDecl, hasPragmaClangTextSectionAttr,
              llvm::StringRef, SectionName) {
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

AST_MATCHER_P(clang::FunctionDecl, hasFuncitonName, llvm::StringRef,
              FunctionName) {
  return Node.getName() == FunctionName;
}

// Custom matcher to check for an AnnotateAttr with a specific annotation value
AST_MATCHER_P(clang::FunctionDecl, hasAnnotateAttrWithValue, llvm::StringRef,
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

AST_MATCHER_P(clang::FunctionDecl, hasAnnotateTypeAttrWithValue,
              llvm::StringRef, AnnotationValue) {
  for (const auto *Attr : Node.attrs()) {
    if (const auto *AnnotateType =
            llvm::dyn_cast<clang::AnnotateTypeAttr>(Attr)) {
      if (AnnotateType->getAnnotation() == AnnotationValue) {
        return true;
      }
    }
  }
  return false;
}
