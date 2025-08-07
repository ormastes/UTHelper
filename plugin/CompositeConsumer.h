#ifndef COMPOSITE_CONSUMER_H
#define COMPOSITE_CONSUMER_H

#include "clang/AST/ASTConsumer.h"
#include <vector>
#include <memory>

class CompositeConsumer : public clang::ASTConsumer {
public:
  void addConsumer(std::unique_ptr<clang::ASTConsumer> Consumer);
  
  void HandleTranslationUnit(clang::ASTContext &Context) override;
  
private:
  std::vector<std::unique_ptr<clang::ASTConsumer>> Consumers;
};

#endif // COMPOSITE_CONSUMER_H