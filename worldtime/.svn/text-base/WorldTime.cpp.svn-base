#include "common.h"
#include "WorldTime.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"World Time",
	PLUGIN_MAKE_VERSION(0, 4, 1, 0),
	"Display world times.",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2004 Scott Ellis",
	"http://scottellis.com.au/",
	0,		//not transient
	0,		//doesn't replace anything built-in
	{ 0x6e8c83d3, 0x418e, 0x4cec, { 0xb3, 0x4e, 0xed, 0x61, 0x3f, 0x51, 0xae, 0x58 } } // {6E8C83D3-418E-4cec-B34E-ED613F51AE58}
};

MM_INTERFACE mmi;
LIST_INTERFACE li;


extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" WORLDTIME_API PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static const MUUID interfaces[] = {MIID_WORLDTIME, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}



void CreatePluginServices() {
}

static int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = (int)strlen((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/WorldTime.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_worldtime.html";
		update.pbBetaVersionPrefix = (BYTE *)"World Time version ";
		
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	
	HWND contact_list_hwnd = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	addmypluginwindow1(contact_list_hwnd);

	return 0;
}

extern "C" WORLDTIME_API int Load(PLUGINLINK *link)
{
	pluginLink=link;

	mir_getMMI(&mmi);
	mir_getLI(&li);

	// ensure datetime picker is loaded
	INITCOMMONCONTROLSEX ccx = {0};
	ccx.dwSize = sizeof(ccx);
	ccx.dwICC = ICC_DATE_CLASSES;
	InitCommonControlsEx(&ccx);

	if(build_timezone_list()) 
	{		
		CreatePluginServices();
		HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);	
		HookEvent( ME_OPT_INITIALISE, WorldTimeOptInit );
	} else {
		MessageBox(0, _T("Unable to read timezone information.\nPlugin disabled."), _T("World Time"), MB_ICONERROR | MB_OK);
	}

	return 0;
}

extern "C" WORLDTIME_API int Unload(void)
{
	plugwin_cleanup();
	return 0;
}

