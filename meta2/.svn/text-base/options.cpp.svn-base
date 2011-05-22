#include "common.h"
#include "options.h"
#include "resource.h"

Options options;

void LoadOptions() {
	//options.dummy = DBGetContactSettingDword(0, MODULE, "Dummy", 0);
}

void SaveOptions() {
	//DBWriteContactSettingDword(0, MODULE, "Dummy", options.dummy);
}

INT_PTR CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HANDLE hItemAll;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		// TODO: read options variable into control states
		return FALSE;		
	case WM_COMMAND:
		// enable the 'apply' button
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
			case 0:
				switch (((LPNMHDR)lParam)->code)
				{
					case PSN_APPLY:
						// TODO: read control states into options variable
						SaveOptions();
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
	odp.flags |= ODPF_TCHAR;
	odp.position					= -790000000;
	odp.hInstance					= hInst;

	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT1);
	odp.ptszTitle					= TranslateT("MetaContacts");
	odp.ptszGroup					= TranslateT("Contact List");
	odp.ptszTab						= TranslateT("General");
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
