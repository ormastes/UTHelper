#include "WrapFunctionConsumer.h"
#include "RemoveFinalConsumer.h"
#include "MakeVirtualConsumer.h"
#include "CompositeConsumer.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/Path.h"


class UTHelperAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    
    // Create appropriate consumer based on mode
    if (RemoveFinal || MakeVirtual) {
      auto composite = std::make_unique<CompositeConsumer>();
      
      if (RemoveFinal) {
        composite->addConsumer(std::make_unique<RemoveFinalConsumer>(Rewrite, BaseFolder));
      }
      
      if (MakeVirtual) {
        composite->addConsumer(std::make_unique<MakeVirtualConsumer>(Rewrite, BaseFolder));
      }
      
      return composite;
    } else if (!PointcutText.empty()) {
      auto consumer = std::make_unique<WrapFunctionConsumer>(Rewrite, PointcutText);
      consumer->setBaseFolder(BaseFolder);
      return consumer;
    } else {
      llvm::errs() << "No operation specified. Use either pointcut=<file>, remove-final, or make-virtual\n";
      return nullptr;
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
      } else if (arg.starts_with("base-folder=")) {
        BaseFolder = arg.substr(strlen("base-folder="));
        // Convert to absolute path if relative
        if (!llvm::sys::path::is_absolute(BaseFolder)) {
          llvm::SmallString<256> AbsPath(BaseFolder);
          std::error_code EC = llvm::sys::fs::make_absolute(AbsPath);
          if (EC) {
            llvm::errs() << "Failed to resolve base folder path: " << EC.message() << "\n";
            return false;
          }
          BaseFolder = std::string(AbsPath.str());
        }
        // Ensure it ends with a separator for easier comparison
        if (!BaseFolder.empty() && BaseFolder.back() != llvm::sys::path::get_separator()[0]) {
          BaseFolder += llvm::sys::path::get_separator();
        }
      } else if (arg == "remove-final") {
        RemoveFinal = true;
      } else if (arg == "make-virtual") {
        MakeVirtual = true;
      } else {
        llvm::errs() << "Unknown argument: " << arg << "\n";
        return false;
      }
    }
    
    // Validate that we have at least one operation
    if (!RemoveFinal && !MakeVirtual && PointcutText.empty()) {
      llvm::errs() << "No operation specified. Use either pointcut=<file>, remove-final, or make-virtual\n";
      return false;
    }
    
    return true;
  }

private:
  clang::Rewriter Rewrite;
  std::string PointcutText;
  std::string BaseFolder;
  bool RemoveFinal = false;
  bool MakeVirtual = false;
};

static clang::FrontendPluginRegistry::Add<UTHelperAction>
    X("uthelper",
      "UTHelper plugin - Remove final keywords, wrap functions, and more");