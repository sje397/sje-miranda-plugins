#ifndef _COMMON_INC
#define _COMMON_INC

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#if defined( UNICODE ) && !defined( _UNICODE )
#define _UNICODE
#endif

#include <tchar.h>

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <commctrl.h>

#define MIRANDA_VER	0x0700

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <m_protocols.h>
#include <stdio.h> // for m_utils
#include <malloc.h> // for m_utils
#include <winsock2.h> // for FD_SETSIZE
#include <m_netlib.h>
#include <m_clist.h>
#include <m_utils.h>
#include <m_langpack.h>
#include <m_idle.h>
#include <m_skin.h>
#include <m_message.h>
#include <m_clui.h>

#include <m_yapp.h>
#include <m_updater.h>
#include <m_metacontacts.h>
#include <m_avatars.h>

extern MM_INTERFACE mmi;

////////////
// included for backward compatibility
#ifndef CMIF_UNICODE
#define CMIF_UNICODE        512      //will return TCHAR* instead of char*
#if defined( _UNICODE )
	#define CMIF_TCHAR       CMIF_UNICODE      //will return TCHAR* instead of char*
#else
	#define CMIF_TCHAR       0       //will return char*, as usual
#endif
#endif
////////////

extern char MODULE[256];

#define CLIENT_VER					673

#define MAX_MESSAGE_SIZE		975


extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern HANDLE mainThread;
extern int code_page;

void SetAllOffline();

extern char mir_ver[];

#define EVENTTYPE_ZAP          EVENTTYPE_MESSAGE //25368 is another special number for status change events that SRMM's will display (thx tweety)

#endif
