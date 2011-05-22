#include "common.h"
#include "menu.h"
#include "resource.h"
[!if MAIN_MENU]

HANDLE hMenuMain = 0, hServiceMenuMain = 0;

int MainMenuService(WPARAM wParam, LPARAM lParam) {
	// TODO: add code here that executes when the menu item is chosen

	// e.g. modify main menu item - see m_clist.h
	//CLISTMENUITEM menu = {0};
	//menu.cbSize=sizeof(menu);
	//menu.flags = CMIM_NAME | CMIF_TCHAR;
	//menu.ptszName = (char *)TranslateT("Changed menu text");
	//CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuMain, (LPARAM)&menu);

	return 0;
}
[!endif]
[!if CONTACT_MENU]

HANDLE hMenuContact = 0, hServiceMenuContact = 0, hEventMenuBuild = 0;

int ContactMenuService(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	// TODO: add code here that executes when the menu item is chosen

	// e.g. you could modify the menu item here - see below
	return 0;
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
[!endif]

void InitMenu() {
	CLISTMENUITEM menu = {0};
	menu.cbSize=sizeof(menu);

	menu.flags = CMIM_ALL | CMIF_TCHAR;
	menu.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_MENU), IMAGE_ICON, 0, 0, 0);
[!if MAIN_MENU]

	// main menu item
	hServiceMenuMain = CreateServiceFunction(MODULE "/MainMenu", MainMenuService);
	menu.ptszName = TranslateT(MODULE);
	menu.pszService = MODULE "/MainMenu";
	menu.position = 0;
	hMenuMain = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);
[!endif]
[!if CONTACT_MENU]

	// contact menu item
	hServiceMenuContact = CreateServiceFunction(MODULE "/ContactMenu", ContactMenuService);
	menu.ptszName = TranslateT(MODULE);
	menu.pszService = MODULE "/ContactMenu";
	menu.position = 0;
	menu.flags = CMIF_NOTOFFLINE; // only show for not-offline contacts
	hMenuContact = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

	hEventMenuBuild = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
[!endif]
}

void DeinitMenu() {
[!if CONTACT_MENU]
	UnhookEvent(hEventMenuBuild);
	DestroyServiceFunction(hServiceMenuContact);
[!endif]
[!if MAIN_MENU]
	DestroyServiceFunction(hServiceMenuMain);
[!endif]
}