#!/bin/sh

CLANG_VERSION=21

do_alt() {
    name=$1
    update-alternatives --remove-all ${name}
    update-alternatives --install /usr/bin/${name} ${name} /usr/bin/${name}-${CLANG_VERSION} 20
}

for i in clang clangd clang++ clang-cpp clang-format git-clang-format clang-tidy clang-doc \
      ; do
    do_alt $i
done
