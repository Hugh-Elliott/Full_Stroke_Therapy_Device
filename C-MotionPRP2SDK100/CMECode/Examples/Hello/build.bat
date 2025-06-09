@echo off
REM Use this file by itself to build the project
REM Use "build clean" to clean the project
REM set the port for downloading code via the makefile (optional)
REM set PORT=COM1
PATH=..\..\GNU\bin\;..\..\utils\bin\
..\..\utils\bin\make %1 %2