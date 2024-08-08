#!/bin/bash

# set -x

if [ -z "$EMSDK" ] ; then
    echo "WARNING: EMSDK unset, trying to use system-default with EM_CONFIG ~/.emscripten (not recommended)"
    echo "WARNING: Consider installing emscripten upstream and setup environment via ~/emsdk/emsdk_env.sh"
    export EM_CONFIG=$HOME/.emscripten
else
    echo "INFO: EMSDK set (recommended), using it"
fi

# RECOMMENDED
#   emsdk_root=`readlink -f $HOME/emsdk`
#   echo "Using EMSDK ${emsdk_root}"
#   . ${emsdk_root}/emsdk_env.sh

# NOT RECOMMENDED
#   export EM_CONFIG=$HOME/.emscripten

# Generate config, clear cach and/or ports
#   emcc --generate-config
#   emcc --clear-cache
#   emcc --clear-ports
#

