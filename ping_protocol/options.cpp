#include "stdafx.h"
#include "options.h"

PingOptions options;

// main ping options 
static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hw;
	OPENFILENAME ofn = {0};
	//char *strptr;

	switch ( msg ) {
	case WM_INITDIALOG: {
		TranslateDialogDefault( hwndDlg );

		SetDlgItemInt(hwndDlg, IDC_PPM, options.ping_period, FALSE);
		SetDlgItemInt(hwndDlg, IDC_PT, options.ping_timeout, FALSE);
		CheckDlgButton(hwndDlg, IDC_CHECKPOPUP, options.show_popup);
		CheckDlgButton(hwndDlg, IDC_CHECKPOPUP2, options.show_popup2);
		CheckDlgButton(hwndDlg, IDC_CHK_BLOCK, options.block_reps);
		CheckDlgButton(hwndDlg, IDC_CHK_LOG, options.logging);
		CheckDlgButton(hwndDlg, IDC_CHK_NOTESTSTATUS, options.no_test_status);
		CheckDlgButton(hwndDlg, IDC_CHK_HIDEPROTO, options.hide_proto);
		CheckDlgButton(hwndDlg, IDC_CHK_USESTATUSMSG, options.use_status_msg);
		hw = GetDlgItem(hwndDlg, IDC_TSTATUS);
		EnableWindow(hw, options.no_test_status? FALSE : TRUE);
		/*
		for(int i = ID_STATUS_OFFLINE; i != ID_STATUS_OUTTOLUNCH; i++) {
			strptr = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)i, (LPARAM)0);
			hw = GetDlgItem(hwndDlg, IDC_RSTATUS);
			SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)strptr);
			hw = GetDlgItem(hwndDlg, IDC_NRSTATUS);
			SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)strptr);
			hw = GetDlgItem(hwndDlg, IDC_TSTATUS);
			SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)strptr);
			hw = GetDlgItem(hwndDlg, IDC_DSTATUS);
			SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)strptr);
		}
		hw = GetDlgItem(hwndDlg, IDC_RSTATUS);
		SendMessage(hw, CB_SETCURSEL, (WPARAM)(options.rstatus - ID_STATUS_OFFLINE), 0);
		hw = GetDlgItem(hwndDlg, IDC_NRSTATUS);
		SendMessage(hw, CB_SETCURSEL, (WPARAM)(options.nrstatus - ID_STATUS_OFFLINE), 0);
		hw = GetDlgItem(hwndDlg, IDC_TSTATUS);
		SendMessage(hw, CB_SETCURSEL, (WPARAM)(options.tstatus - ID_STATUS_OFFLINE), 0);
		hw = GetDlgItem(hwndDlg, IDC_DSTATUS);
		SendMessage(hw, CB_SETCURSEL, (WPARAM)(options.off_status - ID_STATUS_OFFLINE), 0);
		*/
		SetDlgItemInt(hwndDlg, IDC_RPT, options.retries, FALSE);

		SetDlgItemText(hwndDlg, IDC_ED_FILENAME, options.log_filename);
		if(!options.logging) {
			hw = GetDlgItem(hwndDlg, IDC_ED_FILENAME);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_BTN_LOGBROWSE);
			EnableWindow(hw, FALSE);
		}

		if(!ServiceExists( MS_POPUP_ADDPOPUP )) {
			hw = GetDlgItem(hwndDlg, IDC_CHECKPOPUP);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_CHECKPOPUP2);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_CHK_BLOCK);
			EnableWindow(hw, FALSE);
		} 
		return TRUE;
	}
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			switch( LOWORD( wParam )) {
			case IDC_PPM:
			case IDC_PT:
			case IDC_ED_FILENAME:
			case IDC_RPT:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}	
		}

		if (HIWORD( wParam ) == CBN_SELCHANGE) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}

		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_CHK_LOG:
				hw = GetDlgItem(hwndDlg, IDC_ED_FILENAME);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG));
				hw = GetDlgItem(hwndDlg, IDC_BTN_LOGBROWSE);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG));
				// drop through
			case IDC_CHECKPOPUP:
			case IDC_CHECKPOPUP2:
			case IDC_CHK_BLOCK:
			case IDC_CHK_MINMAX:
			case IDC_CHK_HIDEPROTO:
			case IDC_CHK_USESTATUSMSG:
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			case IDC_BTN_VIEWLOG:
				CallService(PROTO "/ViewLogData", 0, 0);
				break;
			case IDC_BTN_LOGBROWSE:
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFile = options.log_filename;
				ofn.hwndOwner = hwndDlg;
				ofn.Flags = CC_FULLOPEN;
				//ofn.lpstrFile[0] = '\0';
				ofn.nMaxFile = sizeof(options.log_filename);
				ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrInitialDir = NULL;
				ofn.Flags = OFN_PATHMUSTEXIST;

				if(GetOpenFileName(&ofn) == TRUE) {
					SetDlgItemText(hwndDlg, IDC_ED_FILENAME, ofn.lpstrFile);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}
				break;
			case IDC_CHK_NOTESTSTATUS:
				hw = GetDlgItem(hwndDlg, IDC_TSTATUS);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTESTSTATUS) ? FALSE : TRUE);
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
			/*
			{
				int sel1, sel2, sel3, sel4;

				hw = GetDlgItem(hwndDlg, IDC_RSTATUS);
				sel1 = SendMessage(hw, CB_GETCURSEL, 0, 0);
				hw = GetDlgItem(hwndDlg, IDC_NRSTATUS);
				sel2 = SendMessage(hw, CB_GETCURSEL, 0, 0);
				hw = GetDlgItem(hwndDlg, IDC_TSTATUS);
				sel3 = SendMessage(hw, CB_GETCURSEL, 0, 0);
				hw = GetDlgItem(hwndDlg, IDC_DSTATUS);
				sel4 = SendMessage(hw, CB_GETCURSEL, 0, 0);

				if(sel1 == sel4 || sel2 == sel4 || sel3 == sel4) {
					MessageBox(hwndDlg, Translate("You cannot set any status to the same as 'disabled'"), Translate("Error"), MB_OK | MB_ICONWARNING);
					SetWindowLong(hwndDlg, DWL_MSGRESULT, PSNRET_INVALID);
					return TRUE;
				}
				
				if(sel1 != -1)
					options.rstatus = ID_STATUS_OFFLINE + sel1;
				if(sel2 != -1)
					options.nrstatus = ID_STATUS_OFFLINE + sel2;
				if(sel4 != -1)
					options.off_status = ID_STATUS_OFFLINE + sel4;
				if(sel3 != -1)
					options.tstatus = ID_STATUS_OFFLINE + sel3;
			}			
			*/
			BOOL trans_success;

			DWORD new_ping_period = GetDlgItemInt( hwndDlg, IDC_PPM, &trans_success, FALSE);
			if(trans_success) {
				options.ping_period = new_ping_period;
			}
			DWORD new_ping_timeout = GetDlgItemInt( hwndDlg, IDC_PT, &trans_success, FALSE);
			if(trans_success) {
				options.ping_timeout = new_ping_timeout;
			}
			options.show_popup = IsDlgButtonChecked(hwndDlg, IDC_CHECKPOPUP) == BST_CHECKED;
			options.show_popup2 = IsDlgButtonChecked(hwndDlg, IDC_CHECKPOPUP2) == BST_CHECKED;
			options.block_reps = IsDlgButtonChecked(hwndDlg, IDC_CHK_BLOCK) == BST_CHECKED;
			options.logging = IsDlgButtonChecked(hwndDlg, IDC_CHK_LOG) == BST_CHECKED;
			GetDlgItemText(hwndDlg, IDC_ED_FILENAME, options.log_filename, MAX_PATH);

			options.no_test_status = IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTESTSTATUS) == BST_CHECKED;
			options.hide_proto = IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDEPROTO) == BST_CHECKED;
			options.use_status_msg = IsDlgButtonChecked(hwndDlg, IDC_CHK_USESTATUSMSG) == BST_CHECKED;
			DWORD new_retries = GetDlgItemInt( hwndDlg, IDC_RPT, &trans_success, FALSE);
			if(trans_success) {
				options.retries = new_retries;
			}
			SaveOptions();

			if(options.use_status_msg)
				reset_myhandle();

			if(options.logging) CallService(PROTO "/Log", (WPARAM)"options changed", 0);
			if(hWakeEvent) SetEvent(hWakeEvent);
			return TRUE;
		}
		break;
	}

	return FALSE;
}

