#include "common.h"
#include "menu.h"
#include "options.h"
#include "utils.h"

HANDLE hMenuBuildEvent, hStartItem, hStopItem;

void FixMenuIcons() {
	// fix menu icons
	CLISTMENUITEM menu = {0};
	menu.cbSize=sizeof(menu);
	menu.flags = CMIM_ICON;

	menu.hIcon = hLockIcon;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStartItem, (LPARAM)&menu);
	menu.hIcon = hUnlockIcon;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStopItem, (LPARAM)&menu);
}

int PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;
	
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE;
	
	char *proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	char *uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
	WORD pol = CONTACT_DEFAULT_POLICY;
	
	if(!proto || !uname || DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) == 1) {
		goto hide_all;
	}
	
	if(proto && metaproto && strcmp(proto, metaproto) == 0) {
		// make menu act as per most online subcontact
		hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM)hContact, 0);
		if(!hContact)
			goto hide_all;
		uname = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
		proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	}

	pol = DBGetContactSettingWord(hContact, MODULE, "Policy", CONTACT_DEFAULT_POLICY);
	if(pol == CONTACT_DEFAULT_POLICY) pol = options.default_policy;
	
	if(pol == OTRL_POLICY_NEVER || pol == OTRL_POLICY_ALWAYS) {
		goto hide_all;
	}

	{
		ConnContext *context = otrl_context_find(otr_user_state, uname, MODULE, proto, FALSE, 0, 0, 0);	
		bool encrypted = context && context->msgstate != OTRL_MSGSTATE_PLAINTEXT;
		if(encrypted) {
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStopItem, (LPARAM)&mi);
			mi.flags |= CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStartItem, (LPARAM)&mi);
		} else {
			if(pol == OTRL_POLICY_MANUAL_MOD) {
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStartItem, (LPARAM)&mi);
				mi.flags |= CMIF_HIDDEN;
				CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStopItem, (LPARAM)&mi);
			} else { // should only be 'opportunistic'
				goto hide_all;
			}
		}	
	}
	
	return 0;

hide_all:
	mi.flags |= CMIF_HIDDEN;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStopItem, (LPARAM)&mi);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hStartItem, (LPARAM)&mi);
	return 0;
}

void InitMenu() {
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_NOTOFFLINE;
	mi.position = -400000;

	mi.pszName = Translate("Stop OTR");
	mi.pszService = MS_OTR_MENUSTOP;
	mi.hIcon = hUnlockIcon;

	hStopItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	mi.pszName = Translate("Start OTR");
	mi.pszService = MS_OTR_MENUSTART;
	mi.hIcon = hLockIcon;

	hStartItem = (HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	hMenuBuildEvent = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

}

void DeinitMenu() {
}
