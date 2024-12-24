set baseDIR=%cd%
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86
cd %baseDIR%

SET PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin;%PATH%
SET INCLUDE=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include;%INCLUDE%
SET LIB=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib;%LIB%
SET CL=/D_USING_V110_SDK71_;%CL%

perl Configure VC-WIN32 no-asm --prefix=build
pause

CALL ms\do_ms
pause

nmake -f ms\nt.mak
pause

nmake -f ms\nt.mak install
pause
