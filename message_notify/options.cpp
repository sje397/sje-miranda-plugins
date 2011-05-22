#include "common.h"
#include "options.h"

Options options;

INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		{
			// initialise and fill listbox
			HWND hwndList = GetDlgItem(hwndDlg, IDC_LST_STATUS);
			ListView_DeleteAllItems(hwndList);

			SendMessage(hwndList,LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

			LVCOLUMN lvc = {0}; 
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.fmt = LVCFMT_LEFT;
	  
			lvc.iSubItem = 0;
			lvc.pszText = TranslateT("Status");	
			lvc.cx = 120;     // width of column in pixels
			ListView_InsertColumn(hwndList, 0, &lvc);


			LVITEM lvI = {0};

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items. 
			lvI.mask = LVIF_TEXT;

			char *strptr;
			wchar_t buff[256];
			for(int i = 0; i < 9; i++) {
				strptr = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)(ID_STATUS_ONLINE + i), (LPARAM)0);
				MultiByteToWideChar((int)CallService(MS_LANGPACK_GETCODEPAGE, 0, 0), 0, strptr, -1, buff, 256);
				lvI.pszText = buff;
				lvI.iItem = i;
				ListView_InsertItem(hwndList, &lvI);
				ListView_SetCheckState(hwndList, i, options.disable_status[i]);
			}
		}

		{
			CheckDlgButton(hwndDlg, IDC_CHK_TABS, options.consider_tabs ? TRUE : FALSE);
			HWND hwnd = GetDlgItem(hwndDlg, IDC_CHK_TABS);
			switch(options.notify_when) {
				case NOTIFY_CLOSED:
					CheckDlgButton(hwndDlg, IDC_RAD_CLOSED, TRUE);
					EnableWindow(hwnd, FALSE);
					break;
				case NOTIFY_NFORE:
					CheckDlgButton(hwndDlg, IDC_RAD_NFORE, TRUE);
					EnableWindow(hwnd, TRUE);
					break;
				case NOTIFY_ALWAYS:
					CheckDlgButton(hwndDlg, IDC_RAD_ALWAYS, TRUE);
					EnableWindow(hwnd, FALSE);
					break;
			};
		}

		{
			CheckDlgButton(hwndDlg, IDC_CHK_CLOSE, options.close_win ? TRUE : FALSE);
			CheckDlgButton(hwndDlg, IDC_CHK_MSG, options.show_msg ? TRUE : FALSE);

			CheckDlgButton(hwndDlg, IDC_CHK_SPEECH, (options.flags & MNNF_SPEAK) ? TRUE : FALSE);
			if(options.flags & MNNF_POPUP) {
				CheckDlgButton(hwndDlg, IDC_CHK_POPUPS, TRUE);
			} else {
				CheckDlgButton(hwndDlg, IDC_CHK_POPUPS, FALSE);
				HWND hwnd = GetDlgItem(hwndDlg, IDC_CHK_CLOSE);
				EnableWindow(hwnd, FALSE);
				hwnd = GetDlgItem(hwndDlg, IDC_CHK_MSG);
				EnableWindow(hwnd, FALSE);
			}

			if(!ServiceExists(MS_SPEAK_SAYEX)) {
				HWND hwnd = GetDlgItem(hwndDlg, IDC_CHK_SPEECH);
				EnableWindow(hwnd, FALSE);
			}
		}


		return FALSE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
				case IDC_RAD_CLOSED:
				case IDC_RAD_ALWAYS: 
					{
						HWND hwnd = GetDlgItem(hwndDlg, IDC_CHK_TABS);
						EnableWindow(hwnd, FALSE);
					}
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					return TRUE;
				case IDC_RAD_NFORE:
					{
						HWND hwnd = GetDlgItem(hwndDlg, IDC_CHK_TABS);
						EnableWindow(hwnd, TRUE);
					}
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					return TRUE;
				case IDC_CHK_MSG:
				case IDC_CHK_CLOSE:
				case IDC_CHK_TABS:
				case IDC_CHK_SPEECH:
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					return TRUE;
				case IDC_CHK_POPUPS:
					{
						bool checked = IsDlgButtonChecked(hwndDlg, IDC_CHK_POPUPS) ? true : false;
						HWND hwnd = GetDlgItem(hwndDlg, IDC_CHK_CLOSE);
						EnableWindow(hwnd, checked ? TRUE : FALSE);
						hwnd = GetDlgItem(hwndDlg, IDC_CHK_MSG);
						EnableWindow(hwnd, checked ? TRUE : FALSE);
					}
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					return TRUE;
			}
		}
		break;

	case WM_NOTIFY:
		if(IsWindowVisible(hwndDlg) && ((LPNMHDR) lParam)->hwndFrom == GetDlgItem(hwndDlg, IDC_LST_STATUS)) {
			switch (((LPNMHDR) lParam)->code) {
				
				case LVN_ITEMCHANGED:
					{
						NMLISTVIEW *nmlv = (NMLISTVIEW *)lParam;
						if((nmlv->uNewState ^ nmlv->uOldState) & LVIS_STATEIMAGEMASK) {
							SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
						}
					}
					break;
			}
		} else 
		if (((LPNMHDR)lParam)->code == (DWORD)PSN_APPLY ) {
			options.consider_tabs = IsDlgButtonChecked(hwndDlg, IDC_CHK_TABS) ? true : false;
			if(IsDlgButtonChecked(hwndDlg, IDC_RAD_CLOSED))
				options.notify_when = NOTIFY_CLOSED;
			else if(IsDlgButtonChecked(hwndDlg, IDC_RAD_NFORE))
				options.notify_when = NOTIFY_NFORE;
			else
				options.notify_when = NOTIFY_ALWAYS;
			options.close_win = IsDlgButtonChecked(hwndDlg, IDC_CHK_CLOSE) ? true : false;
			options.show_msg = IsDlgButtonChecked(hwndDlg, IDC_CHK_MSG) ? true : false;

			for(int i = 0; i < 9; i++)
				options.disable_status[i] = (ListView_GetCheckState(GetDlgItem(hwndDlg, IDC_LST_STATUS), i) ? true : false);

			options.flags = 0;
			options.flags |= (IsDlgButtonChecked(hwndDlg, IDC_CHK_POPUPS) ? MNNF_POPUP : 0);
			options.flags |= (IsDlgButtonChecked(hwndDlg, IDC_CHK_SPEECH) ? MNNF_SPEAK : 0);

			SaveOptions();

			if(options.notify_when == NOTIFY_NFORE)
				StartFocusTimer();
			else
				StopFocusTimer();
			return TRUE;
		}
		break;
	case WM_DESTROY:
		break;
	}
	return FALSE;

}


