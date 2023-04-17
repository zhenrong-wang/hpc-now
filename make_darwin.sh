# This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
# The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
# It is distributed under the license: GNU Public License - v2.0
# Bug report: info@hpc-now.com

#!/bin/bash

echo -e "[ START: ] Building the binaries now ..."
mkdir -p ./bin
rm -rf ./bin/*
clang ./hpcopr/main/hpcopr_main.c -Wall -o ./bin/hpcopr_darwin
clang ./now-crypto/now-crypto.c -Wall -o ./bin/now-crypto-darwin.exe
clang ./installer/installer.c -Wall -lm -o ./bin/installer_darwin

chmod +x ./bin/*
echo -e "[ -DONE- ] All the binaries have been built to the folder './bin' ."