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
#include <richedit.h>
#include <malloc.h>
#include <stdlib.h>

#define MIRANDA_VER	0x0800

#include <newpluginapi.h>
#include <m_system.h>
#include <m_database.h>
#include <m_langpack.h>
#include <m_options.h>
#include <m_protomod.h>
#include <m_protosvc.h>
#include <stdio.h>
#include <m_utils.h>
#include <m_updater.h>
#include <m_popup.h>

#define MODULE						"nohtml"

extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern DWORD mirandaVer;
extern int codepage;

#ifndef MIID_NOHTML
#define MIID_NOHTML	{ 0x9544122C, 0x46DD, 0x4BFA, { 0x8A, 0x5A, 0x5D, 0xDC, 0x3, 0x14, 0x28, 0x93 } }
#endif

#endif
