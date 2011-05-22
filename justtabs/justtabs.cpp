/*
Miranda plugin template, originally by Richard Hughes
http://miranda-icq.sourceforge.net/

This file is placed in the public domain. Anybody is free to use or
modify it as they wish with no restriction.
There is no warranty.
*/

#include "win.h"
#include "options.h"

HINSTANCE hInst;
PLUGINLINK *pluginLink;
MM_INTERFACE memoryManagerInterface = {0};

HANDLE hEventClistDblClick, hEventWindow, hEventIconChanged;

typedef LRESULT (CALLBACK *WNDPROC)(HWND, UINT, WPARAM, LPARAM);

FontID font_id_tabs = {0};
FontIDW font_id_tabsw = {0};
HFONT hFontTabs = 0;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	MODULE,
	PLUGIN_MAKE_VERSION(0,1,2,0),
	"Put SRMM windows into a single frame",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2006 Scott Ellis",
	"http://scottellis.com.au",
	UNICODE_AWARE,		//not transient
	0,					//doesn't replace anything built-in
	{ 0xef551a22, 0xc851, 0x4983, { 0xa3, 0x33, 0x6c, 0x7, 0x68, 0xa0, 0xb1, 0xb2 } } // {EF551A22-C851-4983-A333-6C0768A0B1B2}
};

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
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

static const MUUID interfaces[] = {MIID_JUSTTABS, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


static int CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) {
		case WM_COMMAND:
			if (HIWORD(wParam) == STN_CLICKED) { //It was a click on the Popup.
				PUDeletePopUp(hWnd);
				return TRUE;
			}
			break;
		case UM_FREEPLUGINDATA: {
			//MY_PLUGIN_DATA * mpd = NULL;
			//mpd = (MY_PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd,(LPARAM)mpd);
			//if (mdp > 0) free(mpd);
			return TRUE; //TRUE or FALSE is the same, it gets ignored.
		}
		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void ShowPopup(HANDLE hContact, const TCHAR *msg) {
	if(ServiceExists(MS_POPUP_ADDPOPUPT)) {
		POPUPDATAT ppd;
		TCHAR *lptzContactName;

		ZeroMemory(&ppd, sizeof(ppd)); //This is always a good thing to do.
		ppd.lchContact = hContact; //Be sure to use a GOOD handle, since this will not be checked.
		ppd.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);

		lptzContactName = (TCHAR *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, (LPARAM)GCDNF_TCHAR);
	
		_tcscpy(ppd.lptzContactName, lptzContactName);
		_tcscpy(ppd.lptzText, msg);
		ppd.colorBack = GetSysColor(COLOR_BTNFACE);;
		ppd.colorText = RGB(0,0,0);
		ppd.PluginWindowProc = (WNDPROC)PopupDlgProc;
		ppd.PluginData = 0;
		ppd.iSeconds = 3;

		//Now that every field has been filled, we want to see the popup.
		CallService(MS_POPUP_ADDPOPUPT, (WPARAM)&ppd, 0);
	}
}

int WindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *ed = (MessageWindowEventData *)lParam;

	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)ed->hContact, 0);
	if(proto && DBGetContactSettingByte(ed->hContact, proto, "ChatRoom", 0)) return 0; // ignore chat room contacts

	switch(ed->uType) {
	case MSG_WINDOW_EVT_OPENING:
		SendMessage(pluginwind, WM_PREADDCHILD, (WPARAM)ed->hwndWindow, (LPARAM)ed->hContact);
		break;
	case MSG_WINDOW_EVT_OPEN:
		PostMessage(pluginwind, WM_ADDCHILD, (WPARAM)ed->hwndWindow, (LPARAM)ed->hContact);
		break;
	case MSG_WINDOW_EVT_CLOSING:
		SendMessage(pluginwind, WM_REMCHILD, (WPARAM)ed->hwndWindow, (LPARAM)ed->hContact);
		break;
	}

	return 0;
}

int ClistDblClick(WPARAM wParam, LPARAM lParam) {
	PostMessage(pluginwind, WM_SHOWCONTACTWND, wParam, 0);
	return 0;
}

int ContactIconChanged(WPARAM wParam, LPARAM lParam) {
	PostMessage(pluginwind, WM_RESETTABICONS, wParam, lParam);
	return 0;
}

