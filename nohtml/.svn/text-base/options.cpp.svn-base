#include "common.h"
#include "options.h"
#include "resource.h"

Options options;

void LoadOptions() {
	options.filter_in = (DBGetContactSettingByte(0, MODULE, "FilterIn", 1) != 0);
	options.filter_out = (DBGetContactSettingByte(0, MODULE, "FilterOut", 0) != 0);
	options.bbcodes = (DBGetContactSettingByte(0, MODULE, "BBCodes", 0) != 0);
	options.html = (DBGetContactSettingByte(0, MODULE, "HTML", 0) != 0);

	options.apply_to = DBGetContactSettingDword(0, MODULE, "ApplyTo", ATF_ALL);
}

void SaveOptions() {
	DBWriteContactSettingByte(0, MODULE, "FilterIn", options.filter_in ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "FilterOut", options.filter_out ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "BBCodes", options.bbcodes ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "HTML", options.html ? 1 : 0);

	DBWriteContactSettingDword(0, MODULE, "ApplyTo", options.apply_to);
}

INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HANDLE hItemAll;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		CheckDlgButton(hwndDlg, IDC_CHK_FILTERIN, options.filter_in);
		CheckDlgButton(hwndDlg, IDC_CHK_FILTEROUT, options.filter_out);
		CheckDlgButton(hwndDlg, IDC_CHK_BBCODES, options.bbcodes);
		CheckDlgButton(hwndDlg, IDC_CHK_HTML, options.html);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_BBCODES), IsDlgButtonChecked(hwndDlg, IDC_CHK_FILTERIN));
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_HTML), IsDlgButtonChecked(hwndDlg, IDC_CHK_FILTEROUT));

		CheckDlgButton(hwndDlg, IDC_CHK_ALL, options.apply_to & ATF_ALL);
		CheckDlgButton(hwndDlg, IDC_CHK_OTR, options.apply_to & ATF_OTR);
		EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_OTR), !IsDlgButtonChecked(hwndDlg, IDC_CHK_ALL));

		return FALSE;		
	case WM_COMMAND:
		if(HIWORD(wParam) == BN_CLICKED) {
			switch(LOWORD(wParam)) {
				case IDC_CHK_FILTERIN:
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_BBCODES), IsDlgButtonChecked(hwndDlg, IDC_CHK_FILTERIN));
					break;
				case IDC_CHK_FILTEROUT:
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_HTML), IsDlgButtonChecked(hwndDlg, IDC_CHK_FILTEROUT));
					break;
				case IDC_CHK_ALL:
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHK_OTR), !IsDlgButtonChecked(hwndDlg, IDC_CHK_ALL));
					break;
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
			case 0:
				switch (((LPNMHDR)lParam)->code)
				{
					case PSN_APPLY:
						options.filter_in = IsDlgButtonChecked(hwndDlg, IDC_CHK_FILTERIN);
						options.filter_out = IsDlgButtonChecked(hwndDlg, IDC_CHK_FILTEROUT);
						options.bbcodes = IsDlgButtonChecked(hwndDlg, IDC_CHK_BBCODES);
						options.html = IsDlgButtonChecked(hwndDlg, IDC_CHK_HTML);

						options.apply_to = (IsDlgButtonChecked(hwndDlg, IDC_CHK_ALL) ? ATF_ALL : 0);
						options.apply_to |= (IsDlgButtonChecked(hwndDlg, IDC_CHK_OTR) ? ATF_OTR : 0);
						
						SaveOptions();
						break;
				}
				break;
		}
		break;
	}

	return 0;
}

int OptInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize						= sizeof(odp);
	odp.flags						= ODPF_BOLDGROUPS;
	odp.position					= -790000000;
	odp.hInstance					= hInst;

	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT1);
	odp.pszTitle					= MODULE;
	odp.pszGroup					= "Plugins";
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

HANDLE hEventOptInit;
void InitOptions() {
	hEventOptInit = HookEvent(ME_OPT_INITIALISE, OptInit);
	LoadOptions();
}

void DeinitOptions() {
	UnhookEvent(hEventOptInit);
}
