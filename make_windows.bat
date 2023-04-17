@echo off
echo "[ START: ] Building the binaries now ..."
mkdir .\bin
del /s /q /f .\bin\*
gcc .\hpcopr\main\hpcopr_main.c -Wall -lm -o .\bin\hpcopr_windows.exe
gcc .\now-crypto\now-crypto.c -Wall -lm -o .\bin\now-crypto-windows.exe
gcc .\installer\installer.c -Wall -lm -o .\bin\installer_windows.exe

echo "[ -DONE- ] Please check the console output for building results."