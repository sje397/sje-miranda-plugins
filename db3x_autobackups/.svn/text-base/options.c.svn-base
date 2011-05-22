/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"
#include "../../include/m_utils.h"
#include "../../include/m_trigger.h"
#include <m_updater.h>

#include <shlobj.h>

#//include "database.h"
#include "options.h"

extern HINSTANCE g_hInst;
Options options;


#define SUB_DIR "\\AutoBackups"
int GetProfileDirectory(char *szPath,int cbPath) {
	return CallService(MS_DB_GETPROFILEPATH, (WPARAM)cbPath, (LPARAM)szPath);
}

int Backup(void);
int BackupService(WPARAM wParam, LPARAM lParam);
int SetBackupTimer(void);

int LoadOptions(void) {
	DBVARIANT dbv = {0};

	options.backup_types = (BackupType)DBGetContactSettingByte(0, "db3x", "BackupType", (BYTE)(BT_EXIT | BT_PERIODIC));
	options.period = (unsigned int)DBGetContactSettingWord(0, "db3x", "Period", 1);
	options.period_type = (PeriodType)DBGetContactSettingByte(0, "db3x", "PeriodType", (BYTE)PT_DAYS);

	if(!DBGetContactSettingString(0, "db3x", "Folder", &dbv) && dbv.type == DBVT_ASCIIZ) {
		if(strlen(dbv.pszVal) >= 2 && dbv.pszVal[1] == ':') {
			strncpy(options.folder, dbv.pszVal, MAX_PATH);
		} else {
			GetProfileDirectory(options.folder, MAX_PATH);
			strcat(options.folder, "\\");
			strcat(options.folder, dbv.pszVal);
		}
		DBFreeVariant(&dbv);
	} else {
		int len;
		GetProfileDirectory(options.folder, MAX_PATH);
		len = strlen(options.folder);
		strncat(options.folder, SUB_DIR, MAX_PATH - len);
	}

	options.num_backups = (unsigned int)DBGetContactSettingWord(0, "db3x", "NumBackups", 3);
	
	options.disable_progress = (BOOL)DBGetContactSettingByte(0, "db3x", "NoProgress", 0);

	SetBackupTimer();
	return 0;
}

int SaveOptions(void) {
	char prof_dir[MAX_PATH];
	int prof_len, opt_len;

	DBWriteContactSettingByte(0, "db3x", "BackupType", (BYTE)options.backup_types);
	DBWriteContactSettingWord(0, "db3x", "Period", (WORD)options.period);
	DBWriteContactSettingByte(0, "db3x", "PeriodType", (BYTE)options.period_type);

	GetProfileDirectory(prof_dir, MAX_PATH);
	strcat(prof_dir, "\\");
	prof_len = strlen(prof_dir);
	opt_len = strlen(options.folder);

	if(opt_len > prof_len && strncmp(options.folder, prof_dir, prof_len) == 0) {
		DBWriteContactSettingString(0, "db3x", "Folder", (options.folder + prof_len));
	} else 
		DBWriteContactSettingString(0, "db3x", "Folder", options.folder);

	DBWriteContactSettingWord(0, "db3x", "NumBackups", (WORD)options.num_backups);
	DBWriteContactSettingByte(0, "db3x", "NoProgress", (BYTE)options.disable_progress);

	SetBackupTimer();
	return 0;
}

Options new_options;

