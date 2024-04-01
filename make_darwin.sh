# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

uname -s | grep Darwin >> /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "[ FATAL: ] You are not working on macOS (Darwin). Exit now."
    exit 3
fi

hpcopr_version_code=`cat ./hpcopr/now_macros.h | grep CORE_VERSION_CODE | awk -F"\"" '{print $2}'`
installer_version_code=`cat ./installer/installer.h | grep INSTALLER_VERSION_CODE | awk -F"\"" '{print $2}'`

if [ ! -n "$1" ]; then
    echo "[ -INFO- ] Please specify an option: 'build', 'delete', or 'clear'"
    echo "|          build  - (re)build the binaries"
    echo "|          delete - delete the previous binaries"
    echo "|          clear  - remove the 'build' folder and binaries in it"
    echo "[ -DONE- ] Exit now."
    exit 1
elif [ "$1" = "build" ]; then
    echo "[ START: ] Building the binaries now ..."
    echo "[ -INFO- ] Please build hpcmgr with GNU/Linux, not macOS."
    mkdir -p ./build
    rm -rf ./build/*
    clang ./hpcopr/*.c -Wall -lpthread -o ./build/hpcopr-dwn-${hpcopr_version_code}.exe
    clang -c ./hpcopr/general_funcs.c -Wall -o ./installer/gfuncs.o
    clang -c ./hpcopr/opr_crypto.c -Wall -o ./installer/ocrypto.o
    clang -c ./hpcopr/cluster_general_funcs.c -Wall -o ./installer/cgfuncs.o
    clang -c ./hpcopr/time_process.c -Wall -o ./installer/tproc.o
    clang -c ./hpcopr/general_print_info.c -Wall -o ./installer/gprint.o
    clang -c ./hpcopr/now_md5.c -Wall -o ./installer/md5.o
    clang -c ./hpcopr/now_sha256.c -Wall -o ./installer/sha256.o
    ar -rc ./installer/libnow.a ./installer/gfuncs.o ./installer/ocrypto.o ./installer/cgfuncs.o ./installer/tproc.o ./installer/md5.o ./installer/gprint.o ./installer/sha256.o
    clang ./installer/installer.c ./installer/libnow.a -lpthread -Wall -o ./build/installer-dwn-${installer_version_code}.exe
    clang ./now-crypto/now-crypto-v3-aes.c -Wall -Ofast -o ./build/now-crypto-aes-dwn.exe
    chmod +x ./build/*
    rm -rf ./installer/*.a
    rm -rf ./installer/*.o
elif [ "$1" = "delete" ]; then
    echo "[ START: ] Deleting the binaries now ..."
    rm -rf ./build/*
elif [ "$1" = "clear" ]; then
    echo "[ START: ] Removing the build folder now ..."
    rm -rf ./build
else
    echo "[ -INFO- ] Please specify an option: 'build', 'delete', or 'clear'"
    echo "|          build  - (re)build the binaries"
    echo "|          delete - delete the previous binaries"
    echo "|          clear  - remove the 'build' folder and binaries in it"
    echo "[ -DONE- ] Exit now."
    exit 1
fi
echo "[ -DONE- ] Please check the console output for building results."