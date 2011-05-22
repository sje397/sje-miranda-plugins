#include "common.h"
#include "priorities.h"
#include "resource.h"
#include <malloc.h>

/*
#define ID_STATUS_OFFLINE               40071		->8
#define ID_STATUS_ONLINE                40072		->0
#define ID_STATUS_AWAY                  40073		->4
#define ID_STATUS_DND                   40074		->7
#define ID_STATUS_NA                    40075		->6
#define ID_STATUS_OCCUPIED              40076		->5
#define ID_STATUS_FREECHAT              40077		->1
#define ID_STATUS_INVISIBLE             40078		->0
#define ID_STATUS_ONTHEPHONE            40079		->2
#define ID_STATUS_OUTTOLUNCH            40080		->3
*/

int status_order[10] = {8, 0, 4, 7, 6, 5, 1, 0, 2, 3};

int GetDefaultPrio(int status) {
	return status_order[status - ID_STATUS_OFFLINE];
}

typedef struct {
	int prio[10]; // priority for each status
	BOOL def[10]; // use default for this one?
} ProtoStatusPrio;

ProtoStatusPrio *priorities = 0;

int GetRealPriority(char *proto, int status) {
	char szSetting[256];
	if(!proto) {
		mir_snprintf(szSetting, 256, "DefaultPrio_%d", status);
		return DBGetContactSettingWord(0, MODULE, szSetting, GetDefaultPrio(status));
	} else {
		int prio;
		mir_snprintf(szSetting, 256, "ProtoPrio_%s%d", proto, status);
		prio = DBGetContactSettingWord(0, MODULE, szSetting, 0xFFFF);
		if(prio == 0xFFFF) {
			mir_snprintf(szSetting, 256, "DefaultPrio_%d", status);
			return DBGetContactSettingWord(0, MODULE, szSetting, GetDefaultPrio(status));
		} else
			return prio;
	}
	return 0xFFFF;
}

void ReadPriorities() {
	int num_protocols;
	PROTOCOLDESCRIPTOR **pppDesc;
	char szSetting[256];
	ProtoStatusPrio * current;
	int i, j;

	CallService(MS_PROTO_ENUMPROTOCOLS, (LPARAM)&num_protocols, (WPARAM)&pppDesc);

	current = priorities = (ProtoStatusPrio *)malloc((num_protocols + 1) * sizeof(ProtoStatusPrio));
	for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		mir_snprintf(szSetting, 256, "DefaultPrio_%d", i);
		current->def[i - ID_STATUS_OFFLINE] = TRUE;
		current->prio[i - ID_STATUS_OFFLINE] = DBGetContactSettingWord(0, MODULE, szSetting, GetDefaultPrio(i));
	}
	for(i = 0; i < num_protocols; i++) {
		current = priorities + (i + 1);
		for(j = ID_STATUS_OFFLINE; j <= ID_STATUS_OUTTOLUNCH; j++) {
			mir_snprintf(szSetting, 256, "ProtoPrio_%s%d", pppDesc[i]->szName, j);
			current->prio[j - ID_STATUS_OFFLINE] = DBGetContactSettingWord(0, MODULE, szSetting, 0xFFFF);
			current->def[j - ID_STATUS_OFFLINE] = (current->prio[j - ID_STATUS_OFFLINE] == 0xFFFF);
		}
	}
}

void WritePriorities() {
	int num_protocols;
	PROTOCOLDESCRIPTOR **pppDesc;
	char szSetting[256];
	ProtoStatusPrio * current = priorities;
	int i, j;

	CallService(MS_PROTO_ENUMPROTOCOLS, (LPARAM)&num_protocols, (WPARAM)&pppDesc);

	for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		mir_snprintf(szSetting, 256, "DefaultPrio_%d", i);
		if(current->prio[i - ID_STATUS_OFFLINE] != GetDefaultPrio(i))
			DBWriteContactSettingWord(0, MODULE, szSetting, current->prio[i - ID_STATUS_OFFLINE]);
		else
			DBDeleteContactSetting(0, MODULE, szSetting);
	}
	for(i = 0; i < num_protocols; i++) {
		current = priorities + (i + 1);
		for(j = ID_STATUS_OFFLINE; j <= ID_STATUS_OUTTOLUNCH; j++) {
			mir_snprintf(szSetting, 256, "ProtoPrio_%s%d", pppDesc[i]->szName, j);
			if(!current->def[j - ID_STATUS_OFFLINE])
				DBWriteContactSettingWord(0, MODULE, szSetting, current->prio[j - ID_STATUS_OFFLINE]);
			else
				DBDeleteContactSetting(0, MODULE, szSetting);
		}
	}
}

