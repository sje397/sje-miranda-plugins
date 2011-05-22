/* Replace "dll.h" with the name of your header */
#include "common.h"
#include "version.h"
#include "resource.h"
#include "options.h"
#include "filter.h"

///////////////////////////////////////////////
// Common Plugin Stuff
///////////////////////////////////////////////
HINSTANCE hInst;
PLUGINLINK *pluginLink;
DWORD mirandaVer;
int codepage;

struct MM_INTERFACE mmi;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	//META_PROTO,
	__PLUGIN_NAME,		// altered here and on file listing, so as not to match original
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,		//not transient
	0,
	// TODO: generate your own GUID!!
	{ 0xD78C8249, 0xE8DB, 0x46B1, { 0x92, 0xF3, 0x88, 0xE6, 0xDC, 0x96, 0x4E, 0x8D } }	
};


extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec (dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	mirandaVer = mirandaVersion;
	return &pluginInfo;
}

extern "C" __declspec (dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) {
	mirandaVer = mirandaVersion;
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

static const MUUID interfaces[] = {MIID_NOHTML, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam) {
	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);
		update.szBetaChangelogURL = "https://server.scottellis.com.au/wsvn/mim_plugs/nohtml/?op=log&rev=0&sc=0&isdir=1";

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/nohtml.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_nohtml.html";
		update.pbBetaVersionPrefix = (BYTE *)"No HTML version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

#ifdef _DEBUG
	// show miranda version
	char str[20];
	str[0] = '0';
	str[1] = 'x';
	_itoa(mirandaVer, str+2, 16);
	PUShowMessage(str, SM_NOTIFY);
#endif

	return 0;
}

HANDLE hModulesLoaded;
extern "C" __declspec (dllexport) int Load(PLUGINLINK *link) {
	pluginLink=link;

	codepage = (int)CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);
	mir_getMMI(&mmi);

	InitOptions();

	/////////////
	////// init filter
	RegisterFilter();
	CreateFilterServices();
	AddFilterToContacts();	

	// hook modules loaded
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

extern "C" __declspec (dllexport) int Unload(void) {
	UnhookEvent(hModulesLoaded);
	DeinitFilter();
	DeinitOptions();

	return 0;
}
