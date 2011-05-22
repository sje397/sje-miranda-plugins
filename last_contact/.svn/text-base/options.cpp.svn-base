#include "common.h"
#include "options.h"

Options options;

bool win_key_down;
static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG: {
		TranslateDialogDefault( hwndDlg );

		WORD mod = 0;
		if(options.mod_alt) mod |= HOTKEYF_ALT;
		if(options.mod_shift) mod |= HOTKEYF_SHIFT;
		if(options.mod_ctrl) mod |= HOTKEYF_CONTROL;
		if(options.mod_win) mod |= HOTKEYF_EXT;
		
		CheckDlgButton(hwndDlg, IDC_CHK_WIN, options.mod_win ? TRUE : FALSE);
		SendDlgItemMessage(hwndDlg, IDC_HOTKEY, HKM_SETHOTKEY, (mod << 8) + options.vk, 0);

		CheckDlgButton(hwndDlg, IDC_RAD_HIDE, options.hide_if_visible ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_RAD_MULTI, options.hide_if_visible ? FALSE : TRUE);

		return TRUE;
	}
	case WM_COMMAND:
		if(HIWORD(wParam) == BN_CLICKED) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		} else
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);		
			win_key_down = (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0);
			CheckDlgButton(hwndDlg, IDC_CHK_WIN, win_key_down ? TRUE : FALSE);
		}
		return TRUE;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
			WORD hkcode = (WORD)SendDlgItemMessage(hwndDlg, IDC_HOTKEY, HKM_GETHOTKEY, 0, 0);
			options.vk = LOBYTE(hkcode);
			options.mod_alt = (HIBYTE(hkcode) & HOTKEYF_ALT) != 0;
			options.mod_shift = (HIBYTE(hkcode) & HOTKEYF_SHIFT) != 0;
			options.mod_ctrl = (HIBYTE(hkcode) & HOTKEYF_CONTROL) != 0;
			//options.mod_win = (HIBYTE(hkcode) & HOTKEYF_EXT) != 0;
			options.mod_win = IsDlgButtonChecked(hwndDlg, IDC_CHK_WIN) ? true : false;

			DBWriteContactSettingByte(0, MODULE, "VK", (BYTE)options.vk);
			DBWriteContactSettingByte(0, MODULE, "MOD_ALT", options.mod_alt ? 1 : 0);
			DBWriteContactSettingByte(0, MODULE, "MOD_SHIFT", options.mod_shift ? 1 : 0);
			DBWriteContactSettingByte(0, MODULE, "MOD_CTRL", options.mod_ctrl ? 1 : 0);
			DBWriteContactSettingByte(0, MODULE, "MOD_WIN", options.mod_win ? 1 : 0);

			options.hide_if_visible = IsDlgButtonChecked(hwndDlg, IDC_RAD_HIDE) ? true : false;
			DBWriteContactSettingByte(0, MODULE, "HideIfVisible", options.hide_if_visible ? 1 : 0);

			PostThreadMessage(hotkey_thread_id, WMU_RESET_HOTKEY, 0, 0);
		}
		return TRUE;
	}

	return FALSE;
}


int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize						= sizeof(odp);
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCE(IDD_OPT1);
	odp.pszTitle					= Translate("Last Contact");
	odp.pszGroup					= Translate("Events");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

void InitOptions() {
	options.vk = DBGetContactSettingByte(0, MODULE, "VK", (BYTE)'M');
	options.mod_alt = (DBGetContactSettingByte(0, MODULE, "MOD_ALT", 1) == 1);
	options.mod_shift = (DBGetContactSettingByte(0, MODULE, "MOD_SHIFT", 0) == 1);
	options.mod_ctrl = (DBGetContactSettingByte(0, MODULE, "MOD_CTRL", 0) == 1);
	options.mod_win = (DBGetContactSettingByte(0, MODULE, "MOD_WIN", 0) == 1);

	options.hide_if_visible = (DBGetContactSettingByte(0, MODULE, "HideIfVisible", 1) == 1);
	HookEvent(ME_OPT_INITIALISE, OptInit);
}
