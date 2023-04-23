# This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
# The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
# It is distributed under the license: GNU Public License - v2.0
# Bug report: info@hpc-now.com

#!/bin/bash

if [ ! -n "$1" ]; then
	echo "[ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command."
	echo "|          build  - (re)build the binaries"
    echo "|          delete - delete the previous binaries"
    echo "|          clear  - remove the 'bin' folder"
	echo "[ -DONE- ] Exit now."
    exit 1
elif [ "$1" = "build" ]; then
    echo "[ START: ] Building the binaries now ..."
    echo "[ -INFO- ] Please build hpcmgr with GNU/Linux, not macOS."
    mkdir -p ./build
    rm -rf ./build/*
    clang ./hpcopr/*.c -Wall -o ./build/hpcopr.exe
    clang -c ./hpcopr/general_funcs.c -o ./installer/gfuncs.o
    rm -rf ./installer/libgfuncs.a
    ar -rc ./installer/libgfuncs.a ./installer/gfuncs.o
    rm -rf ./installer/gfuncs.o
    clang ./installer/installer.c libgfuncs.a -Wall -o ./build/installer.exe
    clang ./now-crypto/now-crypto.c -Wall -lm -o ./build/now-crypto.exe
    chmod +x ./build/*
    mv ./installer/libgfuncs.a ./build/
    
elif [ "$1" = "delete" ]; then
    echo "[ START: ] Deleting the binaries now ..."
    rm -rf ./build/*
elif [ "$1" = "clear" ]; then
    echo "[ START: ] Removing the bin folder now ..."
    rm -rf ./build
else
    echo "[ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command."
	echo "|          build  - (re)build the binaries"
    echo "|          delete - delete the previous binaries"
    echo "|          clear  - remove the 'bin' folder"
	echo "[ -DONE- ] Exit now."
    exit 1
fi
echo "[ -DONE- ] Please check the console output for building results."