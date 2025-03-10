eclipse-threadx#!/bin/bash

cd $(dirname $0)

# if threadx repo does not exist, clone it
[ -d ../threadx ] || git clone https://github.com/eclipse-threadx/threadx.git ../threadx --depth 1
[ -d ../filex ] || git clone https://github.com/eclipse-threadx/filex.git ../filex --depth 1
[ -f .run.sh ] || ln -sf ../threadx/scripts/cmake_bootstrap.sh .run.sh
CTEST_PARALLEL_LEVEL=1 ENABLE_IDLE=ON ./.run.sh $*
