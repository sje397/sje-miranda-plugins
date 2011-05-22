// © 2004 Scott Ellis
#ifndef _WORLD_TIME_H
#define _WORLD_TIME_H

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PINGPLUG_VC6_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PINGPLUG_VC6_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef WORLDTIME_EXPORTS
#define WORLDTIME_API __declspec(dllexport)
#else
#define WORLDTIME_API __declspec(dllimport)
#endif

#include "plugwin.h"
#include "timezone.h"

#include <m_clui.h>
#include <m_options.h>

#include "m_updater.h"

extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;
extern PLUGININFOEX pluginInfo;

extern "C" WORLDTIME_API PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion);
extern "C" WORLDTIME_API int Load(PLUGINLINK *link);
extern "C" WORLDTIME_API int Unload(void);

#ifndef MIID_WORLDTIME
#define MIID_WORLDTIME		{0x5e436914, 0x3258, 0x4dfa, { 0x8a, 0xd9, 0xf2, 0xcd, 0x26, 0x43, 0x6f, 0x1}}
#endif

#endif