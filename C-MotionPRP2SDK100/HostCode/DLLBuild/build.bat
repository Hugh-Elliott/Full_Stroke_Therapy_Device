REM Rebuild both the release and debug versions of C-Motion.dll.
REM Delete the /REBUILD argument to only build whatever is out of date.
REM This requires that your environment looks like vcvars32.bat has been run.

REM Visual Studio 6.0
REM msdev  DLLBuild.dsw /MAKE "all - Win32 Debug" /REBUILD
REM msdev  DLLBuild.dsw /MAKE "all - Win32 Release" /REBUILD

REM Visual Studio .NET
REM devenv DLLBuild.sln /rebuild Debug
REM devenv DLLBuild.sln /rebuild Release

REM Visual Studio Express
vcexpress DLLBuild.sln /rebuild Debug
vcexpress DLLBuild.sln /rebuild Release
