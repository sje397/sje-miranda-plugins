#include "windowtimeout.h"

Options options;

void load_options() {
	options.timeout = DBGetContactSettingDword(0, PLUG, "Timeout", 300);
	options.monitor_recv_only = DBGetContactSettingByte(0, PLUG, "RecvOnly", 0);
}

void save_options() {
	DBWriteContactSettingDword(0, PLUG, "Timeout", options.timeout);
	DBWriteContactSettingByte(0, PLUG, "RecvOnly", (BYTE)(options.monitor_recv_only ? 1 : 0));
}

BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hw;
	char buff[512];

	switch ( msg ) {
	case WM_INITDIALOG: 
		{
			TranslateDialogDefault( hwndDlg );
			itoa(options.timeout, buff, 10);
			SetDlgItemText(hwndDlg, IDC_ED_TIMEOUT, buff);
			CheckDlgButton(hwndDlg, IDC_CHK_RECVONLY, options.monitor_recv_only ? TRUE : FALSE);
		}
		return TRUE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_CHK_RECVONLY:
				SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				break;
			}
		} else if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
		}
		return TRUE;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			hw = GetDlgItem(hwndDlg, IDC_ED_TIMEOUT);
			GetWindowText(hw, buff, 512);
			if(strlen(buff) > 0)
				options.timeout = atoi(buff);
			options.monitor_recv_only = IsDlgButtonChecked(hwndDlg, IDC_CHK_RECVONLY);
			save_options();
			return TRUE;
		}
		break;
	}

	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp;
	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize						= sizeof(odp);
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCE(IDD_OPT);
	odp.pszTitle					= Translate("Window Timeout");
	odp.pszGroup					= Translate("Events");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );	

	return 0;
}

