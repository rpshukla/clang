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

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

// Declare command line option for list of desired preprocessor variable names
// Add to MyToolCategory using cl::cat
cl::list<std::string> DesiredNamesOpt(
    cl::Positional,
    cl::desc("[Names of preprocessor variables which should be analyzed]"),
    cl::cat(MyToolCategory));

/**
 * Stores names of preprocessor variables which we are interested in analyzing.
 */
class DesiredNames {
public:
  void insert(std::string name) {
    names.insert(name);
    predefines += "\n#define " + name;
  }

  /**
   * Returns true if name in this->names
   */
  bool contains(std::string name) {
    return names.find(name) != names.end();
  }

  /**
   * Returns string which can be used as a predefine to define all of the
   * desired names
   */
  std::string getPredefines() {
    return predefines;
  }

private:
  std::unordered_set<std::string> names;
  std::string predefines;
} desiredNames;


/**
 * Stores data about an ifdef block
 */
class IfdefNode {
public:
  IfdefNode(std::string name) {
    this->name = name;
    this->parent = nullptr;
  }
  void setEndLocation(clang::SourceLocation Loc) {
    endLocation = Loc;
  }

  std::string name; // The actual identifier string following #ifdef
  clang::SourceLocation startLocation;
  clang::SourceLocation endLocation;

  IfdefNode *parent;
};
// The global list of ifdefNodes
std::vector<IfdefNode> ifdefNodes;

/**
 * Checks to see if one Ifdefnode is nested inside another Ifdefnode. If so,
 * sets the parent field of child to parent.
 * \param parent a potential parent node.
 * \param child a potential child node.
 * \param sourceManager reference to a sourceManager used to decode
 * SourceLocations.
 */
bool checkNested(IfdefNode &parent, IfdefNode &child,
                 clang::SourceManager &sourceManager) {
  bool result = false;
  clang::BeforeThanCompare<clang::SourceLocation> beforeThanCompare(
      sourceManager);

  // Check if parent start location is before child start location
  // Check if child end location is before parent
  if (beforeThanCompare(parent.startLocation, child.startLocation) &&
      beforeThanCompare(child.endLocation, parent.endLocation)) {
    result = true;
  }

  if (result) {
    // Set parent field of child to parent
    child.parent = &parent;
  }

  return result;
}

/**
 * Clang PPCallbacks object to be added to the Preprocessor instance
 */
class IfdefPPCallbacks : public clang::PPCallbacks {
public:
  /**
   * Called whenever an ifdef is seen.
   */
  virtual void Ifdef(clang::SourceLocation Loc,
                     const clang::Token &MacroNameTok,
                     const clang::MacroDefinition &MD) {

    // Get the actual identifier string following #ifdef
    std::string name = MacroNameTok.getIdentifierInfo()->getName();
    if (desiredNames.contains(name)) {
      IfdefNode node(name);
      node.startLocation = Loc;
      ifdefNodes.push_back(node);
    }
  }

  /**
   * Called whenever and endif is seen.
   */
  virtual void Endif(clang::SourceLocation Loc, clang::SourceLocation IfLoc) {
    // Check which IfdefNode the endif corresponds to
    for (unsigned int i = 0; i < ifdefNodes.size(); i++) {
      if(ifdefNodes[i].startLocation == IfLoc) {
        ifdefNodes[i].setEndLocation(Loc);
      }
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
    this->sourceManager = &(CI.getSourceManager());
    clang::Preprocessor &preprocessor = CI.getPreprocessor();

    // Defines all preprocessor variables we may want to check so that the
    // contents of any ifdefs are accessible
    std::string predefines = preprocessor.getPredefines();
    predefines += desiredNames.getPredefines();
    preprocessor.setPredefines(predefines);

    std::unique_ptr<IfdefPPCallbacks> callbacks(new IfdefPPCallbacks);
    preprocessor.addPPCallbacks(std::move(callbacks));

    // Returns true to signal success so that processing continues
    return true;
  }

  virtual void EndSourceFileAction() {
    constructIfdefTree();
  }

private:
  /**
   * Construct tree of ifdef blocks
   */
  void constructIfdefTree() {
    for (unsigned int i = 0; i < ifdefNodes.size(); i++) {
      for (unsigned int j = 0; j < ifdefNodes.size(); j++) {
        if (i == j) continue;
        checkNested(ifdefNodes[i], ifdefNodes[j], *sourceManager);
      }
    }
  }

  clang::SourceManager *sourceManager;
};

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  // Get desired names from command line option and insert them into the global
  // desiredNames object
  for (unsigned int i = 0; i < DesiredNamesOpt.size(); i++) {
    desiredNames.insert(DesiredNamesOpt[i]);
  }

  // 0 on success, 1 if error occurred, 2 if some files skipped
  int result = Tool.run(newFrontendActionFactory<IfdefAnalysisAction>().get());

  // Traverse from each IfdefNode through its parents and print the conjuctions
  for (unsigned int i = 0; i < ifdefNodes.size(); i++) {
    IfdefNode currentNode = ifdefNodes[i];

    std::vector<std::string> names;
    names.push_back(currentNode.name);

    while (currentNode.parent != nullptr) {
      currentNode = *(currentNode.parent);
      names.push_back(currentNode.name);
    }

    // Print in reverse order (parents first)
    outs() << names[names.size() - 1];
    for (unsigned int i = 1; i < names.size(); i++) {
      outs() << " && ";
      outs() << names[names.size() - i - 1];
    }
    outs() << "\n";
  }

  return result;
}
