// xframes.cpp : Defines the entry point for the DLL application.
//

#include "common.h"
#include "xframes.h"
#include "options.h"
#include "frames.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

HANDLE mainThread;

bool metacontacts_installed;
bool unicode_system;

int code_page = CP_ACP;

HANDLE popupWindowList;

DWORD focusTimerId = 0;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"XFrames",
	PLUGIN_MAKE_VERSION(0,1,0,1),
	"External frames support",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	0,		//not transient
	0,		//doesn't replace anything built-in
	{ 0x21ef6349, 0xfe67, 0x463b, { 0x80, 0xf3, 0x93, 0x97, 0xd, 0xb7, 0x3a, 0xd9 } } // {21EF6349-FE67-463b-80F3-93970DB73AD9}
};


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	DisableThreadLibraryCalls(hInst);
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

static const MUUID interfaces[] = {MIID_XFRAMES, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;

		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/xframes.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_xframes.html";
		update.pbBetaVersionPrefix = (BYTE *)"XFrames version ";		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	return 0;
}

int OnPreShutdown(WPARAM wParam, LPARAM lParam) {
	return 0;
}

HANDLE he1, he2;
extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	if(GetModuleHandle(_T("clist_modern")) || GetModuleHandle(_T("clist_nicer")) || GetModuleHandle(_T("clist_mw")))
		return 1;

	pluginLink = link;
	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0 );

	char szVer[128];
	unicode_system = (CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)sizeof(szVer), (LPARAM)szVer) == 0 && strstr(szVer, "Unicode"));
	if(ServiceExists(MS_LANGPACK_GETCODEPAGE)) code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	// Ensure that the common control DLL is loaded (for listview)
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 	

	LoadOptions();
	InitFrames();

	he1 = HookEvent(ME_OPT_INITIALISE, OptInit);
	he2 = HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	DeinitFrames();
	UnhookEvent(he1);
	UnhookEvent(he2);
	return 0;
}
