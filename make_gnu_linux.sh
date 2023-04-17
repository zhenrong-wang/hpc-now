# This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
# The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
# It is distributed under the license: GNU Public License - v2.0
# Bug report: info@hpc-now.com

#!/bin/bash

echo -e "[ START: ] Building the binaries now ..."
mkdir -p ./bin
rm -rf ./bin/*
gcc ./hpcopr/main/hpcopr_main.c -Wall -lm -o ./bin/hpcopr_linux
gcc ./now-crypto/now-crypto.c -Wall -lm -o ./bin/now-crypto-linux.exe
gcc ./installer/installer.c -Wall -lm -o ./bin/installer_linux
gcc ./hpcmgr/hpcmgr.c -Wall -lm -o ./bin/hpcmgr
chmod +x ./bin/*
echo -e "[ -DONE- ] Please check the console output for building results."