#include "common.h"
#include "popups.h"
#include "options.h"

#include <tchar.h>

HICON hPopupIcon;
bool unicode_enabled, classes_enabled;

void __stdcall sttOpenSRMMWindowProc( void* dwParam )
{
	//CallService(MS_MSG_SENDMESSAGE, (WPARAM)dwParam, 0);
	
	// simulate double-click on contact
	CallService(MS_CLIST_CONTACTDOUBLECLICKED, (WPARAM)dwParam, 0);
}

void __stdcall sttCloseSRMMWindowProc( void* dwParam )
{
	HANDLE hContact = (HANDLE)dwParam;
	MessageWindowInputData mwid = {0};
	MessageWindowData mwd = {0};

	mwid.cbSize = sizeof(mwid);
	mwid.hContact = hContact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	mwd.cbSize = sizeof(mwd);

	if(!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd)) {
		HWND hwnd = mwd.hwndWindow;//, parent;
		//while((parent = GetParent(hwnd)) != 0) hwnd = parent; // ensure we have the top level window (need parent window for scriver & tabsrmm)
		
		//PostMessage(hwnd, WM_COMMAND, 1023, 0);			// chat close tab? (unfortunately, chat doesn't implement the messaging API to notify us of window open/close events
		PostMessage(hwnd, WM_COMMAND, IDCANCEL, 0);		// srmm/scriver close session button
		PostMessage(hwnd, WM_COMMAND, 1025, 0);			// tabsrmm close session button

		PostMessageWindowList(WMU_CLOSEPOPUP, (WPARAM)hContact, 0); // close popups
	}
}

LRESULT CALLBACK NullWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	HANDLE hContact, lastDbEvent;
	switch( message ) {
		case UM_INITPOPUP:
			hContact = (HANDLE)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)&hContact);
			if(!InList(hContact, hWnd)) AddToWindowList(hContact, hWnd);
			return TRUE;

		case WM_COMMAND: {
			hContact = (HANDLE)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)&hContact);
			CallFunctionAsync( sttOpenSRMMWindowProc, hContact );
			PUDeletePopUp( hWnd );
			return TRUE;
		}

		case WMU_CLOSEPOPUP: {
			hContact = (HANDLE)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)&hContact);
			if((HANDLE)wParam == hContact)
				PUDeletePopUp( hWnd );
			return TRUE;
		}

		case WM_CONTEXTMENU: {
			// mark most recent event read...FIXME (may not be correct event)
			hContact = (HANDLE)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)&hContact);
			lastDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDLAST, (WPARAM)hContact, 0);
			int count = 0;
			while(lastDbEvent && count++ < 10) {
				CallService(MS_DB_EVENT_MARKREAD, (WPARAM)hContact, (LPARAM)lastDbEvent);
				CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hContact, (LPARAM)lastDbEvent);
				lastDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDPREV, (WPARAM)lastDbEvent, 0);
			}
			if(options.close_win)
				CallFunctionAsync( sttCloseSRMMWindowProc, hContact );
			PUDeletePopUp( hWnd );
			return TRUE;
		}

		case UM_FREEPLUGINDATA: {
			hContact = (HANDLE)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)&hContact);
			RemoveFromWindowList(hContact, hWnd);
			return TRUE;
		}

	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void __stdcall sttPopupProc( void* param ){
//unsigned __stdcall sttPopupProc(void *param) {
	POPUPDATAEX* ppd = ( POPUPDATAEX* )param;

	if ( ServiceExists(MS_POPUP_ADDPOPUPEX) )
		CallService( MS_POPUP_ADDPOPUPEX, ( WPARAM )ppd, 0 );
	else 
		if ( ServiceExists(MS_POPUP_ADDPOPUP) )
			CallService( MS_POPUP_ADDPOPUP, ( WPARAM )ppd, 0 );

	free( ppd );

	//return 0;
}

void __stdcall sttPopupProcW( void* param ) {
//unsigned __stdcall sttPopupProcW(void *param) {
	POPUPDATAW* ppd = ( POPUPDATAW* )param;

	if ( ServiceExists(MS_POPUP_ADDPOPUPW) )
		CallService( MS_POPUP_ADDPOPUPW, ( WPARAM )ppd, 0 );

	free( ppd );

	//return 0;
}

