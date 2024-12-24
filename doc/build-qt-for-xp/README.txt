QT5.7.0在win10下使用visual studio 2015编译（目标平台 xp）
<https://www.cnblogs.com/superbi/p/5672049.html>
====================================================
1. 修改%QTDIR%\qtbase\qmake\Makefile.win32，在 CFLAGS_BARE 后加入 -D_USING_V110_SDK71_
2. 修改%QTDIR%\qtbase\mkspecs\common\msvc-desktop.conf，在DEFINES中加入_USING_V110_SDK71_
   修改QMAKE_LFLAGS_CONSOLE    = /SUBSYSTEM:CONSOLE,5.01
   修改QMAKE_LFLAGS_WINDOWS    = /SUBSYSTEM:WINDOWS,5.01
3. 修改 ./mkspecs/common/msvc-base.conf
   /SUBSYSTEM:CONSOLE@QMAKE_SUBSYSTEM_SUFFIX@ -> /SUBSYSTEM:CONSOLE,5.01
   /SUBSYSTEM:WINDOWS@QMAKE_SUBSYSTEM_SUFFIX@ -> /SUBSYSTEM:WINDOWS,5.01
4. 修改 ./tools/configure/Makefile.win32
   /SUBSYSTEM:CONSOLE -> /SUBSYSTEM:CONSOLE,5.01
5. 在“VS2015 x86本机工具命令提示符”中使用 .\build-qt-for-xp.bat 进行配置，然后使用 nmake 进行编译。


怎样静态编译Qt程序
<http://jingyan.baidu.com/article/363872ec3db7386e4aa16f53.html>
====================================================
1. 打开文件夹下的qmake.conf文件，根据它include的内容再定位到相应的文件。我这里是上级文件夹下的common目录下的msvc-desktop.conf文件。
2. 打开对应的文件后，找到以下编译标志：
      QMAKE_CFLAGS_RELEASE    = -O2 -MD
      QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO += -O2 -MD -Zi
      QMAKE_CFLAGS_DEBUG      = -Zi -MDd
   将其中的MD全部修改为MT（见粗体字，也就是将动态编译修改为静态编译）：
      QMAKE_CFLAGS_RELEASE    = -O2 -MT
      QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO += -O2 -MT -Zi
      QMAKE_CFLAGS_DEBUG      = -Zi -MTd
3. 在“VS2015 x86本机工具命令提示符”中使用“.\configure ……”进行配置，然后使用 nmake 进行编译。


使用Qt5.7.0 VS2015版本生成兼容XP的可执行程序
<http://brightguo.com/how-to-release-qt-app-can-run-on-xp/>
====================================================
实际只需要在pro文件中加入这行：
    QMAKE_LFLAGS_WINDOWS= /SUBSYSTEM:WINDOWS,5.01