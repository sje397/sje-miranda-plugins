// MiniDump.cpp : Defines the entry point for the DLL application.
//

#include "common.h"
#include "MiniDump.h"
#include "DebugTools.h"
#include "str_utils.h"
#include "options.h"

HINSTANCE hInst = 0;
PLUGINLINK *pluginLink = 0;
HANDLE hNetlibUser = 0;

MM_INTERFACE mmi;

HANDLE hDumpPath = 0;
TCHAR stzDumpPath[MAX_PATH] = "";

DWORD mirandaVersion = 0;

extern "C" BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	DisableThreadLibraryCalls(hModule);
	hInst = (HINSTANCE)hModule;
    return TRUE;
}

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESC,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	0,		//not transient
	0,		//doesn't replace anything built-in
	{ 0xe3ab632a, 0x693c, 0x41c2, { 0xa7, 0x55, 0xaa, 0xb7, 0xde, 0x4f, 0x1f, 0x7f } } // {E3AB632A-693C-41c2-A755-AAB7DE4F1F7F}
};

extern "C" MINIDUMP_API PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	pluginInfo.cbSize = sizeof(PLUGININFO);
	return (PLUGININFO*)&pluginInfo;
}

static const MUUID interfaces[] = {MIID_ATTACHE, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


void InitNetlib() {
	NETLIBUSER nl_user = {0};
	nl_user.cbSize = sizeof(nl_user);
	nl_user.szSettingsModule = MODULE;
	nl_user.flags = NUF_OUTGOING | NUF_HTTPCONNS;
	nl_user.szDescriptiveName = MODULE;

	hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nl_user);
}

void DeinitNetlib() {
	if(hNetlibUser)
		CallService(MS_NETLIB_CLOSEHANDLE, (WPARAM)hNetlibUser, 0);
}

int FoldersPathChanged(WPARAM wParam, LPARAM lParam) {
	FOLDERSGETDATA fgd = {0};
	fgd.cbSize = sizeof(fgd);
	fgd.nMaxPathSize = MAX_PATH;

	fgd.szPathT = stzDumpPath;
	CallService(MS_FOLDERS_GET_PATH, (WPARAM)hDumpPath, (WPARAM)&fgd);

	return 0;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam) {
	InitNetlib();

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
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/attache.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_attache.html";
		update.pbBetaVersionPrefix = (BYTE *)"Attache version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}	

	if(ServiceExists(MS_FOLDERS_GET_PATH)) {
		FOLDERSDATA fd = {0};
		fd.cbSize = sizeof(fd);
#ifdef _UNICODE
		fd.flags = FF_UNICODE;
#endif
		strncpy(fd.szSection, Translate("Attache"), 64);

		strncpy(fd.szName, Translate("Crash Dumps"), 64);
		fd.szFormatT = _T(PROFILE_PATH) _T(DEFAULT_DUMP_PATH);
		hDumpPath = (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM)&fd);

		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersPathChanged);
		FoldersPathChanged(0, 0);
	}

	return 0;
}

extern "C" MINIDUMP_API int Load(PLUGINLINK *link) {
	pluginLink = link;

	InitializeDebug();

	mmi.cbSize = sizeof(mmi);
	CallService(MS_SYSTEM_GET_MMI, 0, (LPARAM)&mmi);

	LoadOptions();
	set_codepage();

	mirandaVersion = CallService(MS_SYSTEM_GETVERSION, 0, 0);
	
	HookEvent(ME_OPT_INITIALISE, OptInit);

	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);

	SetErrorMode(SEM_NOGPFAULTERRORBOX);

//#define CRASH_ME
#ifdef CRASH_ME
	// test crash
	int *i = 0;
	*i = 10;
#endif

	return 0;
}

extern "C" MINIDUMP_API int Unload() {
	UninitializeDebug();
	DeinitNetlib();
	return 0;
}
