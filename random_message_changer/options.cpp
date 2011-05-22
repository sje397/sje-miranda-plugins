#include "common.h"
#include "options.h"

Options options;

#define WMU_SETBUTTONS		(WM_USER + 0x100)

void GetDBString(char *buff, int buffsize, char *setting, char *def) {
	DBVARIANT dbv;
	if(!DBGetContactSetting(NULL,RMC,setting,&dbv)) {
		strncpy(buff, dbv.pszVal, buffsize);
		DBFreeVariant(&dbv);
	} else
		strncpy(buff, def, STRING_LENGTH);
}

//Methode für den Optionsdialog
int CALLBACK MainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam){
	switch (uMsg){
		case WM_INITDIALOG:	
			// Die Dialogfelder vorbereiten:
			TranslateDialogDefault(hwndDlg);
			SetDlgItemInt(hwndDlg, IDC_Timer, options.iTimeOut, TRUE);
			CheckDlgButton(hwndDlg,IDC_cStatus,options.cStatus);
			CheckDlgButton(hwndDlg,IDC_cTime,options.cTime);

			CheckDlgButton(hwndDlg,IDC_cAway,options.cAway);
			CheckDlgButton(hwndDlg,IDC_cNA,options.cNa);
			CheckDlgButton(hwndDlg,IDC_cOcc,options.cOcc);
			CheckDlgButton(hwndDlg,IDC_cDnd,options.cDnd);
			CheckDlgButton(hwndDlg,IDC_cFfc,options.cFfc);

			SetDlgItemText(hwndDlg,IDC_file_away, options.file_away);
			SetDlgItemText(hwndDlg,IDC_file_na, options.file_na);
			SetDlgItemText(hwndDlg,IDC_file_occ, options.file_occ);
			SetDlgItemText(hwndDlg,IDC_file_dnd, options.file_dnd);
			SetDlgItemText(hwndDlg,IDC_file_ffc, options.file_ffc);
			
			SendMessage(hwndDlg, WMU_SETBUTTONS, 0, 0);			
			return TRUE;
		case WMU_SETBUTTONS:
			{
				bool en=(IsDlgButtonChecked(hwndDlg,IDC_cTime)==1)||(IsDlgButtonChecked(hwndDlg,IDC_cStatus)==1);
				EnableWindow(GetDlgItem(hwndDlg, IDC_file_away),(IsDlgButtonChecked(hwndDlg,IDC_cAway)==1)&&en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_file_na),(IsDlgButtonChecked(hwndDlg,IDC_cNA)==1)&&en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_file_occ),(IsDlgButtonChecked(hwndDlg,IDC_cOcc)==1)&&en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_file_dnd),(IsDlgButtonChecked(hwndDlg,IDC_cDnd)==1)&&en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_file_ffc),(IsDlgButtonChecked(hwndDlg,IDC_cFfc)==1)&&en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_Timer),IsDlgButtonChecked(hwndDlg,IDC_cTime)==1);

				EnableWindow(GetDlgItem(hwndDlg, IDC_cAway),en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_cNA),en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_cOcc),en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_cDnd),en);
				EnableWindow(GetDlgItem(hwndDlg, IDC_cFfc),en);
			}
			return TRUE;
		case WM_COMMAND:
			if ( HIWORD( wParam ) == BN_CLICKED ) {
				SendMessage(hwndDlg, WMU_SETBUTTONS, 0, 0);			
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			} else if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}

			break;
		case WM_NOTIFY:{
			NMHDR* nmhdr = (NMHDR*)lParam;
			switch (nmhdr->code){
				case PSN_APPLY:
					// DIe DIalogfelder auslesen, in die Variablen speichern und in die Datenbank schreiben
					options.iTimeOut = GetDlgItemInt(hwndDlg, IDC_Timer, NULL, TRUE);
					if (options.iTimeOut<1) {
						options.iTimeOut=1;
						SetDlgItemInt(hwndDlg, IDC_Timer, 1, TRUE);
					}
					DBWriteContactSettingDword(NULL, RMC, "TimeOut", options.iTimeOut);

					options.cStatus = IsDlgButtonChecked(hwndDlg,IDC_cStatus)==1;
					DBWriteContactSettingByte(NULL,RMC,"enable_status",(BYTE)options.cStatus);

					options.cTime = IsDlgButtonChecked(hwndDlg,IDC_cTime)==1;
					DBWriteContactSettingByte(NULL,RMC,"enable_time",(BYTE)options.cTime);

					options.cAway = IsDlgButtonChecked(hwndDlg,IDC_cAway)==1;
					DBWriteContactSettingByte(NULL,RMC,"enable_away",(BYTE)options.cAway);

					options.cNa = IsDlgButtonChecked(hwndDlg,IDC_cNA)==1;
					DBWriteContactSettingByte(NULL,RMC,"enable_na",(BYTE)options.cNa);

					options.cOcc = IsDlgButtonChecked(hwndDlg,IDC_cOcc)==1;
					DBWriteContactSettingByte(NULL,RMC,"enable_occ",(BYTE)options.cOcc);

					options.cDnd = IsDlgButtonChecked(hwndDlg,IDC_cDnd)==1;
					DBWriteContactSettingByte(NULL,RMC,"enable_dnd",(BYTE)options.cDnd);

					options.cFfc = IsDlgButtonChecked(hwndDlg,IDC_cFfc)==1;
					DBWriteContactSettingByte(NULL,RMC,"enable_ffc",(BYTE)options.cFfc);



					GetDlgItemText(hwndDlg,IDC_file_away,options.file_away,STRING_LENGTH);
					DBWriteContactSettingString(NULL,RMC,"file_away",options.file_away);

					GetDlgItemText(hwndDlg,IDC_file_na,options.file_na,STRING_LENGTH);
					DBWriteContactSettingString(NULL,RMC,"file_na",options.file_na);

					GetDlgItemText(hwndDlg,IDC_file_occ,options.file_occ,STRING_LENGTH);
					DBWriteContactSettingString(NULL,RMC,"file_occ",options.file_occ);

					GetDlgItemText(hwndDlg,IDC_file_dnd,options.file_dnd,STRING_LENGTH);
					DBWriteContactSettingString(NULL,RMC,"file_dnd",options.file_dnd);

					GetDlgItemText(hwndDlg,IDC_file_ffc,options.file_ffc,STRING_LENGTH);
					DBWriteContactSettingString(NULL,RMC,"file_ffc",options.file_ffc);

					//Timer starten
					SetChangingTime();
					return TRUE;
			
			}			
			break;
		
		}
	}

	return 0;
}

