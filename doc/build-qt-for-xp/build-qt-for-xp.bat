set baseDIR=%cd%
CALL "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64_x86
cd %baseDIR%

SET PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Bin;%PATH%
SET INCLUDE=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Include;%INCLUDE%
SET LIB=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib;%LIB%
SET CL=/D_USING_V110_SDK71_;%CL%

CALL .\configure.bat -mp -confirm-license -opensource -platform win32-msvc2015 -release -static -prefix C:\Qt\5.7.1-static-vs2017-xp -qt-zlib -qt-libpng -qt-libjpeg -opengl desktop -qt-freetype -no-qml-debug -no-angle -nomake tests -nomake examples -no-directwrite -ssl -openssl-linked OPENSSL_LIBS="-lGdi32 -lssleay32 -llibeay32"
pause

jom -j8
pause

jom install
pause
