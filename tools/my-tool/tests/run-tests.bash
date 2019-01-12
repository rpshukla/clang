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

echo '-----test2-----'
$BD/bin/my-tool A B test2.cpp -- clang++ test test2.cpp
echo

echo '-----test3-----'
$BD/bin/my-tool A B test3.cpp -- clang++ test test3.cpp
echo

echo '-----test4-----'
$BD/bin/my-tool A B test4.cpp -- clang++ test test4.cpp
echo

echo '-----test5-----'
$BD/bin/my-tool A B C test5.cpp -- clang++ test test5.cpp
echo

echo '-----test6-----'
$BD/bin/my-tool A B test6.cpp -- clang++ test test6.cpp
echo

echo '-----test7-----'
$BD/bin/my-tool A B C D E test7.cpp -- clang++ test test7.cpp
echo
