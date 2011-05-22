/* Replace "dll.h" with the name of your header */
#include "common.h"
#include "version.h"
#include "resource.h"
#include "options.h"
#include "proto.h"
#include "menu.h"
#include "icons.h"
#include "api.h"
#include "priorities.h"
#include "m_metacontacts.h"
#include "settings.h"
#include "import.h"

///////////////////////////////////////////////
// Common Plugin Stuff
///////////////////////////////////////////////
HINSTANCE hInst;
PLUGINLINK *pluginLink;
MM_INTERFACE mmi;
UTF8_INTERFACE utfi;
HANDLE metaMainThread;
int codepage;

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
	{ 0x5F9B2B98, 0x3412, 0x4671, { 0x89, 0xAD, 0x5B, 0xA9, 0x53, 0x74, 0xC6, 0xA8 } }	
};


extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved) {
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec (dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	return &pluginInfo;
}


// TODO: add any interfaces you implement to this list
static const MUUID interfaces[] = {MIID_PROTOCOL, MIID_METACONTACTS, MIID_LAST};
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
		update.cpbVersion = (int)strlen((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;
		
		// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
		// before the version that we use to locate it on the page
		// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
		// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
#ifdef _UNICODE
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/meta2.zip";
#else
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/meta2_ansi.zip";
#endif
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_meta2.html";
		update.pbBetaVersionPrefix = (BYTE *)"MetaContacts mk2, version ";
		
		update.cpbBetaVersionPrefix = (int)strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	InitIcons();
	InitMenu();

	return 0;
}

HANDLE hModulesLoaded;
extern "C" __declspec (dllexport) int Load(PLUGINLINK *link) {
	pluginLink=link;
	DuplicateHandle( GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &metaMainThread, 0, FALSE, DUPLICATE_SAME_ACCESS );

	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);
	codepage = (int)CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	RedBlackTree<int> test;
	test.add(1);
	test.add(-3);
	test.add(15);
	test.add(4);
	test.add(2);
	test.add(-1);
	char buff[256];
	for(RedBlackTree<int>::Iterator i = test.start(); i.has_val(); i.next()) {
		mir_snprintf(buff, 256, "%d", i.val());
		PUShowMessage(buff, SM_NOTIFY);
	}
	
	test.remove(-1);
	test.remove(2);
	for(RedBlackTree<int>::Iterator i2 = test.start(); i2.has_val(); i2.next()) {
		mir_snprintf(buff, 256, "%d", i2.val());
		PUShowMessage(buff, SM_NOTIFY);
	}
	

	InitOptions();
	InitPriorities();

	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/Status"));
	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/IdleTS"));
	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/Handle"));
	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/WindowOpen"));
	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/IsSubcontact"));
	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/ForceSend"));
	CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/TempDefault"));

	InitProto();
	InitAPI();

	if(DBGetContactSettingByte(0, MODULE, "FirstRun", 1) == 1) {
		ImportOldMetas();
		DBWriteContactSettingByte(0, MODULE, "FirstRun", 0);
	}

	InitSettings();

	// hook modules loaded
	hModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

extern "C" __declspec (dllexport) int Unload(void) {
	DeinitSettings();
	UnhookEvent(hModulesLoaded);
	DeinitMenu();
	DeinitIcons();
	DeinitAPI();
	DeinitProto();
	DeinitPriorities();
	DeinitOptions();

	return 0;
}
