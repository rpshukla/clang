#include <vector>
#include <unordered_set>

// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"


#include "clang/Frontend/CompilerInstance.h"

using namespace clang::tooling;
using namespace llvm;

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// TODO: remove
#ifdef FOO

#ifdef B
#ifdef C
#endif
#endif

#ifdef D
#endif

#endif

#ifdef BANG
#endif

// TODO: make not global
std::vector<std::string> ifdefNames;
std::unordered_set<std::string> desiredNames = {"FOO", "BANG", "B", "C", "D"};

/**
 * Clang PPCallbacks object to be added to the Preprocessor instance
 */
class IfdefPPCallbacks : public clang::PPCallbacks {
public:
  /*
   * Called whenever an ifdef is seen
   */
  virtual void Ifdef(clang::SourceLocation Loc,
                     const clang::Token &MacroNameTok,
                     const clang::MacroDefinition &MD) {

    // Get the actual identifier string following #ifdef
    std::string name = MacroNameTok.getIdentifierInfo()->getName();
    if (desiredNames.find(name) != desiredNames.end()) {
      ifdefNames.push_back(name);
    }
  }
};

/**
 * The FrontendAction to be passed to the Tool
 */
class IfdefAnalysisAction : public clang::PreprocessOnlyAction {
public:
  /**
   * Callback before starting to process a file. Attaches PPCallbacks
   * for ifdefs.
   */
  bool BeginSourceFileAction(clang::CompilerInstance &CI) {
    clang::Preprocessor &preprocessor = CI.getPreprocessor();

    std::string predefines = preprocessor.getPredefines();
    // Defines all preprocessor variables we may want to check so that the
    // contents of any ifdefs are accessible
    preprocessor.setPredefines(predefines + "\n#define FOO\n"
                               "\n#define BANG\n"
                               "\n#define B\n"
                               "\n#define C\n"
                               "\n#define D\n");

    std::unique_ptr<clang::PPCallbacks> callbacks(new IfdefPPCallbacks);
    preprocessor.addPPCallbacks(std::move(callbacks));

    // Returns true to signal success so that processing continues
    return true;
  }
};

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  // 0 on success, 1 if error occurred, 2 if some files skipped
  int result = Tool.run(newFrontendActionFactory<IfdefAnalysisAction>().get());

  for (unsigned int i = 0; i < ifdefNames.size(); i++) {
    outs() << ifdefNames[i] << "\n";
  }
  if (result != 0) {
    errs() << "Uh oh, exit code " << result << "\n";
    return result;
  }
  outs() << "No errors!\n";
  return result;
}
