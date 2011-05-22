# Microsoft Developer Studio Project File - Name="MySpace" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=MySpace - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MySpace.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MySpace.mak" CFG="MySpace - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MySpace - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MySpace - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MySpace - Win32 Debug (Unicode)" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "MySpace - Win32 Release (Unicode)" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MySpace - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MySpace"
# PROP BASE Intermediate_Dir "MySpace"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "MySpace"
# PROP Intermediate_Dir "MySpace"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /GX /Zi /Od /I "../../include" /D "_DEBUG" /D "_WINDOWS" /D "_USERDLL" /YX /GZ /c
# ADD CPP /nologo /MTd /GX /Zi /Od /I "../../include" /D "_DEBUG" /D "_WINDOWS" /D "_USERDLL" /YX /GZ /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /pdbtype:sept

!ELSEIF  "$(CFG)" == "MySpace - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MySpace"
# PROP BASE Intermediate_Dir "MySpace"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "MySpace"
# PROP Intermediate_Dir "MySpace"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /GX /I "../../include" /D "NDEBUG" /D "_WINDOWS" /D "_USERDLL" /YX /c
# ADD CPP /nologo /MD /GX /O1 /I "../../include" /D "NDEBUG" /D "_WINDOWS" /D "_USERDLL" /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:IX86 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:none /machine:IX86 /out:"release\MySpace.dll"

!ELSEIF  "$(CFG)" == "MySpace - Win32 Debug (Unicode)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "MySpace"
# PROP BASE Intermediate_Dir "MySpace"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "MySpace"
# PROP Intermediate_Dir "MySpace"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /GX /Zi /Od /I "../../include" /D "_DEBUG" /D "_WINDOWS" /D "_USERDLL" /D "UNICODE" /D "_UNICODE" /YX /GZ /c
# ADD CPP /nologo /MTd /GX /Zi /Od /I "../../include" /D "_DEBUG" /D "_WINDOWS" /D "_USERDLL" /D "UNICODE" /D "_UNICODE" /YX /GZ /c
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:IX86 /pdbtype:sept

!ELSEIF  "$(CFG)" == "MySpace - Win32 Release (Unicode)"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "MySpace"
# PROP BASE Intermediate_Dir "MySpace"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "MySpace"
# PROP Intermediate_Dir "MySpace"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /GX /I "../../include" /D "NDEBUG" /D "_WINDOWS" /D "_USERDLL" /D "UNICODE" /D "_UNICODE" /YX /c
# ADD CPP /nologo /MD /GX /O1 /I "../../include" /D "NDEBUG" /D "_WINDOWS" /D "_USERDLL" /D "UNICODE" /D "_UNICODE" /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /win32
# ADD MTL /nologo /win32
# ADD BASE RSC /l 0x409
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:IX86 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /pdb:none /machine:IX86 /out:"release_unicode\MySpace.dll"

!ENDIF 

# Begin Target

# Name "MySpace - Win32 Debug"
# Name "MySpace - Win32 Release"
# Name "MySpace - Win32 Debug (Unicode)"
# Name "MySpace - Win32 Release (Unicode)"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cc;cxx;def;odl;idl;hpj;bat;asm;asmx"
# Begin Source File

SOURCE=.\arc4.cpp
DEP_CPP_ARC4_=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\arc4.h"\
	".\common.h"\
	
# End Source File
# Begin Source File

SOURCE=.\common.cpp
DEP_CPP_COMMO=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\common.h"\
	

!IF  "$(CFG)" == "MySpace - Win32 Debug"

# ADD CPP /nologo /GX /Yc"common.h" /GZ

!ELSEIF  "$(CFG)" == "MySpace - Win32 Release"

# ADD CPP /nologo /GX /Yc"common.h"

!ELSEIF  "$(CFG)" == "MySpace - Win32 Debug (Unicode)"

# ADD CPP /nologo /GX /Yc"common.h" /GZ

!ELSEIF  "$(CFG)" == "MySpace - Win32 Release (Unicode)"

# ADD CPP /nologo /GX /Yc"common.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\menu.cpp
DEP_CPP_MENU_=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\common.h"\
	".\menu.h"\
	
# End Source File
# Begin Source File

SOURCE=.\MySpace.cpp
DEP_CPP_MYSPA=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\arc4.h"\
	".\common.h"\
	".\menu.h"\
	".\net.h"\
	".\options.h"\
	".\proto.h"\
	".\version.h"\
	
# End Source File
# Begin Source File

SOURCE=.\net.cpp
DEP_CPP_NET_C=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\common.h"\
	".\net.h"\
	
# End Source File
# Begin Source File

SOURCE=.\NetMessage.cpp
DEP_CPP_NETME=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\collection.h"\
	".\common.h"\
	".\NetMessage.h"\
	
# End Source File
# Begin Source File

SOURCE=.\nick_dialog.cpp
DEP_CPP_NICK_=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\collection.h"\
	".\common.h"\
	".\NetMessage.h"\
	".\nick_dialog.h"\
	".\server_con.h"\
	
# End Source File
# Begin Source File

SOURCE=.\options.cpp
DEP_CPP_OPTIO=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\common.h"\
	".\options.h"\
	
# End Source File
# Begin Source File

SOURCE=.\proto.cpp
DEP_CPP_PROTO=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\collection.h"\
	".\common.h"\
	".\NetMessage.h"\
	".\proto.h"\
	".\server_con.h"\
	
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;inc;xsd"
# Begin Source File

SOURCE=.\arc4.h
# End Source File
# Begin Source File

SOURCE=.\collection.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\menu.h
# End Source File
# Begin Source File

SOURCE=.\net.h
# End Source File
# Begin Source File

SOURCE=.\NetMessage.h
# End Source File
# Begin Source File

SOURCE=.\nick_dialog.h
# End Source File
# Begin Source File

SOURCE=.\options.h
# End Source File
# Begin Source File

SOURCE=.\proto.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\server_con.cpp
DEP_CPP_SERVE=\
	"..\..\include\m_clist.h"\
	"..\..\include\m_database.h"\
	"..\..\include\m_langpack.h"\
	"..\..\include\m_netlib.h"\
	"..\..\include\m_options.h"\
	"..\..\include\m_plugins.h"\
	"..\..\include\m_popup.h"\
	"..\..\include\m_protocols.h"\
	"..\..\include\m_protomod.h"\
	"..\..\include\m_protosvc.h"\
	"..\..\include\m_system.h"\
	"..\..\include\m_utils.h"\
	"..\..\include\newpluginapi.h"\
	"..\..\include\statusmodes.h"\
	".\arc4.h"\
	".\collection.h"\
	".\common.h"\
	".\net.h"\
	".\NetMessage.h"\
	".\nick_dialog.h"\
	".\options.h"\
	".\server_con.h"\
	
# End Source File
# Begin Source File

SOURCE=.\server_con.h
# End Source File
# Begin Source File

SOURCE=.\version.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav"
# Begin Source File

SOURCE=.\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\menu.ico
# End Source File
# Begin Source File

SOURCE=.\MySpace.rc
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\resource.rc
# End Source File
# Begin Source File

SOURCE=.\version.rc
# PROP Exclude_From_Build 1
# End Source File
# End Group
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Target
# End Project
