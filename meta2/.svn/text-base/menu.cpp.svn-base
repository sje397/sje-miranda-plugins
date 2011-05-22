#include "common.h"
#include "menu.h"
#include "resource.h"
#include "icons.h"
#include "core_functions.h"
#include "api.h"
#include "select_meta.h"
#include "edit_meta.h"

HANDLE hServiceMenuOnOff = 0, hServiceMenuConvert = 0, hServiceMenuAdd = 0, hServiceMenuEdit = 0, 
	hServiceMenuDefault = 0, hServiceMenuRemove = 0, hServiceToggle = 0;
HANDLE hMenuOnOff = 0, hMenuConvert = 0, hMenuAdd = 0, hMenuEdit = 0, hMenuDefault = 0, hMenuRemove = 0;
POINT menuMousePoint;

INT_PTR ServiceMenuOnOff(WPARAM wParam, LPARAM lParam) {
	DBWriteContactSettingByte(0, MODULE, "Enabled", MetaEnabled() ? 0 : 1);
	Meta_Hide(!MetaEnabled());

	CLISTMENUITEM menu = {0};
	menu.cbSize=sizeof(menu);
	menu.flags = CMIM_NAME | CMIM_ICON | CMIF_TCHAR | CMIF_ICONFROMICOLIB;
	menu.ptszName = MetaEnabled() ? _T("Disable MetaContacts") : _T("Enable MetaContacts");
	menu.icolibItem = GetIcolibHandle(MetaEnabled() ? I_MENUOFF : I_MENU);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuOnOff, (LPARAM)&menu);

	return 0;
}

HANDLE hEventMenuBuild = 0;

INT_PTR ContactMenuConvert(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	Meta_Convert(hContact);
	return 0;
}

INT_PTR ContactMenuAdd(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	TCHAR buff[256];
	mir_sntprintf(buff, 256, TranslateT("Adding %s..."), ContactName(hContact));
	HANDLE hMeta = SelectMeta(buff);
	if(hMeta)
		Meta_Assign(hContact, hMeta);
	return 0;
}

INT_PTR ContactMenuRemove(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;

	Meta_Remove(hContact);
	return 0;
}

INT_PTR ContactMenuEdit(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;

	EditMeta(hContact);
	return 0;
}

INT_PTR ContactMenuDefault(WPARAM wParam, LPARAM lParam) {
	HANDLE hSub = (HANDLE)wParam,
		hMeta = (HANDLE)DBGetContactSettingDword(hSub, MODULE, "Handle", 0);
	
	MetaAPI_SetDefaultContact((WPARAM)hMeta, (LPARAM)hSub);
	return 0;
}

// show contact's context menu
VOID CALLBACK sttMenuFunc( ULONG_PTR param )
{
	HMENU hMenu;
	TPMPARAMS tpmp = {0};
	BOOL menuRet;
	HANDLE hSub = (HANDLE)param;

	hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)hSub, 0);

	tpmp.cbSize = sizeof(tpmp);

	menuRet = TrackPopupMenuEx(hMenu, TPM_RETURNCMD, menuMousePoint.x, menuMousePoint.y, (HWND)CallService(MS_CLUI_GETHWND, 0, 0), &tpmp);

	CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(menuRet), MPCF_CONTACTMENU), (LPARAM)hSub);

	DestroyMenu(hMenu);		
}

INT_PTR Meta_ContactMenuFunc(WPARAM wParam, LPARAM lParam) {
	HANDLE hMeta = (HANDLE)wParam;
	unsigned contact_num = (unsigned)lParam;
	if(metaMap.exists(hMeta) && contact_num >= 0 && contact_num < metaMap[hMeta].size()) {
		QueueUserAPC(sttMenuFunc, metaMainThread, (ULONG_PTR)metaMap[hMeta][contact_num].handle());
	}

	return 0;
}

HANDLE hMenuContact[MAX_SUBCONTACTS], hServiceContactMenu = 0;

int PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(CLISTMENUITEM);

	if(MetaEnabled()) {
		if(IsSubcontact(hContact)) {
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuRemove, (LPARAM)&mi);
		} else if(IsMetacontact(hContact)) {
			GetCursorPos(&menuMousePoint);
				
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuRemove, (LPARAM)&mi);

			if(!metaMap.exists(hContact)) {
				PUShowMessage("No such meta!", SM_WARNING);
				return 0;
			}

			// show subcontact menu items
			mi.flags = CMIM_FLAGS | CMIM_NAME | CMIM_ICON | CMIF_TCHAR;
			HIMAGELIST il = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST, 0, 0);
			SubcontactList::Iterator i = metaMap[hContact].start();
			int count = 0;
			TCHAR buff[31];
			while(i.has_val()) {
				// limit names to 30 chars
				mir_sntprintf(buff, 31, _T("%s"), ContactName(i.val().handle()));
				mi.ptszName = buff;
				mi.hIcon = ImageList_GetIcon(il, (int)CallService(MS_CLIST_GETCONTACTICON, (WPARAM)i.val().handle(), 0), 0);

				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuContact[count], (LPARAM)&mi);

				i.next();
				count++;
			}
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			for(;count < MAX_SUBCONTACTS; count++)
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuContact[count], (LPARAM)&mi);

			// show hide nudge menu item