PINGLIST temp_list;
PINGADDRESS add_edit_addr;

BOOL CALLBACK DlgProcDestEdit(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	HWND hw;
	int sel;
	char *strptr;

	switch ( msg ) {
	case WM_INITDIALOG: 
		{
			bool selgroup = false;
			for(int i = ID_STATUS_OFFLINE; i <= ID_STATUS_OUTTOLUNCH; i++) {
				strptr = (char *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM)i, (LPARAM)0);
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
				SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)strptr);
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
				SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)strptr);
			}

			hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
			SendMessage(hw, CB_SETCURSEL, 1, 0);
			hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
			SendMessage(hw, CB_SETCURSEL, 0, 0);

			hw = GetDlgItem(hwndDlg, IDC_COMBO_GROUP);
			SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Translate("<none>"));
			DBVARIANT dbv;
			if(!DBGetContactSetting(0, PROTO, "DefaultPingGroup", &dbv))
				selgroup = true;
			int gi = 1, selind = -1, selind_defgroup = -1;
			do {
				strptr = (char *)CallService(MS_CLIST_GROUPGETNAME2, (WPARAM)gi, 0);
				if(strptr) {
					SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)strptr);
					if(!strncmp(strptr, add_edit_addr.pszCListGroup, MAX_PINGADDRESS_STRING_LENGTH))
						selind = gi;
					if(selgroup && !strncmp(strptr, dbv.pszVal, MAX_PINGADDRESS_STRING_LENGTH))
						selind_defgroup = gi;
				}
				gi++;
			} while(strptr != 0);
			if(selgroup) DBFreeVariant(&dbv);

			if(selind != -1) SendMessage(hw, CB_SETCURSEL, (WPARAM)selind, 0);
			else if(selind_defgroup != -1) SendMessage(hw, CB_SETCURSEL, (WPARAM)selind_defgroup, 0);
			else SendMessage(hw, CB_SETCURSEL, 0, 0);

			SetDlgItemText(hwndDlg, IDC_ED_DESTADDR, add_edit_addr.pszName);
			SetDlgItemText(hwndDlg, IDC_ED_DESTLAB, add_edit_addr.pszLabel);
			SetDlgItemText(hwndDlg, IDC_ED_COMMAND, add_edit_addr.pszCommand);
			SetDlgItemText(hwndDlg, IDC_ED_PARAMS, add_edit_addr.pszParams);

			CheckDlgButton(hwndDlg, IDC_CHK_DESTTCP, add_edit_addr.port != -1);
			if(add_edit_addr.port != -1) {
				hw = GetDlgItem(hwndDlg, IDC_ED_DESTPORT);
				EnableWindow(hw, TRUE);
				SetDlgItemInt(hwndDlg, IDC_ED_DESTPORT, add_edit_addr.port, FALSE);
			}
			{
				int num_protocols;
				PROTOCOLDESCRIPTOR **pppDesc;

				CallService(MS_PROTO_ENUMPROTOCOLS, (LPARAM)&num_protocols, (WPARAM)&pppDesc);
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTPROTO);
				SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Translate("<none>"));
				SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)Translate("<all>"));
				for(int i = 0; i < num_protocols; i++) {
					if(pppDesc[i]->type == PROTOTYPE_PROTOCOL)
						SendMessage(hw, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)pppDesc[i]->szName);
				}

				if(add_edit_addr.pszProto[0] == '\0') {
					SendMessage(hw, CB_SETCURSEL, 0, 0);
				} else {
					SendMessage(hw, CB_SELECTSTRING, 0, (LPARAM)add_edit_addr.pszProto);
					hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
					EnableWindow(hw, TRUE);
					SendMessage(hw, CB_SETCURSEL, (WPARAM)(add_edit_addr.set_status - ID_STATUS_OFFLINE), 0);
					hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
					EnableWindow(hw, TRUE);
					SendMessage(hw, CB_SETCURSEL, (WPARAM)(add_edit_addr.get_status - ID_STATUS_OFFLINE), 0);
				}
			}
			// ? doesn't work? ?
			hw = GetDlgItem(hwndDlg, IDC_ED_DESTADDR);
			SetFocus(hw);
		}
		return TRUE;
	case WM_COMMAND:
		if (HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_COMBO_DESTPROTO) {
			hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTPROTO);
			sel = SendMessage(hw, CB_GETCURSEL, 0, 0);
			if(sel != CB_ERR) {
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
				EnableWindow(hw, sel != 0);
				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
				EnableWindow(hw, sel != 0);
			}
		}

		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_CHK_DESTTCP:
				hw = GetDlgItem(hwndDlg, IDC_ED_DESTPORT);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_DESTTCP));
				break;
			case IDOK:
				GetDlgItemText(hwndDlg, IDC_ED_DESTADDR, add_edit_addr.pszName, MAX_PINGADDRESS_STRING_LENGTH);
				GetDlgItemText(hwndDlg, IDC_ED_DESTLAB, add_edit_addr.pszLabel, MAX_PINGADDRESS_STRING_LENGTH);
				GetDlgItemText(hwndDlg, IDC_ED_COMMAND, add_edit_addr.pszCommand, MAX_PATH);
				GetDlgItemText(hwndDlg, IDC_ED_PARAMS, add_edit_addr.pszParams, MAX_PATH);

				hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTPROTO);
				if(SendMessage(hw, CB_GETCURSEL, 0, 0) != -1) {
					GetDlgItemText(hwndDlg, IDC_COMBO_DESTPROTO, add_edit_addr.pszProto, MAX_PINGADDRESS_STRING_LENGTH);
					if(!strcmp(add_edit_addr.pszProto, Translate("<none>"))) add_edit_addr.pszProto[0] = '\0';
					else {
						hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT);
						sel = SendMessage(hw, CB_GETCURSEL, 0, 0);
						if(sel != -1)
							add_edit_addr.set_status = ID_STATUS_OFFLINE + sel;
						hw = GetDlgItem(hwndDlg, IDC_COMBO_DESTSTAT2);
						sel = SendMessage(hw, CB_GETCURSEL, 0, 0);
						if(sel != -1)
							add_edit_addr.get_status = ID_STATUS_OFFLINE + sel;
					}
				} else
					add_edit_addr.pszProto[0] = '\0';

				hw = GetDlgItem(hwndDlg, IDC_COMBO_GROUP);
				GetDlgItemText(hwndDlg, IDC_COMBO_GROUP, add_edit_addr.pszCListGroup, MAX_PINGADDRESS_STRING_LENGTH);
				if(!strcmp(add_edit_addr.pszCListGroup, Translate("<none>"))) add_edit_addr.pszCListGroup[0] = '\0';
				DBWriteContactSettingString(0, PROTO, "DefaultPingGroup", add_edit_addr.pszCListGroup);
				
				if(IsDlgButtonChecked(hwndDlg, IDC_CHK_DESTTCP)) {
					BOOL tr;
					int port = GetDlgItemInt(hwndDlg, IDC_ED_DESTPORT, &tr, FALSE);
					if(tr) add_edit_addr.port = port;
					else add_edit_addr.port = -1;
				} else
					add_edit_addr.port = -1;

				EndDialog(hwndDlg, IDOK);
				break;
			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
				
		}

		return TRUE;
	}
	return FALSE;
}

