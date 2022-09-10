cd /D "%~dp0"
call premake5.exe vs2022 --file="..\premake5.lua"
pause