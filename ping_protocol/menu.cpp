#include "stdafx.h"
#include "menu.h"

HANDLE hMenuDisable, hMenuGraph, hMenuEdit;
HANDLE hEventMenuBuild;

int MenuBuild(WPARAM wParam, LPARAM lParam) {
	CLISTMENUITEM menu;
	ZeroMemory(&menu,sizeof(menu));
	menu.cbSize=sizeof(menu);

	menu.flags = CMIM_NAME | CMIM_ICON;
	bool disable = DBGetContactSettingWord((HANDLE)wParam, PROTO, "Status", ID_STATUS_OFFLINE) != options.off_status;
	if(disable) {
		menu.hIcon = LoadSkinnedProtoIcon(PROTO, options.off_status);
		menu.pszName = Translate("Disable");
	} else {
		menu.hIcon = LoadSkinnedProtoIcon(PROTO, options.rstatus);
		menu.pszName = Translate("Enable");
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDisable, (LPARAM)&menu);

	// hide graph menu item if window displayed
	//menu.flags = CMIM_FLAGS | (DBGetContactSettingDword((HANDLE)wParam, PROTO, "WindowHandle", 0) == 0 ? 0 : CMIF_HIDDEN);
	//CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuGraph, (LPARAM)&menu);

	return 0;
}

void InitMenus() {

	CLISTMENUITEM menu;
	ZeroMemory(&menu,sizeof(menu));
	menu.cbSize=sizeof(menu);

	// main menu
	menu.flags = 0;
	menu.popupPosition = 500099900;
	menu.pszPopupName = Translate( "PING" );
	menu.cbSize = sizeof( menu );
	menu.position = 2000060000;
	menu.hIcon = LoadSkinnedProtoIcon(PROTO, options.rstatus);
	menu.pszName = Translate( "Enable All Pings" );
	menu.pszService = PROTO "/EnableAll";
	CallService( MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu );

	menu.flags = 0;
	menu.popupPosition = 500299901;
	menu.pszPopupName = Translate( "PING" );
	menu.cbSize = sizeof( menu );
	menu.position = 2000060001;
	menu.popupPosition = 0;
	menu.hIcon = LoadSkinnedProtoIcon(PROTO, options.off_status);
	menu.pszName = Translate( "Disable All Pings" );
	menu.pszService = PROTO "/DisableAll";
	CallService( MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu );

	// contact
	menu.flags = 0;
	menu.popupPosition = 0;
	menu.pszPopupName = 0;
	menu.cbSize = sizeof( menu );
	menu.position =-300100;
	//menu.popupPosition = 0;
	menu.hIcon = LoadSkinnedProtoIcon(PROTO, options.off_status);
	menu.pszName = Translate( "Disable" );
	menu.pszService = PROTO "/ToggleEnabled";
	menu.pszContactOwner = PROTO;
	hMenuDisable = (HANDLE)CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&menu );

	menu.flags = 0;
	menu.popupPosition = 0;
	menu.pszPopupName = 0;
	menu.cbSize = sizeof( menu );
	menu.position =-300090;
	//menu.popupPosition = 0;
	menu.hIcon = LoadSkinnedProtoIcon(PROTO, options.rstatus);
	menu.pszName = Translate( "Graph" );
	menu.pszService = PROTO "/ShowGraph";
	menu.pszContactOwner = PROTO;
	hMenuGraph = (HANDLE)CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&menu );

	menu.flags = 0;
	menu.popupPosition = 0;
	menu.pszPopupName = 0;
	menu.cbSize = sizeof( menu );
	menu.position =-300080;
	//menu.popupPosition = 0;
	menu.hIcon = LoadSkinnedProtoIcon(PROTO, options.rstatus);
	menu.pszName = Translate( "Edit..." );
	menu.pszService = PROTO "/Edit";
	menu.pszContactOwner = PROTO;
	hMenuGraph = (HANDLE)CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&menu );

	hEventMenuBuild = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, MenuBuild);
}
