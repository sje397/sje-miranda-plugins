#include "common.h"
#include "options.h"
#include "resource.h"
#include "DebugTools.h"
#include "str_utils.h"

#include "bzip2-1.0.3/bzlib.h"
extern "C" void compressStream ( FILE *stream, FILE *zStream );

/////////////////////////////
///// HTTP multipart POST upload support
#include "Uploader.h"
#include "UploadSettings.h"
#include "UploadResults.h"

Options options;

void DoTestUpload() {
	if(DBGetContactSettingByte(0, MODULE, "RealCrash", 0)) {
		int *i = 0;
		*i = 1;
	}

	// bzip the file
	char szDumpFileName[MAX_PATH], szBzipFilename[MAX_PATH];

	// bzip the plugin as a test file :)
	GetModuleFileNameA(hInst, szDumpFileName, MAX_PATH);

	strcpy(szBzipFilename, szDumpFileName);
	strcat(szBzipFilename, ".bz2");
	FILE *fin = fopen(szDumpFileName, "rb"), *fout = fopen(szBzipFilename, "w+b");
	if(fin) {
		if(fout) {
			compressStream(fin, fout);
		} else
			fclose(fin);
	}

	TCHAR *stzBzipFilename = a2t(szBzipFilename);

	TCHAR msg[1024];
	mir_sntprintf(msg, 1024, TranslateT("MiniDump crash dump created.\nCrash in module: %s\n\nUpload to miranda-im.org?"), MODULE, stzBzipFilename);

	// upload?
	if(options.upload && (!options.confirm_upload || MessageBox(0, msg, TranslateT("MiniDump - Crash Dump"), MB_YESNO | MB_ICONERROR) == IDYES)) {
		// yes...
		if(HttpUpload(stzBzipFilename, _T(MODULE " Test"), _T(MODULE " Test2"), _T("a.b.c.d"), _T("e.f.g.h"), 0, 0) && options.delete_uploaded)
			DeleteFile(stzBzipFilename);
	} else {
		mir_sntprintf(msg, 1024, TranslateT("Crash in module: %s\nDump file: %s"), MODULE, stzBzipFilename);
		MessageBox(0, msg, TranslateT("MiniDump - Crash Dump Created"), MB_OK | MB_ICONERROR);
	}

	free(stzBzipFilename);
	
}

static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
	
		CheckDlgButton(hwndDlg, IDC_CHK_CONFUPLOAD, options.confirm_upload ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_DELUPLOADED, options.delete_uploaded ? TRUE : FALSE);

		ShowWindow(GetDlgItem(hwndDlg, IDC_BTN_TEST), DBGetContactSettingByte(0, MODULE, "EnableTestButton", 0) ? SW_SHOWNOACTIVATE : SW_HIDE);
		SetDlgItemText(hwndDlg, IDC_ED_ID, options.reporter_id);

		return FALSE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			switch( LOWORD( wParam )) {
			case IDC_ED_ID:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}	
		} else
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			if(LOWORD(wParam) == IDC_BTN_TEST) {
				DoTestUpload();
			} else
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			options.confirm_upload = (IsDlgButtonChecked(hwndDlg, IDC_CHK_CONFUPLOAD) ? true : false);
			options.delete_uploaded = (IsDlgButtonChecked(hwndDlg, IDC_CHK_DELUPLOADED) ? true : false);
			GetDlgItemText(hwndDlg, IDC_ED_ID, options.reporter_id, 512);

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
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle					= Translate(MODULE);
	odp.pszGroup					= Translate("Services");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

void LoadOptions() {
	options.upload = (DBGetContactSettingByte(0, MODULE, "Upload", 1) == 1);
	options.confirm_upload = (DBGetContactSettingByte(0, MODULE, "ConfirmUpload", 1) == 1);
	options.delete_uploaded = (DBGetContactSettingByte(0, MODULE, "DeleteUploaded", 0) == 1);

	DBVARIANT dbv;
	if(!DBGetContactSettingTString(0, MODULE, "ReporterID", &dbv)) {
		_tcsncpy(options.reporter_id, dbv.ptszVal, 512);
		DBFreeVariant(&dbv);
	} else
		options.reporter_id[0] = 0;
}

void SaveOptions() {
	DBWriteContactSettingByte(0, MODULE, "Upload", options.upload ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "ConfirmUpload", options.confirm_upload ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "DeleteUploaded", options.delete_uploaded ? 1 : 0);

	DBWriteContactSettingTString(0, MODULE, "ReporterID", options.reporter_id);
}


