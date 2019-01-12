# Analysing ifdef conditions using clang

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
TODO

Usage:

``` bash
$BD/bin/my-tool -p <build-directory> [desired preprocessor variable names] [sources]
```
Note: "build-directory" refers to the build directory of the code being analyzed. "sources" refers to the source files to analyze.

All the names of preprocessor variables to analyze need to be specified.