// ping destinations list window
static BOOL CALLBACK DlgProcOpts2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hw;
	int sel;

	switch ( msg ) {
	case WM_INITDIALOG: 
		{
			TranslateDialogDefault( hwndDlg );

			Lock(&data_list_cs, "init options dialog");
			temp_list = data_list;
			Unlock(&data_list_cs);

			hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
			for(PINGLIST::iterator i = temp_list.begin(); i != temp_list.end(); i++) {
				//int index = 
					SendMessage(hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)i->pszLabel);
			}

		}
		return TRUE;

	case WM_COMMAND:
		if (HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LST_DEST) {
			hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
			sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
			if(sel != LB_ERR) {
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTREM);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT);
				EnableWindow(hw, TRUE);

				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
				EnableWindow(hw, (sel > 0));
				hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
				int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
				EnableWindow(hw, (sel < count - 1));
			}
		}

		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_BTN_DESTEDIT:
				hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
				sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
				if(sel != LB_ERR) {
					add_edit_addr = temp_list[sel];
					
					if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hwndDlg, DlgProcDestEdit) == IDOK) {

						temp_list[sel] = add_edit_addr;
						SendMessage(hw, LB_DELETESTRING, (WPARAM)sel, (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)sel, (LPARAM)temp_list[sel].pszLabel);
						SendMessage(hw, LB_SETCURSEL, (WPARAM)sel, 0);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTREM);
						EnableWindow(hw, TRUE);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT);
						EnableWindow(hw, TRUE);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
						EnableWindow(hw, TRUE);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
						EnableWindow(hw, TRUE);
	
						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					}
				}
				break;
			case IDC_BTN_DESTADD:

				memset(&add_edit_addr,0,sizeof(add_edit_addr));
				add_edit_addr.cbSize = sizeof(add_edit_addr);
				add_edit_addr.port = -1;
				add_edit_addr.set_status = ID_STATUS_ONLINE;
				add_edit_addr.get_status = ID_STATUS_OFFLINE;
				add_edit_addr.status = options.nrstatus;

				if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG3), hwndDlg, DlgProcDestEdit) == IDOK) {

					temp_list.push_back(add_edit_addr);

					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					int index = SendMessage(hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)add_edit_addr.pszLabel);
					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					SendMessage(hw, LB_SETCURSEL, (WPARAM)index, 0);

					hw = GetDlgItem(hwndDlg, IDC_BTN_DESTREM);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT);
					EnableWindow(hw, TRUE);

					sel = temp_list.size() - 1;
					hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
					EnableWindow(hw, (sel > 0));
					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
					hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
					EnableWindow(hw, (sel < count - 1));
					
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}

				break;
			case IDC_BTN_DESTREM:
				hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
				sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
				if(sel != LB_ERR) {
					SendMessage(hw, LB_DELETESTRING, (WPARAM)sel, 0);
					PINGLIST::iterator i = temp_list.begin();
					while(sel > 0 && i != temp_list.end()) {
						sel--;
						i++;
					}
					if(i != temp_list.end())
						temp_list.erase(i);
				}

				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTREM);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTEDIT);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
				EnableWindow(hw, FALSE);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
				EnableWindow(hw, FALSE);

				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			case IDC_BTN_DESTDOWN:
				{
					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					int sel2 = SendMessage(hw, LB_GETCURSEL, 0, 0);
					if(sel2 != LB_ERR) {
						add_edit_addr = temp_list[sel2];					
						temp_list[sel2] = temp_list[sel2 + 1];					
						temp_list[sel2 + 1] = add_edit_addr;

						SendMessage(hw, LB_DELETESTRING, (WPARAM)sel2, (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)sel2, (LPARAM)temp_list[sel2].pszLabel);
						SendMessage(hw, LB_DELETESTRING, (WPARAM)(sel2 + 1), (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)(sel2 + 1), (LPARAM)temp_list[sel2 + 1].pszLabel);
						SendMessage(hw, LB_SETCURSEL, (WPARAM)(sel2 + 1), 0);

						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
						EnableWindow(hw, (sel2 + 1 > 0));
						hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
						int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
						EnableWindow(hw, (sel2 + 1 < count - 1));

						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					}				
				}
				break;
			case IDC_BTN_DESTUP:
				{
					hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
					int sel2 = SendMessage(hw, LB_GETCURSEL, 0, 0);
					if(sel2 != LB_ERR) {
						add_edit_addr = temp_list[sel2 - 1];					
						temp_list[sel2 - 1] = temp_list[sel2];					
						temp_list[sel2] = add_edit_addr;

						SendMessage(hw, LB_DELETESTRING, (WPARAM)sel2, (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)sel2, (LPARAM)temp_list[sel2].pszLabel);

						SendMessage(hw, LB_DELETESTRING, (WPARAM)(sel2 - 1), (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)(sel2 - 1), (LPARAM)temp_list[sel2 - 1].pszLabel);

						SendMessage(hw, LB_SETCURSEL, (WPARAM)(sel2 - 1), 0);

						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTUP);
						EnableWindow(hw, (sel2 - 1 > 0));
						hw = GetDlgItem(hwndDlg, IDC_LST_DEST);
						int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DESTDOWN);
						EnableWindow(hw, (sel2 - 1 < count - 1));
		
						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					}				
				}

				break;
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
			CallService(PROTO "/SetAndSavePingList", (WPARAM)&temp_list, 0);
			CallService(PROTO "/GetPingList", 0, (LPARAM)&temp_list);
			// the following will be affected due to list rebuild event
			//if(hWakeEvent) SetEvent(hWakeEvent);
			return TRUE;
		}
		break;

	}
	return FALSE;
}

int PingOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize						= sizeof(odp);
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCE(IDD_DIALOG1);
	odp.pszTitle					= Translate("PING");
	odp.pszGroup					= Translate("Network");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= IDC_PPM;
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	OPTIONSDIALOGPAGE odp2 = { 0 };
	odp2.cbSize						= sizeof(odp);
	odp2.position					= -790000100;
	odp2.hInstance					= hInst;
	odp2.pszTemplate				= MAKEINTRESOURCE(IDD_DIALOG2);
	odp2.pszTitle					= Translate("PING Destinations");
	odp2.pszGroup					= Translate("Network");
	odp2.flags						= ODPF_BOLDGROUPS;
	odp2.nIDBottomSimpleControl		= IDC_PPM;
	odp2.pfnDlgProc					= DlgProcOpts2;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp2 );
	return 0;
}

void LoadOptions() {
	options.ping_period = DBGetContactSettingDword(NULL, PROTO, "PingPeriod", DEFAULT_PING_PERIOD);
	
	options.ping_timeout = DBGetContactSettingDword(NULL, PROTO, "PingTimeout", DEFAULT_PING_TIMEOUT);
	//CallService(PROTO "/SetPingTimeout", (WPARAM)options.ping_timeout, 0);
	options.show_popup = (DBGetContactSettingByte(NULL, PROTO, "ShowPopup", DEFAULT_SHOW_POPUP ? 1 : 0) == 1);
	options.show_popup2 = (DBGetContactSettingByte(NULL, PROTO, "ShowPopup2", DEFAULT_SHOW_POPUP2 ? 1 : 0) == 1);
	options.block_reps = (DBGetContactSettingByte(NULL, PROTO, "BlockReps", DEFAULT_BLOCK_REPS ? 1 : 0) == 1);
	options.logging = (DBGetContactSettingByte(NULL, PROTO, "LoggingEnabled", DEFAULT_LOGGING_ENABLED ? 1 : 0) == 1);

	options.rstatus = DBGetContactSettingWord(NULL, PROTO, "RespondingStatus", ID_STATUS_ONLINE);
	options.nrstatus = DBGetContactSettingWord(NULL, PROTO, "NotRespondingStatus", ID_STATUS_AWAY);
	options.tstatus = DBGetContactSettingWord(NULL, PROTO, "TestingStatus", ID_STATUS_OCCUPIED);
	options.off_status = DBGetContactSettingWord(NULL, PROTO, "DisabledStatus", ID_STATUS_DND);

	if(options.rstatus == options.off_status) {
		options.rstatus = ID_STATUS_ONLINE;
		if(options.off_status == ID_STATUS_ONLINE)
			options.off_status = ID_STATUS_DND;
	}
	if(options.nrstatus == options.off_status) {
		options.rstatus = ID_STATUS_AWAY;
		if(options.off_status == ID_STATUS_AWAY)
			options.off_status = ID_STATUS_DND;
	}
	if(options.tstatus == options.off_status) {
		options.tstatus = ID_STATUS_OCCUPIED;
		if(options.off_status == ID_STATUS_OCCUPIED)
			options.off_status = ID_STATUS_DND;
	}
	
	options.no_test_status = (DBGetContactSettingByte(NULL, PROTO, "NoTestStatus", DEFAULT_NO_TEST_STATUS ? 1 : 0) == 1);
	options.hide_proto = (DBGetContactSettingByte(NULL, PROTO, "HideProtocol", DEFAULT_HIDE_PROTOCOL ? 1 : 0) == 1);
	options.use_status_msg = (DBGetContactSettingByte(NULL, PROTO, "UseStatusMessage", DEFAULT_USE_STATUS_MESSAGE ? 1 : 0) == 1);

	options.retries = DBGetContactSettingDword(NULL, PROTO, "Retries", 0);
	
	CallService(PROTO "/GetLogFilename", (WPARAM)MAX_PATH, (LPARAM)options.log_filename);

	ICMP::get_instance()->set_timeout(options.ping_timeout * 1000);
	
}

