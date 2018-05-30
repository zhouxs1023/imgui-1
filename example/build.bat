@echo off

IF NOT DEFINED BaseDir (
    IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community" (
        call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
        set BaseDir=%CD%
    )
)

IF NOT DEFINED BaseDir (
    IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio 14.0" (
        call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
        set BaseDir=%CD%
    )
)

IF NOT DEFINED BaseDir (
    IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio 13.0" (
        call "C:\Program Files (x86)\Microsoft Visual Studio 13.0\VC\vcvarsall.bat" x64
        set BaseDir=%CD%
    )
)

IF NOT DEFINED BaseDir (
    IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio 12.0" (
        call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64
        set BaseDir=%CD%
    )
)

IF NOT DEFINED BaseDir (
    IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio 11.0" (
        call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x64
        set BaseDir=%CD%
    )
)

IF NOT DEFINED BaseDir (
    IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio 10.0" (
        call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x64
        set BaseDir=%CD%
    )
)

pushd %BaseDir%\build
set "Includes=-I%BaseDir%\include -I%BaseDir%\..\"
set "Libs=SDL2.lib SDL2main.lib opengl32.lib gdi32.lib user32.lib"
REM /d2cgsummary for cl compiler stats on function compilation times
cl -EHa -Z7 -F 67108864 -MTd -Od -fp:fast -D "_ITERATOR_DEBUG_LEVEL=0" -nologo %Includes% %BaseDir%\example.c /link -incremental:no -opt:ref -LIBPATH:%BaseDir%\lib -SUBSYSTEM:CONSOLE %Libs%
popd