#include "common.h"
#include "nick_dialog.h"
#include "resource.h"
#include "server_con.h"
#include "formatting.h"

int check_rid = 0;
int set_rid = 0;

HWND nick_dialog = 0;


void CheckAvailable(HWND hwndDlg, TCHAR *nick) {
	ClientNetMessage msg;
	msg.add_int("persist", 1);
	msg.add_int("sesskey", sesskey);
	msg.add_int("cmd", 1);
	msg.add_int("dsn", 5);
	msg.add_int("uid", my_uid);
	msg.add_int("lid", 7);
	msg.add_int("rid", check_rid = req_id++);

	char an[256];
#ifdef _UNICODE
	WideCharToMultiByte(CP_UTF8, 0, nick, -1, an, 256, 0, 0);
#else
	strncpy(an, nick, 256);
#endif
	Dictionary d;
	d.add_string("UserName", an);
	msg.add_dict("body", d);

	SendMessage(msg);
}

void SetNick(HWND hwndDlg, TCHAR *nick) {
	ClientNetMessage msg;
	msg.add_int("persist", 1);
	msg.add_int("sesskey", sesskey);
	msg.add_int("cmd", 2);
	msg.add_int("dsn", 9);
	msg.add_int("uid", my_uid);
	msg.add_int("lid", 14);
	msg.add_int("rid", set_rid = req_id++);

	char an[256];
#ifdef _UNICODE
	WideCharToMultiByte(CP_UTF8, 0, nick, -1, an, 256, 0, 0);
#else
	strncpy(an, nick, 256);
#endif
	Dictionary d;
	d.add_string("UserName", an);
	msg.add_dict("body", d);

	SendMessage(msg);
}

BOOL CALLBACK DlgProcNick(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		nick_dialog = hwndDlg;
		return FALSE;		
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && LOWORD(wParam) == IDC_ED_NICK) {
			TCHAR buff[256];
			GetDlgItemText(hwndDlg, IDC_ED_NICK, buff, 256);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_CHECK), _tcslen(buff) != 0);
			EnableWindow(GetDlgItem(hwndDlg, IDOK), false);			
		} else {
			switch(LOWORD(wParam)) {
				case IDC_BTN_CHECK:
					{
						TCHAR buff[256];
						GetDlgItemText(hwndDlg, IDC_ED_NICK, buff, 256);
						CheckAvailable(hwndDlg, buff);
						SetDlgItemText(hwndDlg, IDC_ST_MSG, TranslateT("Checking availability. Please wait."));
						EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_CHECK), false);			
					}
					break;
				case IDOK:
					{
						TCHAR buff[256];
						GetDlgItemText(hwndDlg, IDC_ED_NICK, buff, 256);
						SetNick(hwndDlg, buff);
						SetDlgItemText(hwndDlg, IDC_ST_MSG, TranslateT("Setting your nick name. Please wait."));
						EnableWindow(GetDlgItem(hwndDlg, IDOK), false);			
					}
					break;
				case IDCANCEL:
					nick_dialog = 0;
					EndDialog(hwndDlg, IDCANCEL);
					break;
			}
		}
		break;
	case WMU_NICKEXISTS:
		if(lParam == check_rid) {
			check_rid = 0;
			if(wParam) {
				SetDlgItemText(hwndDlg, IDC_ST_MSG, TranslateT("Sorry, that nick name is already taken."));
			} else {
				SetDlgItemText(hwndDlg, IDC_ST_MSG, TranslateT("That nick name is available. Press OK to use it as your personal nick name."));
				EnableWindow(GetDlgItem(hwndDlg, IDOK), true);			
			}
		}
		break;
	case WMU_CHANGEOK:
		if(lParam == set_rid) {
			set_rid = 0;
			if(wParam) {
				TCHAR buff[256];
				GetDlgItemText(hwndDlg, IDC_ED_NICK, buff, 256);
				DBWriteContactSettingTString(0, MODULE, "Nick", buff);

				nick_dialog = 0;						
				EndDialog(hwndDlg, IDOK);
			} else {
				SetDlgItemText(hwndDlg, IDC_ST_MSG, TranslateT("Failed to set your nick name."));
				EnableWindow(GetDlgItem(hwndDlg, IDOK), false);			
			}
		}
		break;
	}

	return 0;
}

void CALLBACK sttMainMakeDialogCallback( ULONG dwParam ) {
	HWND dlg = CreateDialog(hInst, MAKEINTRESOURCE(IDD_NICK), GetDesktopWindow(), DlgProcNick);
	ShowWindow(dlg, SW_SHOW);
	UpdateWindow(dlg);
}

void ShowNickDialog() {
	QueueUserAPC(sttMainMakeDialogCallback, mainThread, 0);
}