int GetIsDefault(int proto_index, int status) {
	return (priorities + (proto_index + 1))->def[status - ID_STATUS_OFFLINE];
}

BOOL GetPriority(int proto_index, int status) {
	ProtoStatusPrio * current;
	if(proto_index == -1) {
		current = priorities;
		return current->prio[status - ID_STATUS_OFFLINE];
	} else {
		current = priorities + (proto_index + 1);
		if(current->def[status - ID_STATUS_OFFLINE]) {
			current = priorities;
		}
		return current->prio[status - ID_STATUS_OFFLINE];
	}
	return 0xFFFF;
}

void SetPriority(int proto_index, int status, BOOL def, int prio) {
	ProtoStatusPrio * current;
	if(prio < 0) prio = 0;
	if(prio > 500) prio = 500;
	if(proto_index == -1) {
		current = priorities;
		current->prio[status - ID_STATUS_OFFLINE] = prio;
	} else {
		current = priorities + (proto_index + 1);
		current->def[status - ID_STATUS_OFFLINE] = def;
		if(!def) {
			current->prio[status - ID_STATUS_OFFLINE] = prio;
		}
	}
}

void ResetPriorities() {
	int num_protocols;
	PROTOCOLDESCRIPTOR **pppDesc;
	ProtoStatusPrio * current;
	int i, j;

	CallService(MS_PROTO_ENUMPROTOCOLS, (LPARAM)&num_protocols, (WPARAM)&pppDesc);

	current = priorities;
	for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
		current->def[i - ID_STATUS_OFFLINE] = TRUE;
		current->prio[i - ID_STATUS_OFFLINE] = GetDefaultPrio(i);
	}
	for(i = 0; i < num_protocols; i++) {
		current = priorities + (i + 1);
		for(j = ID_STATUS_OFFLINE; j <= ID_STATUS_OUTTOLUNCH; j++) {
			current->def[j - ID_STATUS_OFFLINE] = TRUE;
		}
	}
}

#define WMU_FILLSTATUSCMB		(WM_USER + 0x100)
#define WMU_FILLPRIODATA		(WM_USER + 0x101)