int SetDlgState(HWND hwndDlg) {
	HWND hwndCtrl;
	char buff[10];

	if(new_options.backup_types == BT_DISABLED) {
		CheckDlgButton(hwndDlg, IDC_RAD_DISABLED, BST_CHECKED);
		hwndCtrl = GetDlgItem(hwndDlg, IDC_RAD_DISABLED);
		EnableWindow(hwndCtrl, FALSE);

		CheckDlgButton(hwndDlg, IDC_RAD_START, BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_EXIT, BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_PERIODIC, BST_UNCHECKED);
		hwndCtrl = GetDlgItem(hwndDlg, IDC_ED_PERIOD);
		EnableWindow(hwndCtrl, FALSE);
		hwndCtrl = GetDlgItem(hwndDlg, IDC_RAD_DAYS);
		EnableWindow(hwndCtrl, FALSE);
		hwndCtrl = GetDlgItem(hwndDlg, IDC_RAD_HOURS);
		EnableWindow(hwndCtrl, FALSE);
	} else {
		hwndCtrl = GetDlgItem(hwndDlg, IDC_RAD_DISABLED);
		EnableWindow(hwndCtrl, TRUE);
		CheckDlgButton(hwndDlg, IDC_RAD_DISABLED, BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_START, new_options.backup_types & BT_START ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_EXIT, new_options.backup_types & BT_EXIT ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_PERIODIC, new_options.backup_types & BT_PERIODIC ? BST_CHECKED : BST_UNCHECKED);
		hwndCtrl = GetDlgItem(hwndDlg, IDC_ED_PERIOD);
		EnableWindow(hwndCtrl, new_options.backup_types & BT_PERIODIC);
		hwndCtrl = GetDlgItem(hwndDlg, IDC_RAD_DAYS);
		EnableWindow(hwndCtrl, new_options.backup_types & BT_PERIODIC);
		hwndCtrl = GetDlgItem(hwndDlg, IDC_RAD_HOURS);
		EnableWindow(hwndCtrl, new_options.backup_types & BT_PERIODIC);
	}

	SetDlgItemTextA(hwndDlg, IDC_ED_PERIOD, itoa(new_options.period, buff, 10));

	switch(new_options.period_type) {
	case PT_DAYS:
		CheckDlgButton(hwndDlg, IDC_RAD_DAYS, BST_CHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_HOURS, BST_UNCHECKED);
		break;
	case PT_HOURS:
		CheckDlgButton(hwndDlg, IDC_RAD_DAYS, BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_RAD_HOURS, BST_CHECKED);
		break;
	}

	SetDlgItemTextA(hwndDlg, IDC_ED_NUMBACKUPS, itoa(new_options.num_backups, buff, 10));

	SetDlgItemTextA(hwndDlg, IDC_ED_FOLDER, new_options.folder);

	CheckDlgButton(hwndDlg, IDC_CHK_NOPROG, new_options.disable_progress ? BST_CHECKED : BST_UNCHECKED);

	return 0;
}

BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char buff[10];
	char folder_buff[MAX_PATH];
	BROWSEINFOA bi;
	LPCITEMIDLIST pidl;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		memcpy(&new_options, &options, sizeof(Options));
		SetDlgState(hwndDlg);
		return TRUE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			switch( LOWORD( wParam )) {
			case IDC_ED_PERIOD:
			case IDC_ED_FOLDER:
			case IDC_ED_NUMBACKUPS:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			}	
		}
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_RAD_DISABLED:
				if(IsDlgButtonChecked(hwndDlg, IDC_RAD_DISABLED)) {
					new_options.backup_types = BT_DISABLED;
				}
				SetDlgState(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_START:
				if(IsDlgButtonChecked(hwndDlg, IDC_RAD_START))
					new_options.backup_types |= BT_START;
				else
					new_options.backup_types &= ~BT_START;
				SetDlgState(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_EXIT:
				if(IsDlgButtonChecked(hwndDlg, IDC_RAD_EXIT))
					new_options.backup_types |= BT_EXIT;
				else
					new_options.backup_types &= ~BT_EXIT;
				SetDlgState(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_PERIODIC:
				if(IsDlgButtonChecked(hwndDlg, IDC_RAD_PERIODIC))
					new_options.backup_types |= BT_PERIODIC;
				else
					new_options.backup_types &= ~BT_PERIODIC;
				SetDlgState(hwndDlg);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_DAYS:
				new_options.period_type = PT_DAYS;
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case IDC_RAD_HOURS:
				new_options.period_type = PT_HOURS;
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_BUT_BROWSE:
				bi.hwndOwner = hwndDlg;
				bi.pidlRoot = 0;
				bi.pszDisplayName = folder_buff;
				bi.lpszTitle = Translate("Select Backup Folder");
				bi.ulFlags = 0;
				bi.lpfn = 0;
				bi.lParam = 0;
				bi.iImage = 0;

				if((pidl = SHBrowseForFolderA(&bi)) != 0) {
					SHGetPathFromIDListA(pidl, folder_buff);
					SetDlgItemTextA(hwndDlg, IDC_ED_FOLDER, folder_buff);
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );

					CoTaskMemFree((void *)pidl);
				}
				break;
			case IDC_BUT_NOW:
				Backup();
				break;
			case IDC_CHK_NOPROG:
				new_options.disable_progress = IsDlgButtonChecked(hwndDlg, IDC_CHK_NOPROG);
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
		}

		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {
			GetDlgItemTextA(hwndDlg, IDC_ED_PERIOD, buff, sizeof(buff));
			new_options.period = atoi(buff);
			GetDlgItemTextA(hwndDlg, IDC_ED_NUMBACKUPS, buff, sizeof(buff));
			new_options.num_backups = atoi(buff);
			GetDlgItemTextA(hwndDlg, IDC_ED_FOLDER, folder_buff, MAX_PATH);

			{
				BOOL folder_ok = TRUE;
				if(!CreateDirectoryA(folder_buff, 0)) {
					DWORD err = GetLastError();
					if(err != ERROR_ALREADY_EXISTS) {
						char msg_buff[512];
						FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, err, 0, msg_buff, 512, 0);
						MessageBoxA(0, msg_buff, Translate("Error Creating Backup Folder"), MB_OK | MB_ICONERROR);
						folder_ok = FALSE;
					}
				}
				
				if(folder_ok) {
					strncpy(new_options.folder, folder_buff, MAX_PATH);
					memcpy(&options, &new_options, sizeof(Options));
					SaveOptions();
				} else {
					memcpy(&new_options, &options, sizeof(Options));
					SetDlgState(hwndDlg);
				}
			}
			return TRUE;

		}
		break;
	}
	
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = {0};
#define OPTIONPAGE_OLD_SIZE2 60
	//odp.cbSize						= sizeof(odp);
	odp.cbSize						= OPTIONPAGE_OLD_SIZE2;
	odp.position					= -790000000;
	odp.hInstance					= g_hInst;
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.pszTitle					= Translate("Database Backups");
	odp.pszGroup					= Translate("Services");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

