#include "common.h"
#include "plugwin.h"
//#include "commctrl.h"

void save_listbox_items() {
	DBWriteContactSettingDword(0, "WorldTime", "NumEntries", listbox_items.getCount());
	int index = 0;
	for(int i = 0; i < listbox_items.getCount(); i++, index++) {
		char p1[20], p2[20], p3[20], p4[20], p5[20];

		mir_snprintf(p1, SIZEOF(p1), "Label%d", i);
		mir_snprintf(p2, SIZEOF(p2), "Index%d", i);
		mir_snprintf(p3, SIZEOF(p3), "Sunrise%d", i);
		mir_snprintf(p4, SIZEOF(p4), "Sunset%d", i);
		mir_snprintf(p5, SIZEOF(p5), "TZName%d", i);

		DBWriteContactSettingTString(0, "WorldTime", p1, listbox_items[i].pszText);
		DBWriteContactSettingDword(0, "WorldTime", p2, listbox_items[i].timezone_list_index);
		DBWriteContactSettingDword(0, "WorldTime", p3, listbox_items[i].sunrise.wHour * 60 + listbox_items[i].sunrise.wMinute);
		DBWriteContactSettingDword(0, "WorldTime", p4, listbox_items[i].sunset.wHour * 60 + listbox_items[i].sunset.wMinute);
		DBWriteContactSettingTString(0, "WorldTime", p5, timezone_list[listbox_items[i].timezone_list_index].tcName);
	}
	DBWriteContactSettingWord(0, "WorldTime", "DataVersion", 1);
}

void copy_listbox_items(ITEMLIST &dest, ITEMLIST &src)
{
	dest.destroy();
	for (int i=0; i < src.getCount(); ++i)
		dest.insert(new LISTITEM(src[i]));
}

void set_minmax(bool mm) {
	hook_window_behaviour_to_clist = mm;
	if(hook_window_behaviour_to_clist) {
		BYTE state = DBGetContactSettingByte(NULL, "CList", "Setting", SETTING_STATE_NORMAL);

		if(pluginwind) {
			if(state == SETTING_STATE_NORMAL)
				ShowWindow(pluginwind, SW_SHOW);
			else
				ShowWindow(pluginwind, SW_HIDE);
		}
	}
}

void set_set_format(bool sf) {
	set_format = sf;
}

void set_time_format(const TCHAR *sf) {
	_tcsncpy(format_string, sf, 512);
}

void set_date_format(const TCHAR *sf) {
	_tcsncpy(date_format_string, sf, 512);
}

void set_show_icons(bool si) {
	show_icons = si;
}

void set_hide_menu(bool hm) {
	hide_menu = hm;
}

ITEMLIST temp_listbox_items(5);

void fill_timezone_list_control(HWND hwndDlg) 
{
	HWND hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES);
	SendMessage(hw, LB_RESETCONTENT, 0, 0);

	if(IsDlgButtonChecked(hwndDlg, IDC_RAD_ALPHA)) {

		for(int i = 0; i < timezone_list.getCount(); i++) {
			SendMessage(hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)timezone_list[i].tcName);
			SendMessage(hw, LB_SETITEMDATA, (WPARAM)i, (LPARAM)timezone_list[i].list_index);
		}
	} else {
		for(int i = 0; i < geo_timezone_list.getCount(); i++) {
			SendMessage(hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)timezone_list[i].tcName);
			SendMessage(hw, LB_SETITEMDATA, (WPARAM)i, (LPARAM)timezone_list[i].list_index);
		}
	}
}


LISTITEM add_edit_item;

