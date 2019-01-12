# Analysing ifdef conditions using clang

This tool was built using the clang LibTooling library. My source code is located in tools/my-tool

## Build instructions
Clone the llvm repository:
```bash
git clone --single-branch -b release_70 https://github.com/llvm-mirror/llvm.git llvm
```
Ensure that the branch 'release_70' is checked out.

Go to the 'tools' directory inside the 'llvm' directory. And clone this repository.
```bash
cd llvm/tools
git clone --single-branch -b ifdef-conditions https://github.com/rpshukla/clang.git clang
```
Ensure that the branch 'ifdef' conditions is checked out.

Make a build directory outside of the llvm directory and cd into it.
```bash
cd ../..
mkdir build
cd build
```

Run cmake and make to build the project.
```bash
cmake -G "Unix Makefiles" ../llvm
make
```

## Running Instructions

### Running tests
I have placed test files in the "tests" directory of the tool. Simply run the "run-tests.bash" script to run the tool on all the test files.

``` bash
cd path/to/clang/tools/my-tool
./run-tests.bash
```
### Command line usage

``` bash
export BD=path/to/build
$BD/bin/my-tool [desired preprocessor variable names] [sources] -- [compile commands]
```
Note: "sources" refers to the source files to analyze.

Note: compile commands are only required since all clang tools require them. However, since this tool only uses the preprocessor, it doesn't really matter what these commands are.

Note: All the names of preprocessor variables to analyze need to be specified.
