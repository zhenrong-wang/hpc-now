# This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
# The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
# It is distributed under the license: GNU Public License - v2.0
# Bug report: info@hpc-now.com

#!/bin/bash

if [ ! -n "$1" ]; then
	echo -e "[ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command."
	echo -e "|          build  - (re)build the binaries"
    echo -e "|          delete - delete the previous binaries"
    echo -e "|          clear  - remove the 'bin' folder"
	echo -e "[ -DONE- ] Exit now."
    exit 1
elif [ "$1" = "build" ]; then
    echo -e "[ START: ] Building the binaries now (including hpcmgr) ..."
    mkdir -p ./bin
    rm -rf ./bin/*
    gcc ./hpcopr/main/hpcopr_main.c -Wall -lm -o ./bin/hpcopr.exe
    gcc ./now-crypto/now-crypto.c -Wall -lm -o ./bin/now-crypto.exe
    gcc ./installer/installer.c -Wall -lm -o ./bin/installer_linux
    gcc ./hpcmgr/hpcmgr.c -Wall -lm -o ./bin/hpcmgr
    chmod +x ./bin/*
elif [ "$1" = "delete" ]; then
    echo -e "[ START: ] Deleting the binaries now ..."
    rm -rf ./bin/*
elif [ "$1" = "clear" ]; then
    echo -e "[ START: ] Removing the bin folder now ..."
    rm -rf ./bin
else
    echo -e "[ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command."
	echo -e "|          build  - (re)build the binaries"
    echo -e "|          delete - delete the previous binaries"
    echo -e "|          clear  - remove the 'bin' folder"
	echo -e "[ -DONE- ] Exit now."
    exit 1
fi
echo -e "[ -DONE- ] Please check the console output for building results."