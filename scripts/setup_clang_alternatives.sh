#!/bin/sh

CLANG_VERSION=15

update-alternatives --remove-all clang
update-alternatives --remove-all clangd
update-alternatives --remove-all clang++
update-alternatives --remove-all clang-cpp
update-alternatives --remove-all clang-format
update-alternatives --remove-all clang-tidy
update-alternatives --install /usr/bin/clang        clang        /usr/bin/clang-${CLANG_VERSION} 20
update-alternatives --install /usr/bin/clangd       clangd       /usr/bin/clangd-${CLANG_VERSION} 20
update-alternatives --install /usr/bin/clang++      clang++      /usr/bin/clang++-${CLANG_VERSION} 20
update-alternatives --install /usr/bin/clang-cpp    clang-cpp    /usr/bin/clang-cpp-${CLANG_VERSION} 20
update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-${CLANG_VERSION} 20
update-alternatives --install /usr/bin/clang-tidy   clang-tidy   /usr/bin/clang-tidy-${CLANG_VERSION} 20