static INT_PTR CALLBACK DlgProcOpts2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	HWND hw;
	int sel;
	TCHAR buf[MAX_NAME_LENGTH];

	switch ( msg ) {
	case WM_INITDIALOG: 
		{
			TranslateDialogDefault( hwndDlg );

			CheckDlgButton(hwndDlg, IDC_RAD_ALPHA, 1);
			CheckDlgButton(hwndDlg, IDC_RAD_GEO, 0);

			fill_timezone_list_control(hwndDlg);

			if(add_edit_item.timezone_list_index != -1) {
				hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES);
				SendMessage(hw, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)timezone_list[add_edit_item.timezone_list_index].tcName);
				SetDlgItemText(hwndDlg, IDC_ED_LABEL, add_edit_item.pszText);

				hw = GetDlgItem(hwndDlg, IDC_ED_LABEL);
				EnableWindow(hw, TRUE);
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, TRUE);

				SendDlgItemMessage(hwndDlg, IDC_TIME_SUNRISE, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&add_edit_item.sunrise);
				SendDlgItemMessage(hwndDlg, IDC_TIME_SUNSET, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&add_edit_item.sunset);

				hw = GetDlgItem(hwndDlg, IDC_ED_DISP);
				SetWindowText(hw, timezone_list[add_edit_item.timezone_list_index].tcDisp);
			} else {
				SYSTEMTIME systime = {0};

				GetSystemTime(&systime);

				systime.wHour = 6;
				systime.wMinute = 0;
				systime.wSecond = 0;
				SendDlgItemMessage(hwndDlg, IDC_TIME_SUNRISE, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&systime);
				systime.wHour = 18;
				SendDlgItemMessage(hwndDlg, IDC_TIME_SUNSET, DTM_SETSYSTEMTIME, (WPARAM)GDT_VALID, (LPARAM)&systime);

			}
		}
		break;

	case WM_COMMAND: 
		if (HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LIST_TIMES) {
			hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES);
			sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
			if(sel != LB_ERR) {
				hw = GetDlgItem(hwndDlg, IDC_ED_LABEL);
				EnableWindow(hw, sel != -1);
				if(sel == -1)
					add_edit_item.timezone_list_index = -1;
				else {
					hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES);
					add_edit_item.timezone_list_index = (int)SendMessage(hw, LB_GETITEMDATA, sel, 0);

					hw = GetDlgItem(hwndDlg, IDC_ED_DISP);
					SetWindowText(hw, timezone_list[add_edit_item.timezone_list_index].tcDisp);
				}
			}
		}

		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			switch( LOWORD( wParam )) {
			case IDC_ED_LABEL:
				GetDlgItemText(hwndDlg, IDC_ED_LABEL, buf, MAX_NAME_LENGTH);
				hw = GetDlgItem(hwndDlg, IDOK);
				EnableWindow(hw, (_tcslen(buf) > 0));
				_tcsncpy(add_edit_item.pszText, buf, MAX_NAME_LENGTH);
			}
		}

		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_RAD_ALPHA:
				CheckDlgButton(hwndDlg, IDC_RAD_GEO, 0);
				fill_timezone_list_control(hwndDlg);
				hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES);
				if(add_edit_item.timezone_list_index != -1)
					SendMessage(hw, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)timezone_list[add_edit_item.timezone_list_index].tcName);
				break;
			case IDC_RAD_GEO:
				CheckDlgButton(hwndDlg, IDC_RAD_ALPHA, 0);
				fill_timezone_list_control(hwndDlg);
				hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES);
				if(add_edit_item.timezone_list_index != -1)
					SendMessage(hw, LB_SELECTSTRING, (WPARAM)-1, (LPARAM)timezone_list[add_edit_item.timezone_list_index].tcName);
				break;
			case IDOK:
				if(SendDlgItemMessage(hwndDlg, IDC_TIME_SUNRISE, DTM_GETSYSTEMTIME, 0, (LPARAM)&add_edit_item.sunrise) != GDT_VALID) {
					add_edit_item.sunrise.wHour = 6;
					add_edit_item.sunrise.wMinute = 0;
					add_edit_item.sunrise.wSecond = 0;
				}
				if(SendDlgItemMessage(hwndDlg, IDC_TIME_SUNSET, DTM_GETSYSTEMTIME, 0, (LPARAM)&add_edit_item.sunset) != GDT_VALID) {
					add_edit_item.sunset.wHour = 18;
					add_edit_item.sunset.wMinute = 0;
					add_edit_item.sunset.wSecond = 0;
				}
				EndDialog(hwndDlg, IDOK);
				break;
			case IDCANCEL:
				EndDialog(hwndDlg, IDCANCEL);
				break;
			}
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	HWND hw;
	int sel;
	DBVARIANT dbv;

	switch ( msg ) {
	case WM_INITDIALOG: {
		TranslateDialogDefault( hwndDlg );

		SendMessage(GetDlgItem(hwndDlg, IDC_SPIN1), UDM_SETRANGE, 0, (LPARAM)MAKELONG(50, 5));
		if(!DBGetContactSetting(NULL,"WorldTime","FontSize",&dbv)
			|| !DBGetContactSetting(NULL,"CLC","Font0Size",&dbv)) 
		{
			SendMessage(GetDlgItem(hwndDlg, IDC_SPIN1), UDM_SETPOS, 0, dbv.bVal);
		}

		SendMessage(GetDlgItem(hwndDlg, IDC_SP_INDENT), UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 0));
		SendMessage(GetDlgItem(hwndDlg, IDC_SP_INDENT), UDM_SETPOS, 0, DBGetContactSettingWord(NULL, "WorldTime", "Indent", 0));
		SendMessage(GetDlgItem(hwndDlg, IDC_SP_ROWHEIGHT), UDM_SETRANGE, 0, (LPARAM)MAKELONG(500, 6));
		SendMessage(GetDlgItem(hwndDlg, IDC_SP_ROWHEIGHT), UDM_SETPOS, 0, DBGetContactSettingWord(NULL, "WorldTime", "RowHeight", GetSystemMetrics(SM_CYSMICON)));

		copy_listbox_items(temp_listbox_items, listbox_items);
		hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);

		for(int ili = 0; ili < temp_listbox_items.getCount(); ili++) {
			sel = SendMessage(hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)&temp_listbox_items[ili].pszText);
		}

		if(!ServiceExists(MS_CLIST_FRAMES_ADDFRAME)) {
			bool minmax = (DBGetContactSettingByte(NULL, "WorldTime", "MinMax", DEFAULT_MINMAX ? 1 : 0) == 1);
			CheckDlgButton(hwndDlg, IDC_CHK_MINMAX, minmax);
			bool hide_menu = (DBGetContactSettingByte(NULL, "WorldTime", "HideMenu", 0) == 1);
			CheckDlgButton(hwndDlg, IDC_CHK_HIDEMENU, hide_menu ? 1 : 0);
		} else {
			CheckDlgButton(hwndDlg, IDC_CHK_HIDEMENU, TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_HIDEMENU), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_SHOW), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_MINMAX), FALSE);
		}

		bool set_format = (DBGetContactSettingByte(NULL, "WorldTime", "EnableTimeFormat", 0) == 1);
		CheckDlgButton(hwndDlg, IDC_CHK_FORMAT, set_format ? 1 : 0);
		bool show_icons = (DBGetContactSettingByte(NULL, "WorldTime", "ShowIcons", 1) == 1);
		CheckDlgButton(hwndDlg, IDC_CHK_ICONS, show_icons ? 1 : 0);
		DBVARIANT dbv;
		if(!DBGetContactSettingTString(NULL, "WorldTime", "TimeFormat", &dbv))
			_tcscpy(format_string, dbv.ptszVal);
		DBFreeVariant(&dbv);
		SetDlgItemText(hwndDlg, IDC_ED_FORMAT, format_string);
		if(!DBGetContactSettingTString(NULL, "WorldTime", "DateFormat", &dbv))
			_tcscpy(date_format_string, dbv.ptszVal);
		DBFreeVariant(&dbv);
		SetDlgItemText(hwndDlg, IDC_ED_DATE_FORMAT, date_format_string);

		if(!set_format) {
			hw = GetDlgItem(hwndDlg, IDC_ED_FORMAT);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_ED_DATE_FORMAT);
			EnableWindow(hw, FALSE);
		}

		SendDlgItemMessage(hwndDlg, IDC_TEXTCOL, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, "WorldTime", "FontCol", GetSysColor(COLOR_WINDOWTEXT)));
		SendDlgItemMessage(hwndDlg, IDC_BGCOL, CPM_SETCOLOUR, 0, DBGetContactSettingDword(NULL, "WorldTime", "BgColour", GetSysColor(COLOR_3DFACE)));

		if(ServiceExists(MS_FONT_REGISTERT)) {
		    ShowWindow(GetDlgItem(hwndDlg, IDC_STATICH), SW_HIDE);
		    ShowWindow(GetDlgItem(hwndDlg, IDC_STATICH2), SW_HIDE);
		    ShowWindow(GetDlgItem(hwndDlg, IDC_SPIN1), SW_HIDE);
		    ShowWindow(GetDlgItem(hwndDlg, IDC_ED_FSIZE), SW_HIDE);
		    ShowWindow(GetDlgItem(hwndDlg, IDC_TEXTCOL), SW_HIDE);

			ShowWindow(GetDlgItem(hwndDlg, IDC_STATFS), SW_SHOW);

		}

		//return TRUE;
		return FALSE;
	}
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			switch( LOWORD( wParam )) {
			case IDC_ED_FORMAT:
			case IDC_ED_DATE_FORMAT:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}	
		}

		if (HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LIST_TIMES2) {
			hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
			sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
			if(sel != LB_ERR) {
				hw = GetDlgItem(hwndDlg, IDC_BTN_REM);
				EnableWindow(hw, sel != -1);
				hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT);
				EnableWindow(hw, sel != -1);

				hw = GetDlgItem(hwndDlg, IDC_BTN_UP);
				EnableWindow(hw, (sel > 0));
				hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
				int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
				hw = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
				EnableWindow(hw, (sel < count - 1));
			}
		}

		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_BTN_SHOW:
				ShowWindow(pluginwind, SW_SHOW);
				break;
			case IDC_BTN_EDIT:
				hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
				sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
				if(sel != LB_ERR && sel != -1) {

					add_edit_item = temp_listbox_items[sel];
					if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hwndDlg, DlgProcOpts2) == IDOK) {
						temp_listbox_items[sel] = add_edit_item;
						
						SendMessage(hw, LB_DELETESTRING, (WPARAM)sel, (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)sel, (LPARAM)add_edit_item.pszText);
						SendMessage(hw, LB_SETCURSEL, (WPARAM)sel, 0);

						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					}
				}
				break;

			case IDC_BTN_ADD:

				add_edit_item.pszText[0] = '\0';
				add_edit_item.timezone_list_index = -1;

				if(DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hwndDlg, DlgProcOpts2) == IDOK) {
					temp_listbox_items.insert(new LISTITEM(add_edit_item));

					hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
					sel = SendMessage(hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)add_edit_item.pszText);
					SendMessage(hw, LB_SETCURSEL, (WPARAM)sel, 0);

					hw = GetDlgItem(hwndDlg, IDC_BTN_REM);
					EnableWindow(hw, TRUE);
					hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT);
					EnableWindow(hw, TRUE);

					sel = temp_listbox_items.getCount() - 1;
					hw = GetDlgItem(hwndDlg, IDC_BTN_UP);
					EnableWindow(hw, (sel > 0));
					hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
					int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
					hw = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
					EnableWindow(hw, (sel < count - 1));

					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}
				break;

			case IDC_BTN_REM:
				hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
				sel = SendMessage(hw, LB_GETCURSEL, 0, 0);
				if(sel != LB_ERR) {
					SendMessage(hw, LB_DELETESTRING, (WPARAM)sel, 0);

					temp_listbox_items.remove(sel);

					hw = GetDlgItem(hwndDlg, IDC_BTN_REM);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_BTN_EDIT);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_BTN_UP);
					EnableWindow(hw, FALSE);
					hw = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
					EnableWindow(hw, FALSE);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				}
				break;
			case IDC_BTN_DOWN:
				{
					hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
					int sel2 = SendMessage(hw, LB_GETCURSEL, 0, 0);
					if(sel2 != LB_ERR) {
						add_edit_item = temp_listbox_items[sel2];					
						temp_listbox_items[sel2] = temp_listbox_items[sel2 + 1];					
						temp_listbox_items[sel2 + 1] = add_edit_item;

						SendMessage(hw, LB_DELETESTRING, (WPARAM)sel2, (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)sel2, (LPARAM)temp_listbox_items[sel2].pszText);
						SendMessage(hw, LB_DELETESTRING, (WPARAM)(sel2 + 1), (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)(sel2 + 1), (LPARAM)temp_listbox_items[sel2 + 1].pszText);
						SendMessage(hw, LB_SETCURSEL, (WPARAM)(sel2 + 1), 0);

						hw = GetDlgItem(hwndDlg, IDC_BTN_UP);
						EnableWindow(hw, (sel2 + 1 > 0));
						hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
						int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
						EnableWindow(hw, (sel2 + 1 < count - 1));

						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					}				
				}
				break;
			case IDC_BTN_UP:
				{
					hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
					int sel2 = SendMessage(hw, LB_GETCURSEL, 0, 0);
					if(sel2 != LB_ERR) {
						add_edit_item = temp_listbox_items[sel2];					
						temp_listbox_items[sel2] = temp_listbox_items[sel2 - 1];					
						temp_listbox_items[sel2 - 1] = add_edit_item;

						SendMessage(hw, LB_DELETESTRING, (WPARAM)sel2, (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)sel2, (LPARAM)temp_listbox_items[sel2].pszText);
						SendMessage(hw, LB_DELETESTRING, (WPARAM)(sel2 - 1), (LPARAM)0);
						SendMessage(hw, LB_INSERTSTRING, (WPARAM)(sel2 - 1), (LPARAM)temp_listbox_items[sel2 - 1].pszText);
						SendMessage(hw, LB_SETCURSEL, (WPARAM)(sel2 - 1), 0);

						hw = GetDlgItem(hwndDlg, IDC_BTN_UP);
						EnableWindow(hw, (sel2 - 1 > 0));
						hw = GetDlgItem(hwndDlg, IDC_LIST_TIMES2);
						int count = SendMessage(hw, LB_GETCOUNT, 0, 0);
						hw = GetDlgItem(hwndDlg, IDC_BTN_DOWN);
						EnableWindow(hw, (sel2 - 1 < count - 1));

						SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					}				
				}
				break;
			case IDC_CHK_ICONS:
			case IDC_CHK_MINMAX:
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			case IDC_CHK_HIDEMENU:
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			case IDC_CHK_FORMAT:
				hw = GetDlgItem(hwndDlg, IDC_ED_FORMAT);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_FORMAT));
				hw = GetDlgItem(hwndDlg, IDC_ED_DATE_FORMAT);
				EnableWindow(hw, IsDlgButtonChecked(hwndDlg, IDC_CHK_FORMAT));
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			}
		}
		if(LOWORD(wParam) == IDC_TEXTCOL || LOWORD(wParam) == IDC_BGCOL
			|| LOWORD(wParam) == IDC_SPIN1 || LOWORD(wParam) == IDC_SP_INDENT || LOWORD(wParam) == IDC_SP_ROWHEIGHT)
		{
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
		}
		break;

	case WM_NOTIFY:
		if(((LPNMHDR)lParam)->code == UDN_DELTAPOS ) {
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
		}
		if(((LPNMHDR)lParam)->code == PSN_APPLY ) {
			bool new_minmax = IsDlgButtonChecked(hwndDlg, IDC_CHK_MINMAX) == BST_CHECKED;
			DBWriteContactSettingByte(NULL, "WorldTime", "MinMax", new_minmax ? 1 : 0);
			set_minmax(new_minmax);

			bool new_set_format = IsDlgButtonChecked(hwndDlg, IDC_CHK_FORMAT) == BST_CHECKED;
			DBWriteContactSettingByte(NULL, "WorldTime", "EnableTimeFormat", new_set_format ? 1 : 0);
			set_set_format(new_set_format);

			bool new_show_icons = IsDlgButtonChecked(hwndDlg, IDC_CHK_ICONS) == BST_CHECKED;
			DBWriteContactSettingByte(NULL, "WorldTime", "ShowIcons", new_show_icons ? 1 : 0);
			set_show_icons(new_show_icons);

			bool new_hide_menu = IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDEMENU) == BST_CHECKED;
			DBWriteContactSettingByte(NULL, "WorldTime", "HideMenu", new_hide_menu ? 1 : 0);
			set_hide_menu(new_hide_menu);

			TCHAR buf[512];
			GetDlgItemText(hwndDlg, IDC_ED_FORMAT, buf, 512);
			DBWriteContactSettingTString(NULL, "WorldTime", "TimeFormat", buf);
			set_time_format(buf);

			GetDlgItemText(hwndDlg, IDC_ED_DATE_FORMAT, buf, 512);
			DBWriteContactSettingTString(NULL, "WorldTime", "DateFormat", buf);
			set_date_format(buf);

			copy_listbox_items(listbox_items, temp_listbox_items);
			save_listbox_items();

			if(!ServiceExists(MS_FONT_REGISTERT)) {
				DBWriteContactSettingDword(NULL, "WorldTime", "FontCol", ContactFontColour = SendDlgItemMessage(hwndDlg, IDC_TEXTCOL, CPM_GETCOLOUR, 0, 0));
				DBWriteContactSettingByte(0, "WorldTime", "FontSize", (BYTE)SendMessage(GetDlgItem(hwndDlg, IDC_SPIN1), UDM_GETPOS, 0, 0) & 255);
			}

			DBWriteContactSettingDword(NULL, "WorldTime", "BgColour", SendDlgItemMessage(hwndDlg, IDC_BGCOL, CPM_GETCOLOUR, 0, 0));

			DBWriteContactSettingWord(0, "WorldTime", "Indent", (WORD)SendMessage(GetDlgItem(hwndDlg, IDC_SP_INDENT), UDM_GETPOS, 0, 0));
			DBWriteContactSettingWord(0, "WorldTime", "RowHeight", (WORD)SendMessage(GetDlgItem(hwndDlg, IDC_SP_ROWHEIGHT), UDM_GETPOS, 0, 0));

			DeleteObject(ContactFont);
			ContactFont = (HFONT)GetFont();

			if(pluginwind) {
				/*
				RECT r;
				SIZE textSize;
				GetWindowRect(pluginwind, &r);
				HFONT hOldFont = (HFONT)SelectObject(GetDC(pluginwind), ContactFont);
				GetTextExtentPoint32(GetDC(pluginwind),"X",1,&textSize);
				SelectObject(GetDC(pluginwind), hOldFont);
				SetWindowPos(pluginwind, 0, 0, 0, r.right - r.left, textSize.cy * listbox_items.size(), SWP_NOZORDER | SWP_NOMOVE);
				if(Frameid != -1) {
					CallService(MS_CLIST_FRAMES_SETFRAMEOPTIONS, (WPARAM)MAKELONG(FO_HEIGHT, Frameid), (LPARAM)DBGetContactSettingByte(0, "WorldTime", "FontSize", 10) * listbox_items.size());
				}
				*/
				FillList(0, 0);				
			}

			return TRUE;
		}
		break;
	}

	return FALSE;
}

int WorldTimeOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };

	odp.cbSize						= sizeof(odp);
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_DIALOG1);
	odp.pszTitle					= LPGEN("World Time");
	odp.pszGroup					= LPGEN("Plugins");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