void SaveOptions() {
	DBWriteContactSettingDword(NULL, PROTO, "PingPeriod", options.ping_period);
	DBWriteContactSettingDword(NULL, PROTO, "PingTimeout", options.ping_timeout);
	//CallService(PROTO "/SetPingTimeout", (WPARAM)options.ping_timeout, 0);
	DBWriteContactSettingByte(NULL, PROTO, "ShowPopup", options.show_popup ? 1 : 0);
	DBWriteContactSettingByte(NULL, PROTO, "ShowPopup2", options.show_popup2 ? 1 : 0);
	DBWriteContactSettingByte(NULL, PROTO, "BlockReps", options.block_reps ? 1 : 0);
	DBWriteContactSettingByte(NULL, PROTO, "LoggingEnabled", options.logging ? 1 : 0);

	DBWriteContactSettingWord(NULL, PROTO, "RespondingStatus", options.rstatus);
	DBWriteContactSettingWord(NULL, PROTO, "NotRespondingStatus", options.nrstatus);
	DBWriteContactSettingWord(NULL, PROTO, "TestingStatus", options.tstatus);
	DBWriteContactSettingWord(NULL, PROTO, "DisabledStatus", options.off_status);

	DBWriteContactSettingByte(NULL, PROTO, "NoTestStatus", options.no_test_status ? 1 : 0);
	DBWriteContactSettingByte(NULL, PROTO, "HideProtocol", options.hide_proto ? 1 : 0);
	DBWriteContactSettingByte(NULL, PROTO, "UseStatusMessage", options.use_status_msg ? 1 : 0);

	DBWriteContactSettingDword(NULL, PROTO, "Retries", options.retries);

	CallService(PROTO "/SetLogFilename", (WPARAM)MAX_PATH, (LPARAM)options.log_filename);

	ICMP::get_instance()->set_timeout(options.ping_timeout * 1000);
}
