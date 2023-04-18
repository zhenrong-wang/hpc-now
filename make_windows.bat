@echo off
:help
if "%~1"=="" (
	echo [ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command.
	echo ^|          build  - ^(re^)build the binaries
      echo ^|          delete - delete the previous binaries
      echo ^|          clear  - remove the 'bin' folder
	echo [ -DONE- ] Exit now.
	exit /b 1
) else if "%~1"=="build" (
	echo [ START: ] Building the binaries now ...
	mkdir .\bin > nul 2>&1
	echo [ -INFO- ] Deleting previously built binaries ^(if exist^)...
	del /s /q /f .\bin\*
	echo [ -INFO- ] Bulding new binaries with the gcc ...
	gcc .\hpcopr\main\hpcopr_main.c -Wall -lm -o .\bin\hpcopr_windows.exe
	gcc .\now-crypto\now-crypto.c -Wall -lm -o .\bin\now-crypto-windows.exe
	gcc .\installer\installer.c -Wall -lm -o .\bin\installer_windows.exe
) else if "%~1"=="delete" (
	echo [ START: ] Deleting the binaries now ...
	del /s /q /f .\bin\*
) else if "%~1"=="clear" (
	echo [ START: ] Deleting the binaries now ...
	del /s /q /f .\bin\*
	echo [ START: ] Removing the bin folder now ...
	rd /s /q .\bin
) else (
	echo [ -INFO- ] Please specify either 'build', 'delete', or 'clear' when running this command.
	echo ^|          build  - ^(re^)build the binaries
      echo ^|          delete - delete the previous binaries
      echo ^|          clear  - remove the 'bin' folder
	echo [ -DONE- ] Exit now.
	exit /b 1
)
echo "[ -DONE- ] Please check the console output for building results."
exit /b 1