int ReloadFont(WPARAM wParam, LPARAM lParam) {
	if(ServiceExists(MS_FONT_GETW)) {
		LOGFONTW log_font;
		if(hFontTabs) DeleteObject(hFontTabs);
		CallService(MS_FONT_GETW, (WPARAM)&font_id_tabsw, (LPARAM)&log_font);
		hFontTabs = CreateFontIndirectW(&log_font);
	} else {
		LOGFONTA log_font;
		if(hFontTabs) DeleteObject(hFontTabs);
		CallService(MS_FONT_GET, (WPARAM)&font_id_tabs, (LPARAM)&log_font);
		hFontTabs = CreateFontIndirectA(&log_font);
	}
	SendMessage(pluginwind, WM_FONTRELOAD, 0, 0);
	return 0;
}

static int MainInit(WPARAM wParam,LPARAM lParam) {
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
		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/justtabs.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_justtabs.html";
		update.pbBetaVersionPrefix = (BYTE *)MODULE " version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	// plugin only works for srmm classic
	if(GetModuleHandle(_T("srmm"))) {
		CreateFrame((HWND)CallService(MS_CLUI_GETHWND, 0, 0), hInst);
		hEventWindow = HookEvent(ME_MSG_WINDOWEVENT, WindowEvent);
		hEventClistDblClick = HookEvent(ME_CLIST_DOUBLECLICKED, ClistDblClick);
		hEventIconChanged = HookEvent(ME_CLIST_CONTACTICONCHANGED, ContactIconChanged);
	}
	
	if(ServiceExists(MS_FONT_REGISTERW)) {
		font_id_tabsw.cbSize = sizeof(FontIDW);
		font_id_tabsw.flags = FIDF_ALLOWEFFECTS;
		//wcsncpy(font_id_tabsw.group, TranslateW("Container"), 64);
		//wcsncpy(font_id_tabsw.name, TranslateW("Tabs"), 64);
		wcsncpy(font_id_tabsw.group, TranslateW(L"Container"), 64);
		wcsncpy(font_id_tabsw.name, TranslateW(L"Tabs"), 64);
		strcpy(font_id_tabsw.dbSettingsGroup, MODULE);
		strcpy(font_id_tabsw.prefix, "FontTabs");
		font_id_tabsw.order = 0;

		CallService(MS_FONT_REGISTERW, (WPARAM)&font_id_tabsw, 0);
		ReloadFont(0, 0);
		HookEvent(ME_FONT_RELOAD, ReloadFont);
	} else if(ServiceExists(MS_FONT_REGISTER)) {
		font_id_tabs.cbSize = sizeof(FontID);
		font_id_tabs.flags = FIDF_ALLOWEFFECTS;
		strncpy(font_id_tabs.group, Translate("Container"), 64);
		strncpy(font_id_tabs.name, Translate("Tabs"), 64);
		strcpy(font_id_tabs.dbSettingsGroup, MODULE);
		strcpy(font_id_tabs.prefix, "FontTabs");
		font_id_tabs.order = 0;

		CallService(MS_FONT_REGISTER, (WPARAM)&font_id_tabs, 0);
		ReloadFont(0, 0);
		HookEvent(ME_FONT_RELOAD, ReloadFont);
	} else {
		LOGFONT lf = {0};
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfHeight = -14;
		lf.lfWeight = FW_BOLD;
		hFontTabs = CreateFontIndirect(&lf);
	}
	
	return 0;
}

static int MainDeInit(WPARAM wParam, LPARAM lParam) {
	if(hEventWindow) UnhookEvent(hEventWindow);
	if(hEventClistDblClick) UnhookEvent(hEventClistDblClick);
	if(hEventIconChanged) UnhookEvent(hEventIconChanged);
	if(pluginwind) {
		DestroyWindow(pluginwind);
		pluginwind = 0;
	}
	return 0;
}

static int DbEventAdded(WPARAM wParam, LPARAM lParam) {
    DBEVENTINFO dbei;

    ZeroMemory(&dbei, sizeof(dbei));
    dbei.cbSize = sizeof(dbei);
    dbei.cbBlob = 0;
    CallService(MS_DB_EVENT_GET, lParam, (LPARAM) & dbei);

	if((dbei.flags & DBEF_READ) || (dbei.flags & DBEF_SENT))
		return 0;

	SendMessage(pluginwind, WM_HIGHLIGHTCONTACTWND, wParam, 0);
	//SendMessage(pluginwind, WM_SHOWCONTACTWND, wParam, 0);

	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;

	memoryManagerInterface.cbSize = sizeof(memoryManagerInterface);
	CallService(MS_SYSTEM_GET_MMI, 0, (LPARAM)&memoryManagerInterface);

	HookEvent(ME_SYSTEM_MODULESLOADED,MainInit);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, MainDeInit);

	HookEvent(ME_DB_EVENT_ADDED, DbEventAdded);

	InitOptions();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

