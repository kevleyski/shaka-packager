#!/bin/bash

# NOTE: dont forget the third party deps
# git clone --recurse-submodules git@github.com:kevleyski/shaka-packager.git

brew install cmake
brew install ninja

CMAKE=/opt/homebrew/bin/cmake

$CMAKE -B cmake-build-release -G Ninja -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Release
$CMAKE --build cmake-build-release --parallel --config Release

$CMAKE -B cmake-build-debug -G Ninja -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DCMAKE_BUILD_TYPE=Debug
$CMAKE --build cmake-build-debug --parallel --config Debug