//Initialisierung des Options-Dialogs
int MessageOptInit(WPARAM wParam, LPARAM lParam){
	OPTIONSDIALOGPAGE odp;
	ZeroMemory(&odp, sizeof(odp));
	odp.cbSize      = sizeof(odp);
	odp.position    = 910000000;
	odp.hInstance   = hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_RMC);
	odp.pszTitle    = "Random Messages";
	odp.pszGroup    = "Status";
	odp.pfnDlgProc  = MainDlgProc;

	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);

	return 0;
}


HANDLE hEventOptInitialise = 0;
void LoadOptions() {
	options.iTimeOut = DBGetContactSettingDword(NULL, RMC, "TimeOut",  10);
	options.cStatus = DBGetContactSettingByte(NULL, RMC, "enable_status",  0)==1;
	options.cTime = DBGetContactSettingByte(NULL, RMC, "enable_time",  0)==1;

	options.cAway = DBGetContactSettingByte(NULL, RMC, "enable_away",  0)==1;
	options.cNa = DBGetContactSettingByte(NULL, RMC, "enable_na",  0)==1;
	options.cOcc = DBGetContactSettingByte(NULL, RMC, "enable_occ",  0)==1;
	options.cDnd = DBGetContactSettingByte(NULL, RMC, "enable_dnd",  0)==1;
	options.cFfc = DBGetContactSettingByte(NULL, RMC, "enable_ffc",  0)==1;

	GetDBString(options.file_away, STRING_LENGTH, "file_away", "plugins\\sample.txt");
	GetDBString(options.file_na, STRING_LENGTH, "file_na", "plugins\\sample.txt");
	GetDBString(options.file_occ, STRING_LENGTH, "file_occ", "plugins\\sample.txt");
	GetDBString(options.file_dnd, STRING_LENGTH, "file_dnd", "plugins\\sample.txt");
	GetDBString(options.file_ffc, STRING_LENGTH, "file_ffc", "plugins\\sample.txt");

	hEventOptInitialise = HookEvent(ME_OPT_INITIALISE, MessageOptInit);
}

