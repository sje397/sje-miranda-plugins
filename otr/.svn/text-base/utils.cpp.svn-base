#include "common.h"
#include "utils.h"
#include "options.h"
#include "menu.h"

#include <shlobj.h>

HICON hProtoIcon, hLockIcon, hUnlockIcon;
HANDLE hIcoLibIconsChanged = 0;

char popup_class_name[128];
LRESULT CALLBACK NullWindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message ) {
		case WM_COMMAND: {
			PUDeletePopUp( hWnd );
			break;
		}

		case WM_CONTEXTMENU:
			PUDeletePopUp( hWnd );
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CALLBACK sttMainThreadCallback( ULONG dwParam )
{
	POPUPDATAEX* ppd = ( POPUPDATAEX* )dwParam;

	if ( ServiceExists(MS_POPUP_ADDPOPUPEX) )
		CallService( MS_POPUP_ADDPOPUPEX, ( WPARAM )ppd, 0 );
	else 
		if ( ServiceExists(MS_POPUP_ADDPOPUP) )
			CallService( MS_POPUP_ADDPOPUP, ( WPARAM )ppd, 0 );

	free( ppd );
}

void ShowPopup( const char* line1, const char* line2, int timeout )
{
	if(CallService(MS_SYSTEM_TERMINATED, 0, 0)) return;

	if ( !ServiceExists( MS_POPUP_ADDPOPUP )) {	
		char title[256];
		sprintf(title, "%s Message", MODULE);

		if(line1 && line2) {
			char *message = new char[strlen(line1) + strlen(line2) + 2]; // newline and null terminator
			sprintf(message, "%s\n%s", line1, line2);
			MessageBox( NULL, message, title, MB_OK | MB_ICONINFORMATION );
			delete message;
		} else if(line1) {
			MessageBox( NULL, line1, title, MB_OK | MB_ICONINFORMATION );
		} else if(line2) {
			MessageBox( NULL, line2, title, MB_OK | MB_ICONINFORMATION );
		}
		return;
	}

	if(ServiceExists(MS_YAPP_CLASSINSTANCE)) {
		ShowClassPopup(popup_class_name, (char *)line1, (char *)line2);
	} else {
		POPUPDATAEX* ppd = ( POPUPDATAEX* )calloc( sizeof( POPUPDATAEX ), 1 );
		memset((void *)ppd, 0, sizeof(POPUPDATAEX));
	
		ppd->lchContact = NULL;
		ppd->lchIcon = hProtoIcon;
	
		if(line1 && line2) {
			strcpy( ppd->lpzContactName, line1 );
			strcpy( ppd->lpzText, line2 );
		} else if(line1) strcpy( ppd->lpzText, line1 );
		else if(line2) strcpy( ppd->lpzText, line2 );
	
		//ppd->colorBack = GetSysColor( COLOR_BTNFACE );
		//ppd->colorText = GetSysColor( COLOR_WINDOWTEXT );
		
		//ppd->colorText = 0x00FFFFFF;	// for old popup modules
		//ppd->colorBack = POPUP_USE_SKINNED_BG;
	
		ppd->iSeconds = timeout;
	
		//ppd->lpzClass = POPUP_CLASS_DEFAULT;
	
		ppd->PluginWindowProc = ( WNDPROC )NullWindowProc;
		ppd->PluginData = NULL;
	
		QueueUserAPC( sttMainThreadCallback , mainThread, ( ULONG )ppd );
	}
}

void ShowWarning(char *msg) {
	char buffer[512];
	ErrorDisplay disp = options.err_method;
	// funny logic :) ... try to avoid message boxes
	// if want baloons but no balloons, try popups
	// if want popups but no popups, try baloons
	// if, after that, you want balloons but no balloons, revert to message boxes
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_POP; 
	if(disp == ED_POP && !ServiceExists(MS_POPUP_SHOWMESSAGE)) disp = ED_BAL;
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_MB;

	sprintf(buffer, "%s Warning", MODULE);


	switch(disp) {
		case ED_POP:
			ShowPopup(buffer, msg, 0);
			break;
		case ED_MB:
			MessageBox(0, msg, buffer, MB_OK | MB_ICONWARNING);
			break;
		case ED_BAL:
			{
				MIRANDASYSTRAYNOTIFY sn = {0};
				sn.cbSize = sizeof(sn);
				sn.szProto = MODULE;
				sn.szInfoTitle = buffer;
				sn.szInfo = msg;
				sn.dwInfoFlags = NIIF_WARNING;
				sn.uTimeout = 10;

				CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&sn);
			}

			break;
	}
}

