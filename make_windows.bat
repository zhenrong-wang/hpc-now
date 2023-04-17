% This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) %
% The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com) %
% It is distributed under the license: GNU Public License - v2.0 %
% Bug report: info@hpc-now.com %

echo "[ START: ] Building the binaries now ..."
mkdir .\bin
del /s /q /f .\bin\*
gcc .\hpcopr\main\hpcopr_main.c -Wall -lm -o .\bin\hpcopr_windows.exe
gcc .\now-crypto\now-crypto.c -Wall -lm -o .\bin\now-crypto-windows.exe
gcc .\installer\installer.c -Wall -lm -o .\bin\installer_windows.exe

echo "[ -DONE- ] Please check the console output for building results."