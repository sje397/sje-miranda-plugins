#include "common.h"
#include "edit_meta.h"
#include "resource.h"
#include "core_functions.h"
#include "icons.h"
#include "api.h"

TCHAR *GetIdString(HANDLE hContact, char *proto) {
	if(proto) {
		char *field = (char *)CallProtoService(proto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);
		DBVARIANT dbv;

		if(!DBGetContactSetting(hContact,proto,field,&dbv)) {
			TCHAR *buff = 0;
			switch(dbv.type)
			{
				case DBVT_ASCIIZ:
					return mir_a2t(dbv.pszVal);
				case DBVT_UTF8:
#ifdef _UNICODE
					return mir_utf8decodeW(dbv.pszVal);
#else
					return mir_utf8decode(dbv.pszVal, 0);
#endif
				case DBVT_WCHAR:
					return mir_u2t(dbv.pwszVal);
				case DBVT_BYTE:
					buff = (TCHAR *)mir_alloc(4 * sizeof(TCHAR));
					mir_sntprintf(buff, 4, _T("%d"),dbv.bVal);
					return buff;
				case DBVT_WORD:
					buff = (TCHAR *)mir_alloc(16 * sizeof(TCHAR));
					mir_sntprintf(buff, 16, _T("%d"),dbv.wVal);
					return buff;
				case DBVT_DWORD:
					buff = (TCHAR *)mir_alloc(32 * sizeof(TCHAR));
					mir_sntprintf(buff, 32, _T("%d"),dbv.dVal);
					return buff;
				default:
					return 0;
			}
			DBFreeVariant(&dbv);
		}
	}
	return 0;
}

HANDLE hMeta;
int def;
SubcontactList subs;
bool changed;

void FillList(HWND hwndLst) {
	LVITEM LvItem = {0};
	LvItem.mask=LVIF_TEXT;   // Text Style
	LvItem.cchTextMax = 256; // Max size of test

	int row = 0;
	char *proto;
	TCHAR *proto_t;

	SendMessage(hwndLst, LVM_DELETEALLITEMS, 0, 0);

	for(SubcontactList::Iterator i = subs.start(); i.has_val(); i.next()) {
		LvItem.iItem = row;

		LvItem.iSubItem = 0; // clist display name
		LvItem.pszText = ContactName(i.val().handle());
		SendMessage(hwndLst, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&LvItem);
		
		proto = ContactProto(i.val().handle());

		LvItem.iSubItem = 1; // id
		LvItem.pszText = GetIdString(i.val().handle(), proto);
		SendMessage(hwndLst, LVM_SETITEM, (WPARAM)0, (LPARAM)&LvItem);
		mir_free(LvItem.pszText);

#ifdef _UNICODE
		proto_t = mir_a2u(proto);
#else
		proto_t = proto;
#endif
		LvItem.iSubItem = 2; // protocol
		LvItem.pszText = proto_t;
		SendMessage(hwndLst, LVM_SETITEM, (WPARAM)0, (LPARAM)&LvItem);
#ifdef _UNICODE
		mir_free(proto_t);
#endif;

		LvItem.iSubItem = 3; // default
		LvItem.pszText = (row == def ? TranslateT("TRUE") : TranslateT("FALSE"));
		SendMessage(hwndLst, LVM_SETITEM, (WPARAM)0, (LPARAM)&LvItem);

		row++;
	}
}

void SetListSelection(HWND hList, int sel) {
	LVITEM LvItem;

	ZeroMemory(&LvItem, sizeof(LvItem));
	LvItem.iItem = sel;
	LvItem.mask = LVIF_STATE;
	LvItem.stateMask = LVIS_SELECTED|LVIS_FOCUSED;
	LvItem.state = LVIS_SELECTED|LVIS_FOCUSED;

	SendMessage(hList, LVM_SETITEMSTATE, (WPARAM)sel, (LPARAM)&LvItem);

}

void ApplyChanges() {
	if(def >= 0 && def < (int)metaMap[hMeta].size())
		MetaAPI_SetDefaultContactNum((WPARAM)hMeta, (LPARAM)def);
	else
		MetaAPI_SetDefaultContactNum((WPARAM)hMeta, (LPARAM)0);

	unsigned long index;
	for(SubcontactList::Iterator i = metaMap[hMeta].start(); i.has_val(); i.next()) {
		if(!subs.get_index(i.val(), index)) {
			Meta_Remove(i.val().handle());
		}
	}
}

