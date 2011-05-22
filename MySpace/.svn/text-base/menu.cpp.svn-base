#include "common.h"
#include "menu.h"
#include "resource.h"
#include "server_con.h"
#include "options.h"
#include "notifications.h"

HANDLE hMenuMain = 0, hServiceMenuMain = 0;

int MainMenuService(WPARAM wParam, LPARAM lParam) {
	CallService(MS_UTILS_OPENURL, (WPARAM)TRUE, (LPARAM)"http://home.myspace.com/index.cfm?fuseaction=user");

	return 0;
}

HANDLE hMenuMainAdd = 0, hServiceMenuMainAdd = 0;

int MainMenuServiceAdd(WPARAM wParam, LPARAM lParam) {
	CallService(MS_UTILS_OPENURL, (WPARAM)TRUE, (LPARAM)"http://www.myspace.com/index.cfm?fuseaction=im.friendslist&tab=1");

	return 0;
}

HANDLE hMenuContact = 0, hServiceMenuContact = 0, hEventMenuBuild = 0;

int ContactMenuService(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	int uid;
	if((uid = DBGetContactSettingDword(hContact, MODULE, "UID", 0)) != 0) {
		char buff[512];
		mir_snprintf(buff, 512, "http://www.myspace.com/%d", uid);
		CallService(MS_UTILS_OPENURL, (WPARAM)TRUE, (LPARAM)buff);
	}
	return 0;
}

void EnableMainAdd(bool enable) {
	CLISTMENUITEM menu = {0};
	menu.cbSize=sizeof(menu);
	menu.flags = CMIM_FLAGS | (enable ? 0 : CMIF_GRAYED);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuMainAdd, (LPARAM)&menu);
}

// this function is called when the contact's menu is about to be shown - you can e.g. 
// modify the menu here using the MS_CLIST_MODIFYMENUITEM service
int PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	// TODO: add code here that executes when the menu is constructed

	// e.g. modify menu item - see m_clist.h
	//CLISTMENUITEM menu = {0};
	//menu.cbSize=sizeof(menu);
	//menu.flags = CMIM_NAME | CMIF_TCHAR;
	//menu.ptszName = (char *)TranslateT("Changed menu text");
	//CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuContact, (LPARAM)&menu);

	return 0;
}

void InitMenu() {
	CLISTMENUITEM menu = {0};
	menu.cbSize=sizeof(menu);
	char buff[512];

	menu.flags = CMIM_ALL;
	menu.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MYSPACE), IMAGE_ICON, 0, 0, 0);

	hServiceMenuMain = CreateProtoServiceFunction(MODULE, "/MainMenu", MainMenuService);
	mir_snprintf(buff, 512, "%s%s", MODULE, "/MainMenu");
	menu.pszName = Translate("Home");
	menu.pszService = buff;
	menu.position = 2000060000;
	menu.popupPosition = 500099900;
	menu.pszPopupName = MODULE;
	hMenuMain = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);

	hServiceMenuMainAdd = CreateProtoServiceFunction(MODULE, "/MainMenuAdd", MainMenuServiceAdd);
	mir_snprintf(buff, 512, "%s%s", MODULE, "/MainMenuAdd");
	menu.pszName = Translate("Add friends");
	menu.pszService = buff;
	menu.position = 2000060001;
	menu.popupPosition = 0;
	menu.pszPopupName = MODULE;
	hMenuMainAdd = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);

	// contact menu item
	hServiceMenuContact = CreateProtoServiceFunction(MODULE, "/ContactMenu", ContactMenuService);
	menu.pszName = Translate("Show profile");
	mir_snprintf(buff, 512, "%s%s", MODULE, "/ContactMenu");
	menu.pszService = buff;
	menu.position = 0;
	menu.pszContactOwner = MODULE;
	//menu.flags |= CMIF_NOTOFFLINE; // only show for not-offline contacts
	hMenuContact = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

	hEventMenuBuild = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	EnableMainAdd(options.handle_links);
}

void DeinitMenu() {
	UnhookEvent(hEventMenuBuild);
	DestroyServiceFunction(hServiceMenuContact);
	DestroyServiceFunction(hServiceMenuMain);
	DestroyServiceFunction(hServiceMenuMainAdd);
}