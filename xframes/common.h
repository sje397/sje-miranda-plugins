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

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <malloc.h>
#include <commctrl.h>

#include <newpluginapi.h>

#include <m_system.h>
#include <m_langpack.h>
#include <m_database.h>
#include <m_options.h>
#include <m_clist.h>
#include <m_clui.h>
#include <m_utils.h>

#include <m_cluiframes.h>

#include <m_updater.h>
#include <m_popup.h>

#define MODULE		"XFrames"

extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;

extern HANDLE mainThread;

extern int code_page;

#ifndef MIID_XFRAMES
#define MIID_XFRAMES	{0xd3155cb0, 0xd8d, 0x4e12, { 0x80, 0x24, 0xc8, 0xb6, 0xb8, 0x2e, 0x8b, 0x91}}
#endif

#endif
