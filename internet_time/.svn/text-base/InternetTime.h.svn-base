// © 2004 Scott Ellis
#ifndef _WORLD_TIME_H
#define _WORLD_TIME_H

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PINGPLUG_VC6_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PINGPLUG_VC6_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef INTERNETTIME_EXPORTS
#define INTERNETTIME_API __declspec(dllexport)
#else
#define INTERNETTIME_API __declspec(dllimport)
#endif

//#include "icmp.h"
#include "plugwin.h"
#include "timezone.h"

//#include "../../include/newpluginapi.h"
#include "../../include/m_clui.h"
#include "../../include/m_system.h"
#include "../../include/m_options.h"

//#include "../../include/m_clist.h"
//#include "../../include/m_skin.h"
//#include "../../include/m_cluiframes.h"
//#include "../../include/m_netlib.h"


extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern PLUGININFO pluginInfo;

extern "C" INTERNETTIME_API PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion);
extern "C" INTERNETTIME_API int Load(PLUGINLINK *link);
extern "C" INTERNETTIME_API int Unload(void);

#endif