// wParam = char *szProto
// lParam = BOOL show
#define MS_NUDGE_SHOWMENU	"NudgeShowMenu"
			{
				char serviceFunc[256];
				hContact = Meta_GetActive((HANDLE)wParam);
				mir_snprintf(serviceFunc, 256, "%s/SendNudge", ContactProto(hContact));
				CallService(MS_NUDGE_SHOWMENU, (WPARAM)MODULE, (LPARAM)ServiceExists(serviceFunc));
			}

		} else {
			mi.flags = CMIM_FLAGS;
			if(metaMap.size()) CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			if(!metaMap.size()) CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuRemove, (LPARAM)&mi);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&mi);
		}
	} else {
		mi.flags = CMIM_FLAGS | CMIF_HIDDEN;
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuAdd, (LPARAM)&mi);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuConvert, (LPARAM)&mi);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuDefault, (LPARAM)&mi);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuRemove, (LPARAM)&mi);
	}

	return 0;
}

void InitMenu() {
	CLISTMENUITEM menu = {0};
	menu.cbSize=sizeof(menu);

	menu.flags = CMIM_ALL | CMIF_TCHAR | CMIF_ICONFROMICOLIB;

	// main menu item
	hServiceMenuOnOff = CreateServiceFunction(MODULE "/MenuOnOff", ServiceMenuOnOff);
	menu.ptszName = MetaEnabled() ? _T("Disable MetaContacts") : _T("Enable MetaContacts");
	menu.pszService = MODULE "/MenuOnOff";
	menu.icolibItem = GetIcolibHandle(MetaEnabled() ? I_MENUOFF : I_MENU);
	hMenuOnOff = (HANDLE)CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);

	// normal and subcontact menu items
	hServiceMenuConvert = CreateServiceFunction(MODULE "/ContactMenuConvert", ContactMenuConvert);
	menu.ptszName = _T("Convert to MetaContact");
	menu.pszService = MODULE "/ContactMenuConvert";
	menu.icolibItem = GetIcolibHandle(I_CONVERT);
	hMenuConvert = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

	hServiceMenuAdd = CreateServiceFunction(MODULE "/ContactMenuAdd", ContactMenuAdd);
	menu.ptszName = _T("Add to existing MetaContact...");
	menu.pszService = MODULE "/ContactMenuAdd";
	menu.icolibItem = GetIcolibHandle(I_ADD);
	hMenuAdd = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

	hServiceMenuRemove = CreateServiceFunction(MODULE "/ContactMenuRemove", ContactMenuRemove);
	menu.ptszName = _T("Remove from MetaContact");
	menu.pszService = MODULE "/ContactMenuRemove";
	menu.icolibItem = GetIcolibHandle(I_REMOVE);
	hMenuRemove = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

	hServiceMenuDefault = CreateServiceFunction(MODULE "/ContactMenuDefault", ContactMenuDefault);
	menu.ptszName = _T("Set as MetaContact default");
	menu.pszService = MODULE "/ContactMenuDefault";
	menu.icolibItem = GetIcolibHandle(I_SETDEFAULT);
	hMenuDefault = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

	// hidden contact menu items...ho hum
	hServiceContactMenu = CreateServiceFunction("MetaContacts/MenuFunc",Meta_ContactMenuFunc);

	menu.flags = CMIM_ALL | CMIF_TCHAR | CMIF_HIDDEN;
	menu.pszContactOwner = MODULE;
	menu.hIcon = 0;
	menu.position = -99000;
	menu.hIcon = 0;

	TCHAR buff[256];
	menu.ptszName = buff;
	for(int i = 0; i < MAX_SUBCONTACTS; i++) {
		menu.position--;
		menu.popupPosition = i;
		mir_sntprintf(buff, 256, _T("Context%d"), i);

		menu.pszService= "MetaContacts/MenuFunc"; 
		
		hMenuContact[i] = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);
	}


	// metacontact items
	menu.flags = CMIM_ALL | CMIF_TCHAR | CMIF_ICONFROMICOLIB;
	hServiceMenuEdit = CreateServiceFunction(MODULE "/ContactMenuEdit", ContactMenuEdit);
	menu.ptszName = _T("Edit MetaContact");
	menu.pszService = MODULE "/ContactMenuEdit";
	menu.icolibItem = GetIcolibHandle(I_EDIT);
	menu.pszContactOwner = MODULE;
	hMenuEdit = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,0,(LPARAM)&menu);

	hEventMenuBuild = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	// for toptoolbar buttons (backward compatible)
	hServiceToggle = CreateServiceFunction("MetaContacts/OnOff", ServiceMenuOnOff);

}

void DeinitMenu() {
	UnhookEvent(hEventMenuBuild);
	DestroyServiceFunction(hServiceMenuRemove);
	DestroyServiceFunction(hServiceMenuDefault);
	DestroyServiceFunction(hServiceMenuEdit);
	DestroyServiceFunction(hServiceMenuAdd);
	DestroyServiceFunction(hServiceMenuConvert);
	DestroyServiceFunction(hServiceMenuOnOff);
	DestroyServiceFunction(hServiceContactMenu);
	DestroyServiceFunction(hServiceToggle);
}