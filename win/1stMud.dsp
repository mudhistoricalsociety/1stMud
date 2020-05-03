# Microsoft Developer Studio Project File - Name="1stMud" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=1stMud - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "1stMud.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "1stMud.mak" CFG="1stMud - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "1stMud - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "1stMud - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "1stMud - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\src\o"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I "..\src\h" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /D "ZLIB_DLL" /YX /FD /c
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x1009 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"."
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "1stMud - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "1stMud___Win32_Debug"
# PROP BASE Intermediate_Dir "1stMud___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\src\o"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /WX /Gm /GX /ZI /Od /I "." /I "..\src\h" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /D "ZLIB_DLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x1009 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"."
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "1stMud - Win32 Release"
# Name "1stMud - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\act_comm.c
# End Source File
# Begin Source File

SOURCE=..\src\act_enter.c
# End Source File
# Begin Source File

SOURCE=..\src\act_info.c
# End Source File
# Begin Source File

SOURCE=..\src\act_move.c
# End Source File
# Begin Source File

SOURCE=..\src\act_obj.c
# End Source File
# Begin Source File

SOURCE=..\src\act_wiz.c
# End Source File
# Begin Source File

SOURCE=..\src\alias.c
# End Source File
# Begin Source File

SOURCE=..\src\ansi.c
# End Source File
# Begin Source File

SOURCE=..\src\arena.c
# End Source File
# Begin Source File

SOURCE=..\src\auction.c
# End Source File
# Begin Source File

SOURCE=..\src\automap.c
# End Source File
# Begin Source File

SOURCE=..\src\ban.c
# End Source File
# Begin Source File

SOURCE=..\src\board.c
# End Source File
# Begin Source File

SOURCE=..\src\buddy.c
# End Source File
# Begin Source File

SOURCE=..\src\channels.c
# End Source File
# Begin Source File

SOURCE=..\src\clans.c
# End Source File
# Begin Source File

SOURCE=..\src\client.c
# End Source File
# Begin Source File

SOURCE=..\src\comm.c
# End Source File
# Begin Source File

SOURCE=..\src\const.c
# End Source File
# Begin Source File

SOURCE=..\src\data_table.c
# End Source File
# Begin Source File

SOURCE=..\src\db.c
# End Source File
# Begin Source File

SOURCE=..\src\db2.c
# End Source File
# Begin Source File

SOURCE=..\src\economy.c
# End Source File
# Begin Source File

SOURCE=..\src\effects.c
# End Source File
# Begin Source File

SOURCE=..\src\explored.c
# End Source File
# Begin Source File

SOURCE=..\src\fight.c
# End Source File
# Begin Source File

SOURCE=..\src\fileio.c
# End Source File
# Begin Source File

SOURCE=..\src\flags.c
# End Source File
# Begin Source File

SOURCE=..\src\gquest.c
# End Source File
# Begin Source File

SOURCE=..\src\handler.c
# End Source File
# Begin Source File

SOURCE=..\src\healer.c
# End Source File
# Begin Source File

SOURCE=..\src\homes.c
# End Source File
# Begin Source File

SOURCE=..\src\hunt.c
# End Source File
# Begin Source File

SOURCE=..\src\i3.c
# End Source File
# Begin Source File

SOURCE=..\src\ignore.c
# End Source File
# Begin Source File

SOURCE=..\src\interp.c
# End Source File
# Begin Source File

SOURCE=..\src\logs.c
# End Source File
# Begin Source File

SOURCE=..\src\lookup.c
# End Source File
# Begin Source File

SOURCE=..\src\magic.c
# End Source File
# Begin Source File

SOURCE=..\src\magic2.c
# End Source File
# Begin Source File

SOURCE=..\src\mccp.c
# End Source File
# Begin Source File

SOURCE=..\src\missing.c
# End Source File
# Begin Source File

SOURCE=..\src\multiclass.c
# End Source File
# Begin Source File

SOURCE=..\src\music.c
# End Source File
# Begin Source File

SOURCE=..\src\namegen.c
# End Source File
# Begin Source File

SOURCE=..\src\nanny.c
# End Source File
# Begin Source File

SOURCE=..\src\olc.c
# End Source File
# Begin Source File

SOURCE=..\src\olc_act.c
# End Source File
# Begin Source File

SOURCE=..\src\olc_save.c
# End Source File
# Begin Source File

SOURCE=..\src\prog_cmds.c
# End Source File
# Begin Source File

SOURCE=..\src\programs.c
# End Source File
# Begin Source File

SOURCE=..\src\quest.c
# End Source File
# Begin Source File

SOURCE=..\src\recycle.c
# End Source File
# Begin Source File

SOURCE=..\src\save.c
# End Source File
# Begin Source File

SOURCE=..\src\scan.c
# End Source File
# Begin Source File

SOURCE=..\src\sendstat.c
# End Source File
# Begin Source File

SOURCE=..\src\signals.c
# End Source File
# Begin Source File

SOURCE=..\src\skills.c
# End Source File
# Begin Source File

SOURCE=..\src\special.c
# End Source File
# Begin Source File

SOURCE=..\src\statlist.c
# End Source File
# Begin Source File

SOURCE=..\src\string.c
# End Source File
# Begin Source File

SOURCE=..\src\tables.c
# End Source File
# Begin Source File

SOURCE=..\src\telnet.c
# End Source File
# Begin Source File

SOURCE=..\src\update.c
# End Source File
# Begin Source File

SOURCE=..\src\war.c
# End Source File
# Begin Source File

SOURCE=..\src\weather.c
# End Source File
# Begin Source File

SOURCE=..\src\webserver.c
# End Source File
# Begin Source File

SOURCE=..\src\wizlist.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
