#include "stdafx.h"
#include "InternetTime.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"Internet Time",
	PLUGIN_MAKE_VERSION(0, 0, 0, 9),
	"Display Swatch Internet Time.",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2004 Scott Ellis",
	"http://scottellis.com.au/",
	0,		//not transient
	0		//doesn't replace anything built-in
};

int winver = 0;

#define VER_9X		1
#define VER_NT40	2
#define VER_2KXP	3

void detect_win_version() {
	OSVERSIONINFO version;
    
	version.dwOSVersionInfoSize = sizeof(version);
	GetVersionEx(&version);

	winver = (version.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ? VER_9X
			   : (version.dwMajorVersion == 4) ? VER_NT40 : VER_2KXP;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" INTERNETTIME_API PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

void CreatePluginServices() {
}

static int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	
	HWND contact_list_hwnd = (HWND)CallService(MS_CLUI_GETHWND, 0, 0);
	addmypluginwindow1(contact_list_hwnd);

	return 0;
}

extern "C" INTERNETTIME_API int Load(PLUGINLINK *link)
{
	pluginLink=link;

/*	
	detect_win_version();
	if(winver < 2) {
		MessageBox(0, "Sorry, this plugin does not work on Win9x (yet).\nPlugin disabled.", "World Time", MB_ICONERROR | MB_OK);
		return 0;
	}
*/
	if(build_timezone_list()) {
		DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, THREAD_SET_CONTEXT, FALSE, 0 );
		

		CreatePluginServices();

		HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);	
	} else {
		MessageBox(0, "Unable to read timezone information.\nPlugin disabled.", "World Time", MB_ICONERROR | MB_OK);
	}

	return 0;
}

extern "C" INTERNETTIME_API int Unload(void)
{
	CloseHandle( mainThread );
	plugwin_cleanup();
	return 0;
}

