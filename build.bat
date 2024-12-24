set BTCTOOLS_INCLUDE_DIR=C:\Users\hu60c\lib32\btctools\include
set BTCTOOLS_LIB_DIR=C:\Users\hu60c\lib32\btctools\lib
set LUA_INCLUDE_DIR=C:\Users\hu60c\vcpkg\installed\x86-windows-static\include\luajit
set LUA_LIB_DIR=C:\Users\hu60c\vcpkg\installed\x86-windows-static\lib
set CRYPTOPP_INCLUDE_DIR=C:\Users\hu60c\vcpkg\installed\x86-windows-static\include
set CRYPTOPP_LIB_DIR=C:\Users\hu60c\vcpkg\installed\x86-windows-static\lib

md build
cd build

C:\Users\hu60c\vcpkg\installed\x86-windows-static\tools\qt5\bin\qmake.exe ..\btctools-gui.pro
C:\Users\hu60c\vcpkg\downloads\tools\jom\jom-1.1.3\jom.exe

cd ..
