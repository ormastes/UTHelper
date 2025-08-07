#include "CompositeConsumer.h"

void CompositeConsumer::addConsumer(std::unique_ptr<clang::ASTConsumer> Consumer) {
  Consumers.push_back(std::move(Consumer));
}

void CompositeConsumer::HandleTranslationUnit(clang::ASTContext &Context) {
  // Run each consumer in sequence
  for (auto &Consumer : Consumers) {
    Consumer->HandleTranslationUnit(Context);
  }
}