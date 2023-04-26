# This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
# The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
# It is distributed under the license: GNU Public License - v2.0
# Bug report: info@hpc-now.com

#!/bin/bash

if [ ! -n "$1" ]; then
	echo -e "[ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command."
	echo -e "|          build  - (re)build the binaries"
    echo -e "|          delete - delete the previous binaries"
    echo -e "|          clear  - remove the 'build' folder"
	echo -e "[ -DONE- ] Exit now."
    exit 1
elif [ "$1" = "build" ]; then
    echo -e "[ START: ] Building the binaries now (including hpcmgr) ..."
    mkdir -p ./build
    rm -rf ./build/*
    gcc ./hpcopr/*.c -Wall -lm -o ./build/hpcopr-lin.exe
    gcc -c ./hpcopr/general_funcs.c -Wall -lm -o ./installer/gfuncs.o
    rm -rf ./installer/libgfuncs.a
    ar -rc ./installer/libgfuncs.a ./installer/gfuncs.o
    rm -rf ./installer/gfuncs.o
    gcc ./installer/installer.c -Wall -lm ./installer/libgfuncs.a -o ./build/installer-lin.exe
    gcc ./now-crypto/now-crypto.c -Wall -lm -o ./build/now-crypto-lin.exe
    gcc ./hpcmgr/hpcmgr.c -Wall -lm -o ./build/hpcmgr.exe
    chmod +x ./build/*
    mv ./installer/libgfuncs.a ./build/
elif [ "$1" = "delete" ]; then
    echo -e "[ START: ] Deleting the binaries now ..."
    rm -rf ./build/*
elif [ "$1" = "clear" ]; then
    echo -e "[ START: ] Removing the bin folder now ..."
    rm -rf ./build
else
    echo -e "[ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command."
	echo -e "|          build  - (re)build the binaries"
    echo -e "|          delete - delete the previous binaries"
    echo -e "|          clear  - remove the 'bin' folder"
	echo -e "[ -DONE- ] Exit now."
    exit 1
fi
echo -e "[ -DONE- ] Please check the console output for building results."