INT_PTR CALLBACK DlgProcOptsPriorities(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hw;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SendMessage(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 0));
		ReadPriorities();
		{
			int num_protocols;
			PROTOCOLDESCRIPTOR **pppDesc;
			int i, index;

			CallService(MS_PROTO_ENUMPROTOCOLS, (LPARAM)&num_protocols, (WPARAM)&pppDesc);
			hw = GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL);
			index = SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)TranslateT("<default>"));
			SendMessage(hw, CB_SETITEMDATA, (WPARAM)index, -1);
			for(i = 0; i < num_protocols; i++) {
				if(pppDesc[i]->type == PROTOTYPE_PROTOCOL) {
					if(strcmp(pppDesc[i]->szName, MODULE) != 0) {
						index = SendMessageA(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)pppDesc[i]->szName);
						SendMessage(hw, CB_SETITEMDATA, (WPARAM)index, i);
					}
				}
			}

			SendMessage(hw, CB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, WMU_FILLSTATUSCMB, 0, 0);
			SendMessage(hwndDlg, WMU_FILLPRIODATA, 0, 0);
		}
		return FALSE;
	case WMU_FILLPRIODATA:
		{
			int sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETCURSEL, 0, 0);
			if(sel != -1) {
				int index = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETITEMDATA, (WPARAM)sel, 0);
				sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETCURSEL, 0, 0);
				if(sel != -1) {
					int status = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETITEMDATA, (WPARAM)sel, 0);
					SetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, GetPriority(index, status), FALSE);
					if(index == -1) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PRIORITY), TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), TRUE);
						CheckDlgButton(hwndDlg, IDC_CHK_DEFAULT, TRUE);
						EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_DEFAULT), FALSE);
					} else {
						if(GetIsDefault(index, status)) {
							CheckDlgButton(hwndDlg, IDC_CHK_DEFAULT, TRUE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PRIORITY), FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), FALSE);
						} else {
							CheckDlgButton(hwndDlg, IDC_CHK_DEFAULT, FALSE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PRIORITY), TRUE);
							EnableWindow(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), TRUE);
						}
						
						EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_DEFAULT), TRUE);
					}
				}
			}
		}
		return TRUE;
	case WMU_FILLSTATUSCMB:
		{
			int sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETCURSEL, 0, 0);
			if(sel != -1) {
				int index = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETITEMDATA, (WPARAM)sel, 0);
				HWND hw = GetDlgItem(hwndDlg, IDC_CMB_STATUS);
				SendMessage(hw, CB_RESETCONTENT, 0, 0);
				if(index == -1) {
					int i;
					for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
						index = SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)(TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, i, GCMDF_TCHAR));
						SendMessage(hw, CB_SETITEMDATA, (WPARAM)index, i);
					}
				} else {
					int num_protocols, caps, i;
					PROTOCOLDESCRIPTOR **pppDesc;
					CallService(MS_PROTO_ENUMPROTOCOLS, (LPARAM)&num_protocols, (WPARAM)&pppDesc);

					caps = CallProtoService(pppDesc[index]->szName, PS_GETCAPS, PFLAGNUM_2, 0);

					for(i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
						if(caps & Proto_Status2Flag(i)) {
							index = SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)(TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, i, GCMDF_TCHAR));
							SendMessage(hw, CB_SETITEMDATA, (WPARAM)index, i);
						}
					}
				}
				SendMessage(hw, CB_SETCURSEL, 0, 0);
				SendMessage(hwndDlg, WMU_FILLPRIODATA, 0, 0);
			}
		}
		return TRUE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_CHK_DEFAULT:
				{
					int sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETCURSEL, 0, 0);
					if(sel != -1) {
						int index = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETITEMDATA, (WPARAM)sel, 0);
						sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETCURSEL, 0, 0);
						if(sel != -1) {
							BOOL checked = IsDlgButtonChecked(hwndDlg, IDC_CHK_DEFAULT);
							int status = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETITEMDATA, (WPARAM)sel, 0);
							if(checked) {
								SetPriority(index, status, TRUE, 0);
								SetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, GetPriority(index, status), FALSE);
							} else {
								SetPriority(index, status, FALSE, GetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, 0, FALSE));
							}
							EnableWindow(GetDlgItem(hwndDlg, IDC_ED_PRIORITY), !checked);
							EnableWindow(GetDlgItem(hwndDlg, IDC_SP_PRIORITY), !checked);
							SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
						}
					}
				}
				break;
			case IDC_BTN_RESET:
				ResetPriorities();
				SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_SETCURSEL, 0, 0);
				SendMessage(hwndDlg, WMU_FILLSTATUSCMB, 0, 0);
				SendMessage(hwndDlg, WMU_FILLPRIODATA, 0, 0);
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			}
		}
		if ( HIWORD( wParam ) == EN_CHANGE && LOWORD(wParam) == IDC_ED_PRIORITY && ( HWND )lParam == GetFocus()) {
			int sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETCURSEL, 0, 0);
			if(sel != -1) {
				int index = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_PROTOCOL), CB_GETITEMDATA, (WPARAM)sel, 0);
				sel = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETCURSEL, 0, 0);
				if(sel != -1) {
					int status = SendMessage(GetDlgItem(hwndDlg, IDC_CMB_STATUS), CB_GETITEMDATA, (WPARAM)sel, 0);
					int prio = GetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, 0, FALSE);
					SetPriority(index, status, FALSE, prio);
					if(prio != GetPriority(index, status))
						SetDlgItemInt(hwndDlg, IDC_ED_PRIORITY, GetPriority(index, status), FALSE);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}
			}
		}
		if ( HIWORD( wParam ) == CBN_SELCHANGE) {
			switch( LOWORD( wParam )) {
			case IDC_CMB_STATUS:
				SendMessage(hwndDlg, WMU_FILLPRIODATA, 0, 0);
				break;
			case IDC_CMB_PROTOCOL:
				SendMessage(hwndDlg, WMU_FILLSTATUSCMB, 0, 0);
				break;
			}
		}
		break;
						
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			WritePriorities();
			return TRUE;
		}
		break;
	case WM_DESTROY:
		free(priorities);
		priorities = 0;
		break;
	}

	return FALSE;
}

int PrioOptInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize						= sizeof(odp);
	odp.hInstance					= hInst;
	odp.flags						= ODPF_BOLDGROUPS;

	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_PRIORITIES);
	odp.pszTitle					= Translate("MetaContacts");
	odp.pszGroup					= Translate("Contact List");
	odp.pszTab						= Translate("Priorities");
	odp.pfnDlgProc					= DlgProcOptsPriorities;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

void InitPriorities() {
	HookEvent(ME_OPT_INITIALISE, PrioOptInit);
}

void DeinitPriorities() {
}
