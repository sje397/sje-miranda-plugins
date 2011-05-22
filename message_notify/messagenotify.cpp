////////////////////////////////////
/// This Miranda plugin (http://www.miranda-im.org) is released under the General Public Licence,
/// available at http://www.gnu.org/copyleft/gpl.html
/// Copyright Scott Ellis 2005 (mail@scottellis.com.au .. www.scottellis.com.au)
////////////////////////////////////

#include "common.h"
#include "options.h"
#include "popups.h"
#include <time.h>

#include "collection.h"

Map<HANDLE, HWND> open_msg_windows;

HINSTANCE hInst;
PLUGINLINK *pluginLink;

MM_INTERFACE mmi;
UTF8_INTERFACE utfi;

bool metacontacts_installed;
bool unicode_system;
bool thread_api = false;

int code_page = CP_ACP;

DWORD focusTimerId = 0;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Message Notify",
	PLUGIN_MAKE_VERSION(0, 4, 1, 0),
	"Show a popup when a message is received",
	"Scott Ellis",
	"mail@scottellis.com.au",
	"© 2005 Scott Ellis",
	"http://www.scottellis.com.au/",
	UNICODE_AWARE,		//not transient
	0,		//doesn't replace anything built-in
	{ 0x715d0a1b, 0x69b7, 0x4553, { 0xa0, 0xdb, 0xae, 0x49, 0x5, 0x5c, 0x2a, 0xe6 } } // {715D0A1B-69B7-4553-A0DB-AE49055C2AE6}
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

static const MUUID interfaces[] = {MIID_MESSAGENOTIFY, MIID_LAST};
extern "C" __declspec(dllexport) const MUUID* MirandaPluginInterfaces(void)
{
	return interfaces;
}


HANDLE hWindowEvent, hEventDbEventAdded, hIdle;
BOOL bIsIdle = FALSE;

bool window_focussed(HWND hwnd, HANDLE hContact) {
	HWND parent;
	while((parent = GetParent(hwnd)) != 0) hwnd = parent; // ensure we have the top level window (need parent window for scriver & tabsrmm)

	// consider minimized window not-focussed
	WINDOWPLACEMENT wp = {0};
	wp.length = sizeof(wp);
	GetWindowPlacement(hwnd, &wp);
	if(hwnd == GetForegroundWindow() && !(wp.showCmd & SW_SHOWMINIMIZED)) {

		// check message API for focus too
		if(options.consider_tabs) {
			MessageWindowInputData mwid = {0};
			MessageWindowData mwd = {0};

			mwid.cbSize = sizeof(mwid);
			mwid.hContact = hContact;
			mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

			mwd.cbSize = sizeof(mwd);

			if(!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd)) {
				//if((mwd.uState & MSG_WINDOW_STATE_VISIBLE) != 0) {
				//	return true;
				//}
				if(IsWindowVisible(mwd.hwndWindow))
					return true;
			} else
				return true;
			
		} else
			return true;
	}

	return false;
}

void __stdcall FocusTimerProc(HWND, UINT, UINT_PTR, DWORD) {
	if(options.notify_when != NOTIFY_NFORE) return;

	/*
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {
		HWND hwnd;
		if((hwnd = (HWND)DBGetContactSettingDword(hContact, MODULE, "WindowHandle", 0)) != 0) {
			if(!EmptyList(hContact)) {
				if(window_focussed(hwnd, hContact))
					PostMessageWindowList(WMU_CLOSEPOPUP, (WPARAM)hContact, 0); // close popups
			}
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
	*/
	for(Map<HANDLE, HWND>::Iterator i = open_msg_windows.start(); i.has_val(); i.next()) {
		if(!EmptyList(i.val().first)) {
			if(window_focussed(i.val().second, i.val().first))
				PostMessageWindowList(WMU_CLOSEPOPUP, (WPARAM)i.val().first, 0); // close popups
		}
	}

}

void StartFocusTimer() {
	if(!focusTimerId)
		focusTimerId = SetTimer(0, 0, 500, FocusTimerProc);
}

void StopFocusTimer() {
	if(focusTimerId) {
		KillTimer(0, focusTimerId);
		focusTimerId = 0;
	}
}



//#define CONTROL_GLOBAL_STATUS

#ifdef CONTROL_GLOBAL_STATUS
WORD saved_status;
#endif

