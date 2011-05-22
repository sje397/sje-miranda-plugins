/* Replace "dll.h" with the name of your header */
#include "common.h"
#include "version.h"
#include "resource.h"
#include "options.h"
#include "proto.h"
#include "net.h"
#include "menu.h"
#include "arc4.h"
#include "notifications.h"
#include "srmm_icon.h"
#include "links.h"

///////////////////////////////////////////////
// Common Plugin Stuff
///////////////////////////////////////////////
HINSTANCE hInst;
PLUGINLINK *pluginLink;
HANDLE mainThread;
int code_page;

char MODULE[256];

MM_INTERFACE mmi;

char mir_ver[256];

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
	{ 0xA0D1D7B2, 0x6B1, 0x437D, { 0x84, 0x91, 0xA7, 0x46, 0x58, 0xB9, 0x63, 0x70 } }	
};


extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec (dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	if (mirandaVersion < PLUGIN_MAKE_VERSION(0, 7, 0, 32))
		return 0;
	return &pluginInfo;
}


static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_LAST};
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
		update.szBetaChangelogURL = "https://server.scottellis.com.au/wsvn/mim_plugs/MySpace/?op=log&rev=0&sc=0&isdir=1";

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
#ifdef _UNICODE
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/MySpace.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_MySpace.html";
#else
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/MySpace_ansi.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_MySpace_ansi.html";
#endif
		update.pbBetaVersionPrefix = (BYTE *)"MySpace version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	InitNetlib();
	InitMenu();
	InitNotifications();
	InitSRMMIcon();

	return 0;
}

void SetAllOffline() {
	char *proto;
	HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
	while ( hContact != NULL )
	{
		proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
		if ( proto && !strcmp( MODULE, proto)) {
			DBWriteContactSettingWord(hContact, MODULE, "Status", ID_STATUS_OFFLINE);
			DBWriteContactSettingDword(hContact, MODULE, "IdleTS", 0);
		}
		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}

HANDLE hModulesLoaded;
extern "C" __declspec (dllexport) int Load(PLUGINLINK *link) {
	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &mainThread, 0, FALSE, DUPLICATE_SAME_ACCESS );

	pluginLink=link;

	code_page = (int)CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);


	mir_getMMI(&mmi);
	// Get module name from DLL file name
	{
		char* str1;
		char str2[MAX_PATH];

		GetModuleFileNameA(hInst, str2, MAX_PATH);
		str1 = strrchr(str2, '\\');
		if (str1 != NULL && strlen(str1+1) > 4) {
			strncpy(MODULE, str1+1, strlen(str1+1)-4);
			MODULE[strlen(str1+1)-3] = 0;
		} 
		CharUpperA(MODULE);
	}

	char temp[256];
	CallService(MS_SYSTEM_GETVERSIONTEXT, 256, (LPARAM)temp);
	mir_snprintf(mir_ver, 256, "Miranda IM %s (MySpace %d.%d.%d.%d)", temp, __FILEVERSION_STRING);

	if(ServiceExists(MS_DB_SETSETTINGRESIDENT)) { // 0.6+
		char buff[256];
		mir_snprintf(buff, 256, "%s/%s", MODULE, "Status");
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)buff);
		mir_snprintf(buff, 256, "%s/%s", MODULE, "StatusMsg");
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)buff);
		mir_snprintf(buff, 256, "%s/%s", MODULE, "IdleTS");
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)buff);
	}

	InitARC4Module();
	InitOptions();

	/////////////
	////// init protocol
	RegisterProto();
	CreateProtoServices();

	SetAllOffline();

	myspace_links_init();

	// hook modules loaded
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

extern "C" __declspec (dllexport) int Unload(void) {
	UnhookEvent(hModulesLoaded);
	DeinitSRMMIcon();
	DeinitMenu();
	DeinitNetlib();
	DeinitProto();

	myspace_links_destroy();
	DeinitOptions();
	DeinitNotifications();
	DeinitARC4Module();

	return 0;
}
