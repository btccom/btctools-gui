https://blogs.msdn.microsoft.com/vcblog/2012/10/08/windows-xp-targeting-with-c-in-visual-studio-2012/

Targeting from the Command Line

Visual Studio 2012 solutions and projects which have been switched to the v110_xp toolset can be built from the command line using MSBuild or DEVENV without additional steps. 

However, if you wish to use CL and Link directly, additional steps are needed. Note that the steps below may be automated by creating a batch script.

    Set the path and environment variables for Visual Studio 2012 command-line builds. 
    Set the required SDK paths and compiler flags using the following commands:

    set INCLUDE=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Include;%INCLUDE%
    set PATH=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Bin;%PATH%
    set LIB=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Lib;%LIB%
    set CL=/D_USING_V110_SDK71_;%CL%

    When targeting x64, set the lib path as follows:
    set LIB=%ProgramFiles(x86)%\Microsoft SDKs\Windows\7.1A\Lib\x64;%LIB%
     
    Specify the correct subsystem and subsystem version for the linker based on the type of application you are building. Applications targeting the x86 version of Windows XP must specify subsystem version 5.01, and applications targeting x64 must specify version 5.02.

    For x86 console applications: 
    set LINK=/SUBSYSTEM:CONSOLE,5.01 %LINK%

    For x64 console applications:
    set LINK=/SUBSYSTEM:CONSOLE,5.02 %LINK%
     
    Execute CL and Link as you normally would within the command prompt.