int OnIdleChanged(WPARAM wParam, LPARAM lParam) {
#ifdef CONTROL_GLOBAL_STATUS
	int st;
#endif

	bIsIdle = (lParam & IDF_ISIDLE);

#ifdef CONTROL_GLOBAL_STATUS
	st = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	if(bIsIdle && st != ID_STATUS_OFFLINE) {
		saved_status = st;
		CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)ID_STATUS_AWAY, 0);	
	} else if(!bIsIdle && saved_status != ID_STATUS_OFFLINE) {
		CallService(MS_CLIST_SETSTATUSMODE, (WPARAM)saved_status, 0);
	}
#endif

	return 0;
}

typedef struct {
	HANDLE hContact;
	BYTE *blob;
	int blobsize;
	int flags;
	HANDLE hEvent;
} CheckWindowData;

unsigned int __stdcall sttCheckWindowProc( VOID *dwParam ) {
	//bool debug = (DBGetContactSettingByte(0, MODULE, "Debug", 0) != 0);

	CheckWindowData *cd = (CheckWindowData *)dwParam;
	if(!thread_api) SetEvent(cd->hEvent);

	Sleep(500); // wait for message window to open from event, if it's going to

	//if(debug) PUShowMessage("start of notify thread", SM_NOTIFY);

	HWND hwnd = open_msg_windows[cd->hContact];//(HWND)DBGetContactSettingDword(cd->hContact, MODULE, "WindowHandle", 0);
	bool window_open = (hwnd != 0);
	bool window_has_focus = window_open && window_focussed(hwnd, cd->hContact);

	if(options.notify_when == NOTIFY_ALWAYS
		|| (options.notify_when == NOTIFY_NFORE && !window_has_focus) 
		|| (options.notify_when == NOTIFY_CLOSED && !window_open))
	{
		// show a popup
		if(IsUnicodePopupsEnabled()) {
			//if(debug) PUShowMessage("showing popup (unicode)", SM_NOTIFY);
			// get contact display name from clist
			wchar_t swzContactDisplayName[512];

			if(unicode_system) {
				wcsncpy(swzContactDisplayName, (wchar_t *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)cd->hContact, GCDNF_UNICODE), 512);
			} else {
				char *szCDN = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)cd->hContact, 0);
				if(szCDN && szCDN[0]) {
					MultiByteToWideChar(code_page, 0, (char *) szCDN, -1, swzContactDisplayName, 512);
				} else {
					wcsncpy(swzContactDisplayName, TranslateT("(Unknown)"),  512);
				}
			}

			if(cd->blobsize && options.show_msg) {
				// does blob contain unicode message?
				int msglen = strlen((char *)cd->blob) + 1;
				if(msglen != cd->blobsize && wcslen((wchar_t *)(&cd->blob[msglen]))) {
					// yes
					if(options.flags & MNNF_POPUP) ShowPopupW(cd->hContact, swzContactDisplayName, options.show_msg ? (wchar_t *)(&cd->blob[msglen]) : 0);
					if(options.flags & MNNF_SPEAK) Speak_SayExW("messages", cd->hContact, (wchar_t *)(&cd->blob[msglen]));
				} else  {
					// no, convert to unciode
					wchar_t msg[MAX_SECONDLINE];
					if(cd->flags & DBEF_UTF)
						MultiByteToWideChar(CP_UTF8, 0, (char *) cd->blob, -1, msg, MAX_SECONDLINE);
					else
						MultiByteToWideChar(code_page, 0, (char *) cd->blob, -1, msg, MAX_SECONDLINE);
					if(options.flags & MNNF_POPUP) {
						if(options.show_msg) {
							ShowPopupW(cd->hContact, swzContactDisplayName, msg);
						} else 
							ShowPopupW(cd->hContact, swzContactDisplayName, 0);
					}
					if(options.flags & MNNF_SPEAK) Speak_SayExW("messages", cd->hContact, msg);
				}
			} else { 
				// no message
				if(options.flags & MNNF_POPUP) ShowPopupW(cd->hContact, swzContactDisplayName, 0);
				if(options.flags & MNNF_SPEAK) Speak_SayExW("messages", cd->hContact, L"empty message");
			}
		} else {
			//if(debug) PUShowMessage("showing popup (ansi)", SM_NOTIFY);
			char *szCDN = (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)cd->hContact, 0);
			if(szCDN && szCDN[0]) {
				if(cd->blobsize) {
					if(cd->flags & DBEF_UTF) {
						char *msg = mir_utf8decodecp((char *)cd->blob, code_page, 0);
						if((options.flags & MNNF_POPUP) && options.show_msg) ShowPopup(cd->hContact, szCDN, (char *)msg);
						if(options.flags & MNNF_SPEAK) Speak_SayExA("messages", cd->hContact, (char *)msg);
						mir_free(msg);
					} else {
						if((options.flags & MNNF_POPUP) && options.show_msg) ShowPopup(cd->hContact, szCDN, (char *)cd->blob);
						if(options.flags & MNNF_SPEAK) Speak_SayExA("messages", cd->hContact, (char *)cd->blob);
					}
				} else {
					if(options.flags & MNNF_POPUP) ShowPopup(cd->hContact, szCDN, 0);
					if(options.flags & MNNF_SPEAK) Speak_SayExA("messages", cd->hContact, "empty message");
				}
			}
		}
	} else {
		//if(debug) PUShowMessage("no popup - notify when condition not met", SM_NOTIFY);
	}

	if(cd->blobsize) 
		delete[] cd->blob;
	delete cd;

	return 0;
}