void ShowError(char *msg) {
	char buffer[512];
	ErrorDisplay disp = options.err_method;
	// funny logic :) ... try to avoid message boxes
	// if want baloons but no balloons, try popups
	// if want popups but no popups, try baloons
	// if, after that, you want balloons but no balloons, revert to message boxes
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_POP;
	if(disp == ED_POP && !ServiceExists(MS_POPUP_SHOWMESSAGE)) disp = ED_BAL;
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_MB;

	sprintf(buffer, "%s Error", MODULE);


	switch(disp) {
		case ED_POP:
			ShowPopup(buffer, msg, 0);
			break;
		case ED_MB:
			MessageBox(0, msg, buffer, MB_OK | MB_ICONWARNING);
			break;
		case ED_BAL:
			{
				MIRANDASYSTRAYNOTIFY sn = {0};
				sn.cbSize = sizeof(sn);
				sn.szProto = MODULE;
				sn.szInfoTitle = buffer;
				sn.szInfo = msg;
				sn.dwInfoFlags = NIIF_ERROR;
				sn.uTimeout = 10;

				CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&sn);
			}

			break;
	}
}

bool CreatePath(const char *path) {
	if(!path) return false;

	char folder[MAX_PATH];
	strcpy(folder, path);
	int i = 0;
	
	SetLastError(ERROR_SUCCESS);
	char *p = &folder[i];
	while(folder[i]) {
		p = strchr(p, _T('\\'));
		if(p) {
			i = p - folder;
			p++;

			if(folder[i]) folder[i] = 0;
		} else 
			i = strlen(folder);

		/*
		// this fails when the user does not have permission to create intermediate folders
		if(!CreateDirectory(folder, 0)) {
			DWORD err = GetLastError();
			if(err != ERROR_ALREADY_EXISTS) {
				//MessageBox(0, Translate("Could not create temporary folder."), Translate("Error"), MB_OK | MB_ICONERROR);
				//return 1;
				return false;
			}
		}
		*/
		CreateDirectory(folder, 0);

		folder[i] = path[i];
	}

	DWORD lerr = GetLastError();
	return (lerr == ERROR_SUCCESS || lerr == ERROR_ALREADY_EXISTS);
	
	/*
	// this function seems to be unavailable in shell32.dll under NT4
	int ret = SHCreateDirectoryEx(0, path, 0);
	return  (ret == ERROR_SUCCESS) || (ret == ERROR_FILE_EXISTS) || (ret == ERROR_ALREADY_EXISTS);
	*/
}


int ReloadIcons(WPARAM wParam, LPARAM lParam) {
	hProtoIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"otr_popups");
	hLockIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"otr_secure");
	hUnlockIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"otr_insecure");

	if(ServiceExists(MS_MSG_MODIFYICON)) {
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULE;
		sid.dwId = 0;
		sid.hIcon = hLockIcon;
		sid.hIconDisabled = hUnlockIcon;
		sid.flags = MBF_DISABLED;
		sid.szTooltip = Translate("OTR Encryption Status");
		CallService(MS_MSG_MODIFYICON, 0, (LPARAM)&sid);
	}
	
	FixMenuIcons();
	
	return 0;
}

void InitUtils() {
	if(ServiceExists(MS_SKIN2_ADDICON)) {
		SKINICONDESC sid = {0};

		sid.cbSize = sizeof(SKINICONDESC);
		sid.pszSection = "OTR";
		sid.hDefaultIcon = 0;

		sid.pszDescription = Translate("Popups Icon");
		sid.pszName = "otr_popups";
		sid.pszDefaultFile = "otr.dll";
		sid.iDefaultIndex = 0;
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_START), IMAGE_ICON, 0, 0, 0);
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Secure");
		sid.pszName = "otr_secure";
		sid.pszDefaultFile = "otr.dll";
		sid.iDefaultIndex = 1;
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_START), IMAGE_ICON, 0, 0, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		sid.pszDescription = Translate("Insecure");
		sid.pszName = "otr_insecure";
		sid.pszDefaultFile = "otr.dll";
		sid.iDefaultIndex = 2;
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, 0, 0, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);

		hProtoIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"otr_popups");
		hLockIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"otr_secure");
		hUnlockIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)"otr_insecure");

		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
	} else {		
		hProtoIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_START), IMAGE_ICON, 16, 16, 0);
		hLockIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_START), IMAGE_ICON, 16, 16, 0);
		hUnlockIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_STOP), IMAGE_ICON, 16, 16, 0);
	}

	if(ServiceExists(MS_YAPP_REGISTERCLASS)) {
		mir_snprintf(popup_class_name, 128, "%s/Notify", MODULE);

		PopupClass test = {0};
		test.cbSize = sizeof(test);
		test.hIcon = hProtoIcon;
		test.pszDescription = Translate("OTR");
		test.pszName = popup_class_name;
		test.windowProc = NullWindowProc;
		CallService(MS_YAPP_REGISTERCLASS, 0, (WPARAM)&test);
	}

}

void DeinitUtils() {
	if(hIcoLibIconsChanged) UnhookEvent(hIcoLibIconsChanged);
	else {
		DestroyIcon(hProtoIcon);
		DestroyIcon(hLockIcon);
		DestroyIcon(hUnlockIcon);
	}
}
