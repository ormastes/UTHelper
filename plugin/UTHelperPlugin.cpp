#include "WrapFunctionConsumer.h"
#include "UnifiedASTVisitor.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/Path.h"


class UTHelperAction : public clang::PluginASTAction {
public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI, llvm::StringRef) override {
    Rewrite.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    
    // Check if base-folder is provided (mandatory)
    if (BaseFolder.empty()) {
      llvm::errs() << "Error: base-folder parameter is mandatory\n";
      return nullptr;
    }
    
    // If pointcut mode is specified, use only that
    if (!PointcutText.empty()) {
      auto consumer = std::make_unique<WrapFunctionConsumer>(Rewrite, PointcutText);
      consumer->setBaseFolder(BaseFolder);
      return consumer;
    }
    
    // Default mode: all transformations enabled unless explicitly disabled
    return std::make_unique<UnifiedASTConsumer>(Rewrite, BaseFolder,
                                                !DisableRemoveFinal,
                                                !DisableMakeVirtual,
                                                !DisableAddFriend,
                                                CustomFriends);
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
      } else if (arg == "disable-remove-final") {
        DisableRemoveFinal = true;
      } else if (arg == "disable-make-virtual") {
        DisableMakeVirtual = true;
      } else if (arg == "disable-add-friend") {
        DisableAddFriend = true;
      } else if (arg.starts_with("custom-friends=")) {
        std::string friendsList = arg.substr(strlen("custom-friends="));
        parseFriendsList(friendsList);
      } else {
        llvm::errs() << "Unknown argument: " << arg << "\n";
        return false;
      }
    }
    
    // Validate that base-folder is provided (mandatory)
    if (BaseFolder.empty()) {
      llvm::errs() << "Error: base-folder parameter is mandatory\n";
      llvm::errs() << "Usage: -Xclang -plugin-arg-uthelper -Xclang base-folder=<path>\n";
      return false;
    }
    
    return true;
  }

private:
  void parseFriendsList(const std::string &friendsList) {
    // Parse semicolon-separated list of friend templates
    size_t start = 0;
    size_t end = friendsList.find(';');
    
    while (end != std::string::npos) {
      std::string friendTemplate = friendsList.substr(start, end - start);
      if (!friendTemplate.empty()) {
        CustomFriends.push_back({friendTemplate});
      }
      start = end + 1;
      end = friendsList.find(';', start);
    }
    
    // Don't forget the last one
    if (start < friendsList.length()) {
      std::string friendTemplate = friendsList.substr(start);
      if (!friendTemplate.empty()) {
        CustomFriends.push_back({friendTemplate});
      }
    }
  }

  clang::Rewriter Rewrite;
  std::string PointcutText;
  std::string BaseFolder;
  bool DisableRemoveFinal = false;
  bool DisableMakeVirtual = false;
  bool DisableAddFriend = false;
  std::vector<FriendTemplate> CustomFriends;
};

static clang::FrontendPluginRegistry::Add<UTHelperAction>
    X("uthelper",
      "UTHelper plugin - Remove final keywords, wrap functions, and more");