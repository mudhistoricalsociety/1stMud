@if "%1" == "" goto help

:: Check for WinZip.
@if exist %ProgramFiles%\WinZip\wzzip.exe goto wzzip

:: Then PowerArchiver Command Line version.
@if exist C:\PACL\pacomp.exe goto pacomp

:: Then PowerArchiver.
@if exist %ProgramFiles%\PowerArchiver\powerarc.exe goto powerarc

:: Then PKZIP.
@if exist %ProgramFiles%\PKZIP\pkzip.exe goto pkzip

:: Then Cygwin zip.
@if exist %HOMEDRIVE%\cygwin\bin\zip.exe goto zip

:: Then Cygwin tar.
@if exist %HOMEDRIVE%\cygwin\bin\tar.exe goto tar

goto progerr

:pacomp
@set cprog=C:\PACL\pacomp.exe -r -xrom -x*.o -x*~ -x*.out -xcore
@set filext=
@goto start

:powerarc
@set cprog=%ProgramFiles%\PowerArchiver\powerarc.exe
@set filext=
@goto start

:zip
@set cprog=%HOMEDRIVE%\cygwin\bin\zip.exe
@set filext=
@goto start

:tar
@set cprog=%HOMEDRIVE%\cygwin\bin\tar.exe --exclude='rom' --exclude='*.o' --exclude='*~' --exclude='*.out' --exclude='core' -czf
@set filext=.tar.gz
@goto start

:pkzip
@set cprog=%ProgramFiles%\PKZIP\pkzip.exe -add
@set filext=
@goto start

:wzzip
@set cprog=%ProgramFiles%\WinZip\wzzip.exe
@set filext=
@goto start

:start

@set bdir=..\backup

@echo.|date|find "current" > cuttent.bat
@echo set date=%%4 > current.bat
@set uniq=%date:/=%
@del cuttent.bat
@del current.bat

@if "%1" == "all" goto all
@if "%1" == "player" goto player
@if "%1" == "area" goto area
@if "%1" == "src" goto src
@if "%1" == "clean" goto clean
@if "%1" == "data" goto data

@goto help

@cd ..\

:all
	@%0 player
	@%0 area
	@%0 data
	@%0 src
	@echo Done backup all.
@goto end

:player
	@set file=%bdir%\player.%uniq%%filext%
	@%cprog% %file% player
	@echo Done with %file%.
@goto end

:area
	@set file=%bdir%\area.%uniq%%filext%
        @%cprog% %file% area
        @echo Done with %file%.
@goto end

:data
	@set file=%bdir%\data.%uniq%%filext%
        @%cprog% %file% data
        @echo Done with %file%.
@goto end

:src
	@set file=%bdir%\src.%uniq%%filext%
        @%cprog% %file% src
        @echo Done with %file%.
@goto end

:clean
        @del /Q /F /S %bdir%\*.tar.gz %bdir%\*.zip
        @echo Done cleaning backups.
@goto end

:help
	@echo Usage %0 [all,player,area,src,data,clean]
@goto end

:progerr
@echo Unable to find a compression program.  Try installing cygwin or pkzip.
@goto end

:end
