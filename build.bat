@echo off
if exist build rmdir /s /q build
mkdir build
pushd build

@REM /Zi
set CompilerOptions=/nologo /MD /Zi

set LinkerOptions=user32.lib gdi32.lib winmm.lib

cl %CompilerOptions% ..\main.c %LinkerOptions%


set arg1=%1
IF "%arg1%" == "run" (
    call .\main.exe
)

popd