int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize						= sizeof(odp);
	odp.flags						= ODPF_BOLDGROUPS;
	odp.hInstance					= hInst;
	odp.pszGroup					= "Popups";
	odp.pszTitle					= MODULE;

	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pfnDlgProc					= DlgProcOpts;
	
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );
	return 0;
}

void LoadOptions() {
	options.notify_when = DBGetContactSettingByte(0, MODULE, "NotifyWhen", NOTIFY_NFORE);
	options.consider_tabs = (DBGetContactSettingByte(0, MODULE, "ConsiderTabs", 1) == 1);
	options.close_win = (DBGetContactSettingByte(0, MODULE, "CloseWin", 0) == 1);
	options.show_msg = (DBGetContactSettingByte(0, MODULE, "ShowMsg", 1) == 1);

	char buff[128];
	for(int i = 0; i < 9; i++) {
		sprintf(buff, "DisableStatus%d", i);
		options.disable_status[i] = (DBGetContactSettingByte(0, MODULE, buff, 0) == 1);
	}

	options.flags = (int)DBGetContactSettingDword(0, MODULE, "Flags", MNNF_POPUP);
}

void SaveOptions() {
	DBWriteContactSettingByte(0, MODULE, "NotifyWhen", options.notify_when);
	DBWriteContactSettingByte(0, MODULE, "ConsiderTabs", options.consider_tabs ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "CloseWin", options.close_win ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "ShowMsg", options.show_msg ? 1 : 0);
	char buff[128];
	for(int i = 0; i < 9; i++) {
		sprintf(buff, "DisableStatus%d", i);
		DBWriteContactSettingByte(0, MODULE, buff, options.disable_status[i] ? 1 : 0);
	}
	DBWriteContactSettingDword(0, MODULE, "Flags", (DWORD)options.flags);
}