int OnDatabaseEventAdded(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	
	//bool debug = (DBGetContactSettingByte(0, MODULE, "Debug", 0) != 0);
	// safety checks

	int status = CallService(MS_CLIST_GETSTATUSMODE, 0, 0);
	if(status >= ID_STATUS_ONLINE && status <= ID_STATUS_OUTTOLUNCH && options.disable_status[status - ID_STATUS_ONLINE]) {
		//if(debug) PUShowMessage("ignoring event (disabled for this status)", SM_NOTIFY);
		return 0;
	}

	// messages from this contact ignored
	if(CallService(MS_IGNORE_ISIGNORED, (WPARAM)hContact, (LPARAM)IGNOREEVENT_MESSAGE)) {
		//if(debug) PUShowMessage("ignoring event (contact ignored)", SM_NOTIFY);
		return 0;
	}

	// if contact 'not on list', use default ignore setting
	if(DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) && CallService(MS_IGNORE_ISIGNORED, 0, (LPARAM)IGNOREEVENT_MESSAGE)) {
		//if(debug) PUShowMessage("ignoring event (contact not on list and defaulting to ignored)", SM_NOTIFY);
		return 0;
	}

	HANDLE hDbEvent = (HANDLE)lParam;
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);

	if(options.show_msg || (options.flags & MNNF_SPEAK)) {
		if((dbei.cbBlob = (DWORD)CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0)) == (DWORD)-1) {
			//if(debug) PUShowMessage("invalid event", SM_NOTIFY);
			return 0;
		}
		if(dbei.cbBlob > 0) dbei.pBlob = new BYTE[dbei.cbBlob];
	} else 
		dbei.cbBlob = 0;

	if(CallService(MS_DB_EVENT_GET, (WPARAM)hDbEvent, (LPARAM)&dbei)) {
		//if(debug) PUShowMessage("get event failed", SM_NOTIFY);
		if(dbei.pBlob) delete[] dbei.pBlob;
		return 0;
	}

	if ((dbei.flags & DBEF_SENT) || (dbei.eventType != EVENTTYPE_MESSAGE) || (dbei.flags & DBEF_READ)) {
		//if(debug) PUShowMessage("ignoring event (read, sent, or type != message)", SM_NOTIFY);
		if(dbei.pBlob) delete[] dbei.pBlob;
		return 0;
	}

	CheckWindowData *cd = new CheckWindowData;
	cd->hContact = hContact;
	cd->blobsize = dbei.cbBlob;
	cd->flags = dbei.flags;
	cd->blob = dbei.pBlob;

	// spawn a thread to deal with the data
	if(thread_api) {
		//if(debug) PUShowMessage("spawning notify thread (thread api)", SM_NOTIFY);
		FORK_THREADEX_PARAMS tparam = {0};
		tparam.arg = (VOID *)cd;
		tparam.pFunc = sttCheckWindowProc;
		CallService(MS_SYSTEM_FORK_THREAD_EX, 0, (LPARAM)&tparam);
	} else {
		//if(debug) PUShowMessage("spawning notify thread (no thread api)", SM_NOTIFY);
		cd->hEvent = CreateEvent(0, 0, 0, 0);
		CloseHandle((HANDLE)_beginthreadex(0, 0, sttCheckWindowProc, (VOID *)cd, 0, 0));
		WaitForSingleObject(cd->hEvent, INFINITE);
		CloseHandle(cd->hEvent);
	}

	return 0;
}

