/* Replace "dll.h" with the name of your header */
#include "common.h"
#include "version.h"
#include "resource.h"
[!if OPTIONS_PAGE]
#include "options.h"
[!endif]
[!if FILTER_CODE]
#include "filter.h"
[!endif]
[!if USE_NETLIB]
#include "net.h"
[!endif]
[!if MAIN_MENU || CONTACT_MENU]
#include "menu.h"
[!endif]

///////////////////////////////////////////////
// Common Plugin Stuff
///////////////////////////////////////////////
HINSTANCE hInst;
PLUGINLINK *pluginLink;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	0,
	[!output GUID_PLUGIN]	
};


extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec (dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	return &pluginInfo;
}


/*
// uncomment this for pre 0.7 compatibility
extern "C" __declspec (dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) {
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}
*/

// TODO: add any interfaces you implement to this list
static const MUUID interfaces[] = {MIID_[!output SAFE_PROJECT_NAME_CAPS], MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam) {
[!if USE_NETLIB]
	InitNetlib();
[!endif]
[!if MAIN_MENU || CONTACT_MENU]
	InitMenu();
[!endif]

	return 0;
}

HANDLE hModulesLoaded;
extern "C" __declspec (dllexport) int Load(PLUGINLINK *link) {
	pluginLink=link;

[!if OPTIONS_PAGE]
	InitOptions();
[!endif]
[!if FILTER_CODE]

	/////////////
	////// init filter
	RegisterFilter();
	CreateFilterServices();
	AddFilterToContacts();	
[!endif]

	// hook modules loaded
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

extern "C" __declspec (dllexport) int Unload(void) {
	UnhookEvent(hModulesLoaded);
[!if MAIN_MENU || CONTACT_MENU]
	DeinitMenu();
[!endif]
[!if USE_NETLIB]
	DeinitNetlib();
[!endif]
[!if FILTER_CODE]
	DeinitFilter();
[!endif]
[!if OPTIONS_PAGE]
	DeinitOptions();
[!endif]

	return 0;
}
