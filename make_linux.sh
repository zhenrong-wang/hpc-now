# Copyright (C) 2022-present Shanghai HPC-NOW Technologies Co., Ltd.
# This code is distributed under the license: MIT License
# Originally written by Zhenrong WANG
# mailto: zhenrongwang@live.com | wangzhenrong@hpc-now.com

#!/bin/bash

hpcopr_version_code=`cat ./hpcopr/now_macros.h | grep CORE_VERSION_CODE | awk -F"\"" '{print $2}'`
installer_version_code=`cat ./installer/installer.h | grep INSTALLER_VERSION_CODE | awk -F"\"" '{print $2}'`

if [ ! -n "$1" ]; then
	echo -e "[ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command."
	echo -e "|          build  - (re)build the binaries"
    echo -e "|          delete - delete the previous binaries"
    echo -e "|          clear  - remove the 'build' folder"
	echo -e "[ -DONE- ] Exit now."
    exit 1
elif [ "$1" = "build" ]; then
    echo -e "[ START: ] Building the binaries now (including hpcmgr and now-server) ..."
    mkdir -p ./build
    rm -rf ./build/*
    gcc ./hpcopr/*.c -Wall -o ./build/hpcopr-lin-${hpcopr_version_code}.exe
    gcc -c ./hpcopr/general_funcs.c -Wall -o ./installer/gfuncs.o
    gcc -c ./hpcopr/opr_crypto.c -Wall -o ./installer/ocrypto.o
    gcc -c ./hpcopr/cluster_general_funcs.c -Wall -o ./installer/cgfuncs.o
    gcc -c ./hpcopr/time_process.c -Wall -o ./installer/tproc.o
    gcc -c ./hpcopr/general_print_info.c -Wall -o ./installer/gprint.o
    gcc -c ./hpcopr/now_md5.c -Wall -o ./installer/md5.o
    gcc -c ./hpcopr/now_sha256.c -Wall -o ./installer/sha256.o
    ar -rc ./installer/libnow.a ./installer/gfuncs.o ./installer/ocrypto.o ./installer/cgfuncs.o ./installer/tproc.o ./installer/md5.o ./installer/gprint.o ./installer/sha256.o
    gcc ./installer/installer.c -Wall ./installer/libnow.a -o ./build/installer-lin-${installer_version_code}.exe
    gcc ./now-crypto/now-crypto-v3-aes.c -Wall -Ofast -o ./build/now-crypto-aes-lin.exe
    gcc ./hpcmgr/hpcmgr.c -Wall -o ./build/hpcmgr.exe
    gcc ./now-server/now-server.c -Wall -o ./build/now-server.exe
    chmod +x ./build/*
    rm -rf ./installer/*.a
    rm -rf ./installer/*.o
elif [ "$1" = "delete" ]; then
    echo -e "[ START: ] Deleting the binaries now ..."
    rm -rf ./build/*
elif [ "$1" = "clear" ]; then
    echo -e "[ START: ] Removing the build folder now ..."
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