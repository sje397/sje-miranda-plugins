#include "win.h"
#include "options.h"

Options options;

void EnableTitleBar() {
	if(!pluginwind) return;

	bool visible = IsWindowVisible(pluginwind) ? true : false;

	SetWindowLong(pluginwind, GWL_STYLE, (options.show_titlebar ? STYLE_TITLE : STYLE_NOTITLE) | (visible ? WS_VISIBLE : 0));
	SetWindowPos(pluginwind, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void ResetTabIcons() {
	PostMessage(pluginwind, WM_RESETTABICONS, 0, 0);
}

static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG: {
		TranslateDialogDefault( hwndDlg );
		CheckDlgButton(hwndDlg, IDC_CHK_TITLE, options.show_titlebar ? FALSE : TRUE);
		CheckDlgButton(hwndDlg, IDC_CHK_TABICON, options.tab_icon ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_ONTOP, options.ontop ? TRUE : FALSE);
		return TRUE;
	}
	case WM_COMMAND:
		if(HIWORD(wParam) == BN_CLICKED) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		return TRUE;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
			options.show_titlebar = IsDlgButtonChecked(hwndDlg, IDC_CHK_TITLE) ? false : true;
			DBWriteContactSettingByte(0, MODULE, "EnableTitle", options.show_titlebar ? 1 : 0);
			EnableTitleBar();

			options.tab_icon = IsDlgButtonChecked(hwndDlg, IDC_CHK_TABICON) ? true : false;
			DBWriteContactSettingByte(0, MODULE, "TabIcon", options.tab_icon ? 1 : 0);
			ResetTabIcons();

			options.ontop = IsDlgButtonChecked(hwndDlg, IDC_CHK_ONTOP) ? true : false;
			DBWriteContactSettingByte(0, MODULE, "OnTop", options.ontop ? 1 : 0);
			FixWindowStyle();
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
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT1);
	odp.pszTitle					= Translate("Messaging Tabs");
	odp.pszGroup					= Translate("Events");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

void InitOptions() {
	options.show_titlebar = (DBGetContactSettingByte(0, MODULE, "EnableTitle", 1) == 1);
	options.tab_icon = (DBGetContactSettingByte(0, MODULE, "TabIcon", 0) == 1);
	options.ontop = (DBGetContactSettingByte(0, MODULE, "OnTop", 0) == 1);

	HookEvent(ME_OPT_INITIALISE, OptInit);
}
