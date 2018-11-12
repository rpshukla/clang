# Variability-Aware Clang
A variability-aware analysis tool built on top of the clang source code.
<!-- Who, Where, When -->
This was developed as part of [SMR](https://sarahnadi.org/smr/) in collaboration with [IBM](https://www-01.ibm.com/ibm/cas/canada/).

Written by Jacob Reckhard and Sarah Nadi<br>
Emails: reckhard@ualberta.ca, nadi@ualberta.ca

<!-- What -->
## What it is

This is a tool for performing a variability-aware analysis. What it does is it
takes your source code, and it looks through it for errors that could occur due
to variability. It's to be used in a similar manner to your regular compiler,
except instead of compiling your code to an executable, it will instead search
it for possible bugs.

<!-- Why -->
## Why do we need this

Take a look at the following code snippet

```C++
#ifdef Animals
    Item cat = 10.99;
    Item dog = 13.99;
    Item mouse = 9.99;
#endif
#ifdef Computers
    Item keyboard = 34.99;
    Item monitor = 99.99;
    Item mouse = 4.99;
#endif
```

Lets say this is your web store. It all looks good, if you want to sell animals
you can, if you want to sell computer parts, that works fine too. However, if
you try and sell both at once, your code doesn't even compile. In this simple
example it's an easy fix, however, it could be a non trivial problem. The worst
part is, it's not immediately obvious. This compile error might not come up
until much later when you have written lots of other code depending on the
erroneous code.



<!-- How -->
## How to Build
In order to build clang, you need to have llvm, so go and clone that. We only
need the one branch.\
`$ git clone --single-branch -b release_60 https://github.com/llvm-mirror/llvm.git llvm`\
Then clone this project into the proper spot in the llvm source tree\
`$ git clone https://github.com/jacobrec/varclang.git llvm/tools/clang`\
Then make and enter a build directory. In source builds are not supported\
`$ mkdir build && cd build`\
Then run cmake to generate build files\
`$ cmake -G "Unix Makefiles" ../llvm`\
Finally, build the project\
`$ make`\
Since this is just a modified version of clang, the [official
documentation](https://clang.llvm.org/get_started.html) is a
great place to go for build options.

The cmake command I used was 
```
CXX=clang++ CC=clang cmake -DCMAKE_BUILD_TYPE=DEBUG -DLLVM_INCLUDE_TESTS=OFF -DLLVM_USE_LINKER=lld -DLLVM_TARGETS_TO_BUILD=X86 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DLLVM_INCLUDE_EXAMPLES=OFF -DLLVM_BUILD_EXAMPLES=OFF -DLLVM_BUILD_TESTS=OFF -G "Ninja" ../.llvm
```

## How to Use
Now that you have built the project. If you go into build/bin, there will be a
whole bunch of binaries. For this, there are two important ones. clang and
clang++.

Lets say you have a c++ file you want to analyze. If this file is named
file.cpp. You would type

`build/bin/clang++ -fsyntax-only file.cpp`

This will do the variability aware analysis with every ifdef it reaches. If you
include any header files from the standard library, this can be a problem, as
you probably don't want to do your analysis on all of the standard library, you
just care about your code. So, you can create a config file to specify the
macros to perform variability aware analysis on.

This config file is simple. It is just a newline separated list of macro names
to analyze.

To specify you want to use your config file, you do this.\
`build/bin/clang++ -fsyntax-only -Xclang -var-config=my_config_file file.cpp`\
It is important that the flag -Xclang immediately precedes the -var-config flag.

For C programs, it is the same, except you use build/bin/clang instead of
build/bin/clang++

## How it Works


## How to Improve
Some know flaws:
- Certain programs cause it to crash

Some things that could be improved:
- Simplifying boolean expressions
- Expanding to allow analysis on #if and #elif



## Info on clang
Information on Clang:             http://clang.llvm.org/
Building and using Clang:         http://clang.llvm.org/get_started.html
Information on the LLVM project:  http://llvm.org/

