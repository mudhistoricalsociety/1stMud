
@echo off
setlocal

cd ..\area

set shutd=..\data\shutdown.txt
if exist %shutd% del %shutd%

set bina=..\bin\rom.exe

:while1
echo Starting 1stMud service.
%bina% %1 %2 %3 %4 %5 %6 %7 %8 %9
if not exist %shutd% goto :while1

echo Shutting down 1stMud service.
del %shutd%
endlocal
pause
