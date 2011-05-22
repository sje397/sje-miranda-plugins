#include "common.h"
#include "icons.h"
#include "resource.h"
#include "menu.h"

HANDLE hIcoLibIconsChanged = NULL;


typedef struct {
	char* szDescr;
	char* szName;
	int   defIconID;
	HANDLE hIcolib;
} IconStruct;

static IconStruct iconList[] = {
	{ "Toggle Off",              "mc_off",      IDI_MCMENUOFF,     0},
	{ "Toggle On",               "mc_on",       IDI_MCMENU,        0},
	{ "Convert to MetaContact",  "mc_convert",  IDI_MCCONVERT,     0},
	{ "Add to Existing",         "mc_add",      IDI_MCADD,         0},
	{ "Edit",                    "mc_edit",     IDI_MCEDIT,        0},
	{ "Set to Default",          "mc_default",  IDI_MCSETDEFAULT,  0},
	{ "Remove",                  "mc_remove",   IDI_MCREMOVE,      0},
};


HICON LoadIconEx(IconIndex i) {
	HICON hIcon;

	hIcon = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)iconList[(int)i].szName);

	return hIcon;
}

HANDLE GetIcolibHandle(IconIndex i) {
	return iconList[i].hIcolib;
}

void ReleaseIconEx(HICON hIcon) {
	CallService(MS_SKIN2_RELEASEICON, (WPARAM)hIcon, 0);
}

int ReloadIcons(WPARAM wParam, LPARAM lParam) {
	// fix menu icons
	CLISTMENUITEM menu = {0};

	menu.cbSize = sizeof(menu);
	menu.flags = CMIM_ICON;

	menu.hIcon = LoadIconEx(MetaEnabled() ? I_MENUOFF : I_MENU);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuOnOff, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);

	menu.hIcon = LoadIconEx(I_CONVERT);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);
	
	menu.hIcon = LoadIconEx(I_ADD);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);
	
	menu.hIcon = LoadIconEx(I_EDIT);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuEdit, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);

	menu.hIcon = LoadIconEx(I_SETDEFAULT);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);

	menu.hIcon = LoadIconEx(I_REMOVE);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuRemove, (LPARAM)&menu);
	ReleaseIconEx(menu.hIcon);

	return 0;
}

void InitIcons()  {
	SKINICONDESC sid = {0};
	char path[MAX_PATH];
	int i;
	
	sid.cbSize = sizeof(SKINICONDESC);
	sid.pszSection = MODULE;
	sid.pszDefaultFile = path;
	GetModuleFileNameA(hInst, path, sizeof(path));

	for (i = 0; i < sizeof(iconList) / sizeof(IconStruct); ++i)
	{
		sid.pszDescription = Translate(iconList[i].szDescr);
		sid.pszName = iconList[i].szName;
		sid.iDefaultIndex = -iconList[i].defIconID;
		iconList[i].hIcolib = (HANDLE)CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);
	}

	//hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);

	//ReloadIcons(0, 0); 
}

void DeinitIcons() {
	UnhookEvent(hIcoLibIconsChanged);
}
