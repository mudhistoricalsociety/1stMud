
@if "%1" == "" goto error
@if "%1" == "zlib" goto zlib
@if "%1" == "cygwin" goto cygwin
@goto error

:error
@echo Syntax: %0 win32	    - install MSVC exe copy and zlib1.dll.
@echo       : %0 zlib       - install zlib1.dll.
@echo       : %0 cygwin     - install cygwin1.dll.
@goto exit

:zlib
@if exist %windir%\zlib1.dll goto exit
@copy ..\win\zlib1.dll %windir%
@echo zlib1.dll installed to %windir%
@goto exit

:cygwin
@if exist %windir%\cygwin1.dll goto exit
@copy ..\win\cygwin1.dll %windir%
@echo cygwin1.dll installed to %windir%.
@goto exit

:exit