#define WMU_SET_BUTTONS			(WM_USER + 0x100)
INT_PTR CALLBACK Meta_EditDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch(msg) {
		case WM_INITDIALOG:
			TranslateDialogDefault( hwndDlg );
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(I_EDIT));

			SetDlgItemText(hwndDlg, IDC_ED_NAME, ContactName(hMeta));

			{
				HWND hwndLst = GetDlgItem(hwndDlg, IDC_LST_CONTACTS);
				SendMessage(hwndLst,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT); // Set style

				LVCOLUMN LvCol = {0};
				LvCol.mask=LVCF_TEXT|LVCF_WIDTH|LVCF_SUBITEM;

				LvCol.pszText=TranslateT("Contact");
				LvCol.cx=100;
				SendMessage(hwndLst,LVM_INSERTCOLUMN,0,(LPARAM)&LvCol);

				LvCol.pszText=TranslateT("Id");
				LvCol.cx=130;
				SendMessage(hwndLst,LVM_INSERTCOLUMN,1,(LPARAM)&LvCol);
				LvCol.pszText=TranslateT("Protocol");
				LvCol.cx=100;
				SendMessage(hwndLst,LVM_INSERTCOLUMN,2,(LPARAM)&LvCol);
				LvCol.pszText=TranslateT("Default");
				LvCol.cx=60;
				SendMessage(hwndLst,LVM_INSERTCOLUMN,3,(LPARAM)&LvCol);

				FillList(hwndLst);
				SendMessage(hwndDlg, WMU_SET_BUTTONS, 0, 0);
			}

			break;
		case WMU_SET_BUTTONS:
			{
				HWND hwnd = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hwnd, changed);
				hwnd = GetDlgItem(hwndDlg, IDC_VALIDATE);
				EnableWindow(hwnd, changed);

				int index = SendDlgItemMessage(hwndDlg, IDC_LST_CONTACTS, LVM_GETNEXTITEM,-1,LVNI_FOCUSED|LVNI_SELECTED); // return item selected
				hwnd = GetDlgItem(hwndDlg, IDC_BTN_SETDEFAULT);
				EnableWindow(hwnd, index != -1 && index != def);
				hwnd = GetDlgItem(hwndDlg, IDC_BTN_REM);
				EnableWindow(hwnd, index != -1);
 
			}
			return TRUE;
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED) {
				switch(LOWORD(wParam))
				{
					case IDC_BTN_SETDEFAULT:
						{
							int index = SendDlgItemMessage(hwndDlg, IDC_LST_CONTACTS, LVM_GETNEXTITEM,-1,LVNI_FOCUSED|LVNI_SELECTED); // return item selected
							def = index;
							changed = true;
							FillList(GetDlgItem(hwndDlg, IDC_LST_CONTACTS));
							SetListSelection(GetDlgItem(hwndDlg, IDC_LST_CONTACTS), index);
							SendMessage(hwndDlg, WMU_SET_BUTTONS, 0, 0);
						}
						break;
					case IDC_BTN_REM:
						{
							int index = SendDlgItemMessage(hwndDlg, IDC_LST_CONTACTS, LVM_GETNEXTITEM,-1,LVNI_FOCUSED|LVNI_SELECTED); // return item selected
							if(index >= 0 && index < (int)subs.size()) {
								subs.remove(subs[index].handle());
								if(def >= index && def > 0) def--;
								changed = true;
								FillList(GetDlgItem(hwndDlg, IDC_LST_CONTACTS));
								SendMessage(hwndDlg, WMU_SET_BUTTONS, 0, 0);
							}
						}
						break;
					case IDOK:
						EndDialog(hwndDlg, IDOK);
						break;
					case IDCANCEL:
						EndDialog(hwndDlg, IDCANCEL);
						break;
					case IDC_VALIDATE:
						ApplyChanges();
						break;
				}
				return TRUE;
			}
			break;
		case WM_NOTIFY: // the message that is being sent always
			switch(LOWORD(wParam)) // hit control
			{
				case IDC_LST_CONTACTS:      // did we hit our ListView contorl?
					if(((LPNMHDR)lParam)->code == NM_CLICK) {
						SendMessage(hwndDlg, WMU_SET_BUTTONS, 0, 0);
					}
					break;
			}
			break;

	}
	return FALSE;
}

void EditMeta(HANDLE hM) {
	hMeta = hM;
	subs.add_all(metaMap[hMeta]);
	def = (int)DBGetContactSettingByte(hMeta, MODULE, "Default", -1);
	changed = false;

	HWND clui = (HWND)CallService(MS_CLUI_GETHWND,0,0);
	if(DialogBox(hInst, MAKEINTRESOURCE(IDD_METAEDIT), clui, Meta_EditDialogProc) == IDOK) {

		// apply changes

		ApplyChanges();
	}
	subs.clear();
}