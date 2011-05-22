#include "commonheaders.h"

CLIST_INTERFACE cli = {0};

void ( *pfnOldAddContactToTree )( HWND hwnd, struct ClcData *dat, HANDLE hContact, int updateTotalCount, int checkHideOffline);

void fnAddContactToTree(HWND hwnd, struct ClcData *dat, HANDLE hContact, int updateTotalCount, int checkHideOffline)
{
	if(!DBGetContactSettingByte(0, "MetaContacts", "Enabled", 1) || !DBGetContactSettingByte(hContact, "MetaContacts", "IsSubcontact", 0))
		pfnOldAddContactToTree(hwnd, dat, hContact, updateTotalCount, checkHideOffline);
}


void fnRebuildEntireList(HWND hwnd, struct ClcData *dat)
{
	char *szProto;
	DWORD style = GetWindowLong(hwnd, GWL_STYLE);
	HANDLE hContact;
	struct ClcGroup *group;
	DBVARIANT dbv;

	dat->list.expanded = 1;
	dat->list.hideOffline = DBGetContactSettingByte(NULL, "CLC", "HideOfflineRoot", 0);
	dat->list.cl.count = dat->list.cl.limit = 0;
	dat->selection = -1;
	{
		int i;
		TCHAR *szGroupName;
		DWORD groupFlags;

		for (i = 1;; i++) {
			szGroupName = cli.pfnGetGroupName(i, &groupFlags);
			if (szGroupName == NULL)
				break;
			cli.pfnAddGroup(hwnd, dat, szGroupName, groupFlags, i, 0);
		}
	}

	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) {
		if ((style & CLS_SHOWHIDDEN || !DBGetContactSettingByte(hContact, "CList", "Hidden", 0))
			&& (!DBGetContactSettingByte(0, "MetaContacts", "Enabled", 1) || !DBGetContactSettingByte(hContact, "MetaContacts", "IsSubcontact", 0)))
		{
			if (DBGetContactSettingTString(hContact, "CList", "Group", &dbv))
				group = &dat->list;
			else {
				group = cli.pfnAddGroup(hwnd, dat, dbv.ptszVal, (DWORD) - 1, 0, 0);
				mir_free(dbv.ptszVal);
			}

			if (group != NULL) {
				group->totalMembers++;
				if (!(style & CLS_NOHIDEOFFLINE) && (style & CLS_HIDEOFFLINE || group->hideOffline)) {
					szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
					if (szProto == NULL) {
						if (!cli.pfnIsHiddenMode(dat, ID_STATUS_OFFLINE))
							cli.pfnAddContactToGroup(dat, group, hContact);
					}
					else if (!cli.pfnIsHiddenMode(dat, DBGetContactSettingWord(hContact, szProto, "Status", ID_STATUS_OFFLINE)))
						cli.pfnAddContactToGroup(dat, group, hContact);
				}
				else cli.pfnAddContactToGroup(dat, group, hContact);
			}
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}

	if (style & CLS_HIDEEMPTYGROUPS) {
		group = &dat->list;
		group->scanIndex = 0;
		for (;;) {
			if (group->scanIndex == group->cl.count) {
				group = group->parent;
				if (group == NULL)
					break;
			}
			else if (group->cl.items[group->scanIndex]->type == CLCIT_GROUP) {
				if (group->cl.items[group->scanIndex]->group->cl.count == 0) {
					group = cli.pfnRemoveItemFromGroup(hwnd, group, group->cl.items[group->scanIndex], 0);
				}
				else {
					group = group->cl.items[group->scanIndex]->group;
					group->scanIndex = 0;
				}
				continue;
			}
			group->scanIndex++;
		}
	}

	cli.pfnSortCLC(hwnd, dat, 0);
}

int SettingChanged(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;

	if(!hContact) {
		DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
		if(strcmp(cws->szModule, "MetaContacts") == 0 && strcmp(cws->szSetting, "Enabled") == 0)
			cli.pfnClcBroadcast( INTM_NAMEORDERCHANGED, 0, 0); // causes list rebuild

		return 0;
	}

	if(!DBGetContactSettingByte(0, "MetaContacts", "Enabled", 1)) return 0;

	{
		DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
		if(strcmp(cws->szSetting, "IsSubcontact") == 0)
			cli.pfnClcBroadcast( INTM_NAMEORDERCHANGED, 0, 0); // causes list rebuild
		return 0;
	}

	return 0;
}

void AddMetaMods(CLIST_INTERFACE *pcli) {
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SettingChanged);

	pfnOldAddContactToTree = pcli->pfnAddContactToTree;
	pcli->pfnAddContactToTree = fnAddContactToTree;

	pcli->pfnRebuildEntireList = fnRebuildEntireList;
	cli = *pcli; // this should be fixed
}

