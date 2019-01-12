#!/usr/bin/env bash

# Runs the tool on all test files
# Assumes the location of the build directory is the same as specified in the REAMDE

# Get build directory
pushd ../../../../../../build > /dev/null
export BD=$(pwd)
popd > /dev/null

# Run the tests
# note: the compile commands specified after '--' are only there because clang tools
# require them. This tool does not actually use the compiler.
echo '-----test1-----'
$BD/bin/my-tool A B test1.cpp -- clang++ test test1.cpp
echo
