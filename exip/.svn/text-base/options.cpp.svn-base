#include "options.h"

Options options;

// main ping options 
static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch ( msg ) {
	case WM_INITDIALOG: {
		TranslateDialogDefault( hwndDlg );

		{
			DBVARIANT dbv;
			if(!DBGetContactSetting(0, MODULE, "ExternalIP", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_IPADDRESS, dbv.pszVal);
				DBFreeVariant(&dbv);
			} else {
				SetDlgItemText(hwndDlg, IDC_IPADDRESS, Translate("Unknown"));
			}
		}

		return FALSE;
	}
	case WM_COMMAND:
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {

			SaveOptions();

			return TRUE;
		}
		break;
	}

	return FALSE;
}

int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
#define OPTIONPAGE_OLD_SIZE2 60
	//odp.cbSize						= sizeof(odp);
	odp.cbSize						= OPTIONPAGE_OLD_SIZE2;
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCE(IDD_OPT);
	odp.pszTitle					= Translate(MODULE);
	odp.pszGroup					= Translate("Plugins");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

void LoadOptions() {
}

void SaveOptions() {
}
