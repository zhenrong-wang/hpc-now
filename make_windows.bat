:: This code is written and maintained by Zhenrong WANG (mailto: wangzhenrong@hpc-now.com) 
:: The founder of Shanghai HPC-NOW Technologies Co., Ltd (website: https://www.hpc-now.com)
:: It is distributed under the license: GNU Public License - v2.0
:: Bug report: info@hpc-now.com

@echo off
for /f tokens^=2^ delims^=^" %%a in  ('findstr CORE_VERSION_CODE .\\hpcopr\\now_macros.h') do set hpcopr_version_code=%%a
for /f tokens^=2^ delims^=^" %%a in  ('findstr INSTALLER_VERSION_CODE .\\installer\\installer.h') do set installer_version_code=%%a
:help
if "%~1"=="" (
	echo [ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command.
	echo ^|          build  - ^(re^)build the binaries
    echo ^|          delete - delete the previous binaries
    echo ^|          clear  - remove the 'build' folder
	echo [ -DONE- ] Exit now.
	exit /b 1
) else if "%~1"=="build" (
	echo [ START: ] Building the binaries now ...
    echo [ -INFO- ] Please build hpcmgr with GNU/Linux, not Windows.
	mkdir .\build > nul 2>&1
	echo [ -INFO- ] Deleting previously built binaries ^(if exist^)...
	del /s /q /f .\build\*
	echo [ -INFO- ] Bulding new binaries with the gcc ...
	gcc .\hpcopr\*.c -Wall -o .\build\hpcopr-win-%hpcopr_version_code%.exe
	gcc -c .\hpcopr\general_funcs.c -o .\installer\gfuncs.o
	del /f /s /q .\installer\libgfuncs.a
	ar -rc .\installer\libgfuncs.a .\installer\gfuncs.o
	gcc .\installer\installer.c .\installer\libgfuncs.a -Wall -o .\build\installer-win-%installer_version_code%.exe
	move /y .\installer\libgfuncs.a .\build\libgfuncs.a
	del /f /s /q .\installer\gfuncs.o
	gcc .\now-crypto\now-crypto.c -Wall -o .\build\now-crypto-win.exe
	gcc .\now-crypto\now-crypto-v2.c -Wall -o .\build\now-crypto-v2-win.exe
) else if "%~1"=="delete" (
	echo [ START: ] Deleting the binaries now ...
	del /s /q /f .\build\*
) else if "%~1"=="clear" (
	echo [ START: ] Deleting the binaries now ...
	del /s /q /f .\build\*
	echo [ START: ] Removing the build folder now ...
	rd /s /q .\build
) else (
	echo [ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command.
	echo ^|          build  - ^(re^)build the binaries
    echo ^|          delete - delete the previous binaries
    echo ^|          clear  - remove the 'bin' folder
	echo [ -DONE- ] Exit now.
	exit /b 1
)
echo [ -DONE- ] Please check the console output for building results.
exit /b 1