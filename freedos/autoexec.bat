@echo off
rem autoexec.bat for dosdoor
path z:\\bin;z:\\dosemu
set TEMP=c:\\tmp
prompt $P$G
unix -s DOSDRIVE_D
if "%DOSDRIVE_D%" == "" goto nodrived
lredir del d: > nul
lredir d: linux\\fs%DOSDRIVE_D%
:nodrived
unix -e
