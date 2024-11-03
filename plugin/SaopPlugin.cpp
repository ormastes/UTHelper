
#include "WrapFunctionConsumer.h"


#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"



class WrapFunctionAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    if (PointcutText.empty()) {
      llvm::errs() << "No pointcut text provided\n";
      return nullptr;
    } else {
      if (auto val = std::make_unique<WrapFunctionConsumer>(Rewrite, PointcutText)) {
        return val;
      } else  {
        llvm::errs() << "Failed to create WrapFunctionConsumer\n";
        return nullptr;
      }
    }
  }


  void EndSourceFileAction() override {
    clang::SourceManager &SM = Rewrite.getSourceMgr();
    if (const clang::RewriteBuffer *RewriteBuf = Rewrite.getRewriteBufferFor(SM.getMainFileID())) {
        llvm::outs() << std::string(RewriteBuf->begin(), RewriteBuf->end());
    } else {
        // Output the original source code if no transformations were made
        llvm::StringRef Buffer = SM.getBufferData(SM.getMainFileID());
        llvm::outs() << Buffer;
    }
  }
  bool ParseArgs(const clang::CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    for (const auto &arg : args) {
      if (arg.starts_with("pointcut=")) {
        PointcutText = arg.substr(strlen("pointcut="));
        if (PointcutText.empty()) {
          llvm::errs() << "Empty pointcut text\n";
          return false;
        }
      } else {
        llvm::errs() << "Unknown argument: " << arg << "\n";
        return false;
      }
    }
    return true;
  }


private:
  clang::Rewriter Rewrite;
  std::string PointcutText;
};

static clang::FrontendPluginRegistry::Add<WrapFunctionAction>
    X("saop-plugin",
      "Rename functions with 'wrap' attribute and create wrapper functions");