int ModulesLoaded(WPARAM wParam, LPARAM lParam) {
	CLISTMENUITEM menu;

	HookEvent(ME_OPT_INITIALISE, OptInit);

	ZeroMemory(&menu,sizeof(menu));
	menu.cbSize=sizeof(menu);
	menu.flags = CMIM_ALL;
	
	menu.hIcon=LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_MENU));

	// main menu item
	menu.pszName = (char *)Translate("Backup Profile");
	menu.pszService = MS_DB_BACKUP;
	menu.position = 500100000;
	CallService(MS_CLIST_ADDMAINMENUITEM,0,(LPARAM)&menu);

	// register trigger action for triggerplugin
	if(ServiceExists(MS_TRIGGER_REGISTERACTION)) {
		// create update action for triggerplugin
		ACTIONREGISTER ar = {0};
		ar.cbSize = sizeof(ACTIONREGISTER);
		ar.pszName = Translate("Backup Database");
		ar.pszService = MS_DB_BACKUPTRGR;

		CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);
	}


	return 0;
}

// can't do this on unload, since other plugins will be have already been unloaded, but their hooks 
// for setting changed event not cleared. the backup on exit function will write to the db, calling those hooks.
int PreShutdown(WPARAM wParam, LPARAM lParam) {
	if(options.backup_types & BT_EXIT)
		Backup();

	return 0;
}

int BackupServiceTrgr(WPARAM wParam, LPARAM lParam) {
	if(wParam & ACT_PERFORM) {
		return CallService(MS_DB_BACKUP, 0, 0);
	}
	return 0;
}

int InitOptions(void) {

	OleInitialize(0);

	CreateServiceFunction(MS_DB_BACKUP, BackupService);
	CreateServiceFunction(MS_DB_BACKUPTRGR, BackupServiceTrgr);

	// we have to wait for the options module to load!
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);	

	HookEvent(ME_SYSTEM_PRESHUTDOWN, PreShutdown);	

	// but, our stuff has loaded so...
	LoadOptions();

	if(options.backup_types & BT_START)
		Backup();

	return 0;
}

int UninitOptions(void) {

	OleUninitialize();

	return 0;
}