void ShowPopup( HANDLE hContact, const char* line1, const char* line2, int flags )
{
	if(CallService(MS_SYSTEM_TERMINATED, 0, 0)) return;

	if ( !ServiceExists( MS_POPUP_ADDPOPUP )) {	
		char title[256];
		sprintf(title, "%s Message", MODULE);

		if(line1 && line2) {
			char *message = new char[strlen(line1) + strlen(line2) + 2]; // newline and null terminator
			sprintf(message, "%s\n%s", line1, line2);
			MessageBoxA( NULL, message, title, MB_OK | MB_ICONINFORMATION );
			delete message;
		} else if(line1) {
			MessageBoxA( NULL, line1, title, MB_OK | MB_ICONINFORMATION );
		} else if(line2) {
			MessageBoxA( NULL, line2, title, MB_OK | MB_ICONINFORMATION );
		}
		return;
	}

	if(classes_enabled) {
		POPUPDATACLASS d = {sizeof(d), "messagenotify"};
		d.pszTitle = line1;
		d.pszText = line2;
		d.hContact = hContact;
		d.PluginData = (void *)hContact;
		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&d);
	} else {
		POPUPDATAEX* ppd = ( POPUPDATAEX* )malloc( sizeof( POPUPDATAEX ));
		memset((void *)ppd, 0, sizeof(POPUPDATAEX));

		ppd->lchContact = hContact;

		HANDLE hMeta = 0;
		if((hMeta = (HANDLE)CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0)) != 0)
			ppd->lchContact = hMeta;

		ppd->lchIcon = hPopupIcon;

		if(line1) strncpy( ppd->lpzContactName, line1, MAX_CONTACTNAME );
		if(line2) strncpy( ppd->lpzText, line2, MAX_SECONDLINE );

		ppd->PluginWindowProc = ( WNDPROC )NullWindowProc;
		ppd->PluginData = NULL;

		ppd->PluginData = (void *)hContact;

		CallFunctionAsync( sttPopupProc, ppd );
	}
}

void ShowPopupW( HANDLE hContact, const wchar_t* line1, const wchar_t* line2, int flags )
{
	if(CallService(MS_SYSTEM_TERMINATED, 0, 0)) return;

	if ( !ServiceExists( MS_POPUP_ADDPOPUPW )) {	
		wchar_t title[256];
		_stprintf(title, _T("%s Message"), _T(MODULE));

		if(line1 && line2) {
			wchar_t *message = new wchar_t[_tcslen(line1) + _tcslen(line2) + 2]; // newline and null terminator
			_stprintf(message, _T("%s\n%s"), line1, line2);
			MessageBoxW( NULL, message, title, MB_OK | MB_ICONINFORMATION );
			delete message;
		} else if(line1) {
			MessageBoxW( NULL, line1, title, MB_OK | MB_ICONINFORMATION );
		} else if(line2) {
			MessageBoxW( NULL, line2, title, MB_OK | MB_ICONINFORMATION );
		}
		return;
	}

	if(classes_enabled) {
		POPUPDATACLASS d = {sizeof(d), "messagenotify"};
		d.pwszTitle = line1;
		d.pwszText = line2;
		d.hContact = hContact;
		d.PluginData = (void *)hContact;
		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&d);
	} else {
		POPUPDATAW* ppd = ( POPUPDATAW* )malloc( sizeof( POPUPDATAW ));
		memset((void *)ppd, 0, sizeof(POPUPDATAW));

		ppd->lchContact = hContact;

		HANDLE hMeta = 0;
		if((hMeta = (HANDLE)CallService(MS_MC_GETMETACONTACT, (WPARAM)hContact, 0)) != 0)
			ppd->lchContact = hMeta;

		ppd->lchIcon = hPopupIcon;

		if(line1) _tcsncpy( ppd->lpwzContactName, line1, MAX_CONTACTNAME );
		if(line2) _tcsncpy( ppd->lpwzText, line2, MAX_SECONDLINE );

		ppd->PluginWindowProc = ( WNDPROC )NullWindowProc;
		ppd->PluginData = NULL;

		ppd->PluginData = (void *)hContact;

		CallFunctionAsync( sttPopupProcW, ppd );
	}
}

void ShowWarning(char *msg) {
	if(ServiceExists(MS_POPUP_SHOWMESSAGE)) {
		char message[1024];
		sprintf(message, "%s: %s", MODULE, msg);
		PUShowMessage(message, SM_WARNING);
	} else {
		char title[512];
		sprintf(title, "%s Warning", MODULE);
		MessageBoxA(0, msg, title, MB_OK | MB_ICONWARNING);
	}
}

void ShowError(char *msg) {
	if(ServiceExists(MS_POPUP_SHOWMESSAGE)) {
		char message[1024];
		sprintf(message, "%s: %s", MODULE, msg);
		PUShowMessage(message, SM_WARNING);
	} else {
		char title[512];
		sprintf(title, "%s Error", MODULE);
		MessageBoxA(0, msg, title, MB_OK | MB_ICONERROR);
	}
}

bool IsUnicodePopupsEnabled() {
	return unicode_enabled;
}

void InitUtils() {
	hPopupIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	unicode_enabled = ServiceExists(MS_POPUP_ADDPOPUPW) ? true : false;
	classes_enabled = ServiceExists(MS_POPUP_REGISTERCLASS) ? true : false;
	if(classes_enabled) {
		POPUPCLASS test = {0};
		test.cbSize = sizeof(test);
		test.flags = PCF_TCHAR;
		test.hIcon = hPopupIcon;
		test.iSeconds = -1;
		test.ptszDescription = TranslateT("Message Notify");
		test.pszName = "messagenotify";
		test.PluginWindowProc = NullWindowProc;
		CallService(MS_POPUP_REGISTERCLASS, 0, (WPARAM)&test);
	}
}

void DeinitUtils() {
	DestroyIcon(hPopupIcon);
}

