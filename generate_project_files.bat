@echo off
call premake5.exe --file=premake5.lua vs2022
REM If the exit code is not 0, there is probably an error
IF %ERRORLEVEL% NEQ 0 PAUSE