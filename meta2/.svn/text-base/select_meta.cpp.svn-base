#include "common.h"
#include "select_meta.h"
#include "resource.h"
#include "icons.h"


void FillList(HWND list, bool sort)
{
	SendMessage(list, LB_RESETCONTENT, 0, 0);
	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	int count = 0;
	while(hContact) {
		if(!IsMetacontact(hContact)) {
			// This isn't a MetaContact, go to the next
			hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
			continue;
		}

		// get contact display name from clist
		TCHAR *szCDN = ContactName(hContact);
		int index;
		if(sort) {
			int j;
			TCHAR buff[1024];
			for(j = 0; j < count; j++) {
				SendMessage(list, LB_GETTEXT, j, (LPARAM)buff);
				if(_tcscmp(buff, szCDN) > 0) break;
			}
			index = SendMessage(list, LB_INSERTSTRING, (WPARAM)j, (LPARAM)szCDN);
		} else 
			index = SendMessage(list, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)szCDN);
		SendMessage(list, LB_SETITEMDATA, (WPARAM)index, (LPARAM)hContact);
		count++;

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0);
	}
}

INT_PTR CALLBACK Meta_SelectDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_INITDIALOG:
		{	
			TranslateDialogDefault( hwndDlg );
			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx(I_ADD));
			SetWindowText(hwndDlg, (TCHAR *)lParam);
			FillList(GetDlgItem(hwndDlg, IDC_METALIST), false);
		}
		case WM_COMMAND:
			if(HIWORD(wParam) == BN_CLICKED) {
				switch(LOWORD(wParam))
				{
					case IDOK:
					{
						HANDLE hContact = (HANDLE)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
						int item = SendMessage(GetDlgItem(hwndDlg, IDC_METALIST),LB_GETCURSEL, 0, 0);
						HANDLE hMeta = (HANDLE)SendMessage(GetDlgItem(hwndDlg, IDC_METALIST), LB_GETITEMDATA, (WPARAM)item, 0);

						EndDialog(hwndDlg, (INT_PTR)hMeta);
						break;
					}
					case IDCANCEL:
						EndDialog(hwndDlg, 0);
						break;

					case IDC_CHK_SRT:
						FillList(GetDlgItem(hwndDlg,IDC_METALIST), IsDlgButtonChecked(hwndDlg, IDC_CHK_SRT) != 0);
						EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
						break;
				}
				return TRUE;
			}
			if(LOWORD(wParam) == IDC_METALIST && HIWORD(wParam) == LBN_SELCHANGE) {
				int item = SendMessage(GetDlgItem(hwndDlg, IDC_METALIST),LB_GETCURSEL, 0, 0);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), item != -1);
			}
			break;
		case WM_DESTROY:
		{	// Free all allocated memory and return the focus to the CList
			HWND clist = GetParent(hwndDlg);
			ReleaseIconEx((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
			EndDialog(hwndDlg,TRUE);
			SetFocus(clist);
			return TRUE;
		}
	}
	return FALSE;	// All other Message are not handled
}


HANDLE SelectMeta(TCHAR *msg) {
	HWND clui = (HWND)CallService(MS_CLUI_GETHWND,0,0);
	return (HANDLE)DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_METASELECT), clui, Meta_SelectDialogProc, (WPARAM)msg);
}