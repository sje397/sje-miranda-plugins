/* Replace "dll.h" with the name of your header */
#include "common.h"
#include "private.h"
#include "resource.h"
#include "icons.h"
#include "options.h"
#include "services.h"
#include "iax_interface.h"
#include "menu.h"

///////////////////////////////////////////////
// Common Plugin Stuff
///////////////////////////////////////////////
HINSTANCE hInst;
PLUGINLINK *pluginLink;

// plugin stuff
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	MODULE,
	PLUGIN_MAKE_VERSION(VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD),
	DESC_STRING,
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	0,		//not transient
	0,		//doesn't replace anything built-in
	{ 0x65e1a7f4, 0x1408, 0x4462, { 0xb8, 0xfd, 0x9d, 0xab, 0xb8, 0x37, 0x1f, 0x81 } } // {65E1A7F4-1408-4462-B8FD-9DABB8371F81}
};

extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec (dllexport) PLUGININFOEX* __cdecl MirandaPluginInfoEx(DWORD mirandaVersion) {
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}

extern "C" __declspec (dllexport) PLUGININFO* __cdecl MirandaPluginInfo(DWORD mirandaVersion) {
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
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

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/iax.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_iax.html";
		update.pbBetaVersionPrefix = (BYTE *)"IAX, version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	InitIcons();
	
	PreInitOptions();

	if(!InitIAXInterface()) {
		MessageBox(0, Translate("Failed to initialize IAX Client library. Plugin disabled."), Translate("IAX Plugin error"), MB_OK | MB_ICONERROR);
		return 1;
	}

	InitOptions();
	InitMenu();

	return 0;
}

int PreShutdown(WPARAM wParam, LPARAM lParam) {
	DeinitIAXInterface();	
	DeinitMenu();
	return 0;
}

HANDLE hModulesLoaded, hPreShutdown;
extern "C" __declspec (dllexport) int __cdecl Load(PLUGINLINK *link) {
	pluginLink=link;

	DWORD mirandaVersion = CallService(MS_SYSTEM_GETVERSION, 0, 0);
	if(mirandaVersion < (MIRANDA_VER << 8)) {
		MessageBox(0, Translate("This plugin requires Miranda version 0.6+. Plugin disabled."), Translate("IAX Plugin error"), MB_OK | MB_ICONERROR);
		return 1;
	}

	if(ServiceExists(MS_DB_SETSETTINGRESIDENT)) { // 0.6+
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/Status"));
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/LineNo"));
	}

	INITCOMMONCONTROLSEX icex;

	// Ensure that the common control DLL is loaded (for listview)
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 	

	InitServices();

	PROTOCOLDESCRIPTOR pd = {0};
	pd.cbSize = sizeof(pd);
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL;
	CallService(MS_PROTO_REGISTERMODULE,0,(LPARAM)&pd);

	// since we can call people when not registered...
	SetContactStatus(ID_STATUS_ONLINE);

	// hook modules loaded
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	hPreShutdown = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);
	return 0;
}

extern "C" __declspec (dllexport) int __cdecl Unload(void) {
	DeinitOptions();	
	DeinitServices();
	DeinitIcons();
	return 0;
}
