#include "options.h"

Options options;

static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		return FALSE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
					return TRUE;
			}
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			SaveOptions();
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
#define OPTIONPAGE_OLD_SIZE2 60
	//odp.cbSize						= sizeof(odp);
	odp.cbSize						= OPTIONPAGE_OLD_SIZE2;
	odp.pszTab						= 0;
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCE(IDD_OPTIONS);
	odp.pszTitle					= Translate(MODULE);
	odp.pszGroup					= Translate("Events");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts;
	//CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

void LoadOptions() {
}

void SaveOptions() {
}