/*
void __cdecl windowData(void *param) {
	Sleep(500);

	HWND hWnd = (HWND)param;
	char msg[512];
	mir_snprintf(msg, 512, "window: %x, parent: %x, foreground: %x", hWnd, GetParent(hWnd), GetForegroundWindow());
	PUShowMessage(msg, SM_NOTIFY);
}
*/

int OnWindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *mwed = (MessageWindowEventData *)lParam;
	
	switch(mwed->uType) {
		//case MSG_WINDOW_EVT_OPENING:
		case MSG_WINDOW_EVT_OPEN:
			//DBWriteContactSettingDword(mwed->hContact, MODULE, "WindowHandle", (DWORD)mwed->hwndWindow);
			open_msg_windows[mwed->hContact] = mwed->hwndWindow;
			if(options.notify_when == NOTIFY_CLOSED)
				PostMessageWindowList(WMU_CLOSEPOPUP, (WPARAM)mwed->hContact, 0);
			//mir_forkthread(windowData, (void *)mwed->hwndWindow);
			break;
		//case MSG_WINDOW_EVT_CLOSING:
		case MSG_WINDOW_EVT_CLOSE:
			//DBWriteContactSettingDword(mwed->hContact, MODULE, "WindowHandle", 0);
			open_msg_windows.remove(mwed->hContact);
			break;
		default:
			//ShowWarning("Window custom");
			break;
	}

	return 0;
}

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {
	InitUtils(); // depends on popup module being loaded already

	if(ServiceExists(MS_UPDATE_REGISTER)) {
		// register with updater
		Update update = {0};
		char szVersion[16];

		update.cbSize = sizeof(Update);

		update.szComponentName = pluginInfo.shortName;
		update.pbVersion = (BYTE *)CreateVersionString(pluginInfo.version, szVersion);
		update.cpbVersion = strlen((char *)update.pbVersion);

		update.szUpdateURL = UPDATER_AUTOREGISTER;

		update.szBetaUpdateURL = "http://www.scottellis.com.au/miranda_plugins/messagenotify.zip";
		update.szBetaVersionURL = "http://www.scottellis.com.au/miranda_plugins/ver_messagenotify.html";
		update.pbBetaVersionPrefix = (BYTE *)"Message Notify version ";
		
		update.cpbBetaVersionPrefix = strlen((char *)update.pbBetaVersionPrefix);

		CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);
	}

	// register with speak plugin (see m_speak.h)
	Speak_Register(MODULE, "messages", "Messages", "core_main_1");

	metacontacts_installed = (ServiceExists(MS_MC_GETMETACONTACT) != 0);

	hIdle = HookEvent(ME_IDLE_CHANGED, OnIdleChanged);
	hWindowEvent = HookEvent(ME_MSG_WINDOWEVENT, OnWindowEvent);

	if(options.notify_when == NOTIFY_NFORE)
		StartFocusTimer();
	

	return 0;
}

int OnPreShutdown(WPARAM wParam, LPARAM lParam) {
	StopFocusTimer();
	return 0;
}

extern "C" int __declspec(dllexport) Load(PLUGINLINK *link)
{
	pluginLink = link;

	mir_getMMI(&mmi);
	mir_getUTFI(&utfi);

	char szVer[128];
	unicode_system = (CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)sizeof(szVer), (LPARAM)szVer) == 0 && strstr(szVer, "Unicode"));

	INITCOMMONCONTROLSEX icex;

	// Ensure that the common control DLL is loaded (for listview)
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC  = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex); 	

	if(ServiceExists(MS_DB_SETSETTINGRESIDENT)) { // 0.6+
		CallService(MS_DB_SETSETTINGRESIDENT, TRUE, (LPARAM)(MODULE "/WindowHandle"));
	}

	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {
		if(DBGetContactSettingDword(hContact, MODULE, "WindowHandle", 0) != 0) 
			DBWriteContactSettingDword(hContact, MODULE, "WindowHandle", 0);

		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}

	thread_api = (ServiceExists(MS_SYSTEM_FORK_THREAD_EX) != 0);

	InitWindowList();

	LoadOptions();

	if(ServiceExists(MS_LANGPACK_GETCODEPAGE))
		code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	HookEvent(ME_OPT_INITIALISE, OptInit);

	hEventDbEventAdded = HookEvent(ME_DB_EVENT_ADDED, OnDatabaseEventAdded); // hooked early so SRMM can't mark it read too soon

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	UnhookEvent(hIdle);
	UnhookEvent(hWindowEvent);
	UnhookEvent(hEventDbEventAdded);

	DeinitUtils();
	DeinitWindowList();

	return 0;
}
