#include "common.h"
#include "srmm_icon.h"
#include "resource.h"
#include "notifications.h"
#include "server_con.h"

#include <ctime>

int WindowEvent(WPARAM wParam, LPARAM lParam) {
	MessageWindowEventData *wd = (MessageWindowEventData *)lParam;
	if(wd->uType != MSG_WINDOW_EVT_OPEN) return 0;

	HANDLE hContact = wd->hContact, hSub = ServiceExists(MS_MC_GETMOSTONLINECONTACT) ? (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0) : 0;

	bool hide = false;
	if((hSub == 0 && !CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)MODULE)) || (hSub && !CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hSub, (LPARAM)MODULE)))
		hide = true;

	if(hSub || !hide) { // avoid writing db param for non-myspace and non-meta contacts
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULE;
		sid.flags = (hide ? MBF_HIDDEN : 0);
		CallService(MS_MSG_MODIFYICON, (WPARAM)wd->hContact, (LPARAM)&sid);
	}

	return 0;
}

int IconPressed(WPARAM wParam, LPARAM lParam) {
	StatusIconClickData *cd = (StatusIconClickData *)lParam;
	if(strcmp(cd->szModule, MODULE) != 0) return 0;

	HANDLE hContact = (HANDLE)wParam, hSub;
	if(ServiceExists(MS_MC_GETMOSTONLINECONTACT) && (hSub = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0)) != 0)
		hContact = hSub;

	if(!CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)MODULE))
		return 0;

	HMENU hMenu = CreatePopupMenu();
	for(int i = 0; zap_array[i]; i++) {
		AppendMenuA(hMenu,MF_STRING,i+1, Translate(zap_array[i]));
	}	
	BOOL cmd = TrackPopupMenu(hMenu, TPM_NONOTIFY | TPM_RETURNCMD, cd->clickLocation.x, cd->clickLocation.y, 0, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), 0);
	DestroyMenu(hMenu);

	if(cmd) {
		int zap_num = cmd - 1;
		NotifyZapSend(hContact, zap_num);
	}
	return 0;
}

int ContactAdded(WPARAM wParam, LPARAM lParam) {

	return 0;
}

HICON hZapIcon = 0;
HANDLE hEventIconPressed = 0, hEventWindow = 0, hEventContactAdded;
void InitSRMMIcon() {
	// add icon to srmm status icons
	if(ServiceExists(MS_MSG_ADDICON)) {
		hZapIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_ZAPS), IMAGE_ICON, 0, 0, 0);//LoadIcon(hInst, MAKEINTRESOURCE(IDI_ZAPS));
		// create icon hidden
		StatusIconData sid = {0};
		sid.cbSize = sizeof(sid);
		sid.szModule = MODULE;
		sid.hIcon = hZapIcon;
		sid.flags = MBF_HIDDEN;
		sid.szTooltip = Translate("Send ZAP");
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
		
		// hook the window events so that we can can change the status of the icon for myspace contacts, new contacts, and metacontacts if necessary
		hEventWindow = HookEvent(ME_MSG_WINDOWEVENT, WindowEvent);
		hEventIconPressed = HookEvent(ME_MSG_ICONPRESSED, IconPressed);

	}

	hEventContactAdded = HookEvent(ME_DB_CONTACT_ADDED, ContactAdded);
}

void DeinitSRMMIcon() {
	if(hEventContactAdded) UnhookEvent(hEventContactAdded);
	if(hZapIcon) DestroyIcon(hZapIcon);
	if(hEventIconPressed) UnhookEvent(hEventIconPressed);
	if(hEventWindow) UnhookEvent(hEventWindow);
}
