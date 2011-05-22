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
#include "options.h"

#define SHOW_PROGRESS_TIME		1000		// show progress window when backup time exceeds this many milliseconds

extern HANDLE hDbFile;
extern CRITICAL_SECTION csDbAccess;
extern HINSTANCE g_hInst;

extern Options options;

UINT timer_id = 0;

void Map();
void UnMap();

BOOL CALLBACK DlgProcProgress(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			HWND prog = GetDlgItem(hwndDlg, IDC_PROGRESS);

			TranslateDialogDefault( hwndDlg );
			
			SendMessage(prog, PBM_SETPOS, 0, 0);
		}
		break;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDCANCEL:
				// in the progress dialog, use the user data to indicate that the user has pressed cancel
				SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)1);
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

int RotateBackups(HWND progress_dialog, DWORD start_time) {
	char num1[4], num2[4];
	char backupfilename1[MAX_PATH], backupfilename2[MAX_PATH];
	int i;
	HWND prog = GetDlgItem(progress_dialog, IDC_PROGRESS);
	MSG msg;

	// delete the last file, so that the rest can be rename operations
	strcpy(backupfilename1, options.folder);
	strcat(backupfilename1, "\\db_backup");
	itoa(options.num_backups - 1, num1, 10);
	strcat(backupfilename1, num1);
	strcat(backupfilename1, ".dat");
	DeleteFileA(backupfilename1);

	for(i = options.num_backups - 1; i > 0 && GetWindowLong(progress_dialog, GWL_USERDATA) == 0; i--) {
		strcpy(backupfilename1, options.folder);
		strcat(backupfilename1, "\\db_backup");
		itoa(i - 1, num1, 10);
		strcat(backupfilename1, num1);
		strcat(backupfilename1, ".dat");

		strcpy(backupfilename2, options.folder);
		strcat(backupfilename2, "\\db_backup");
		itoa(i, num2, 10);
		strcat(backupfilename2, num2);
		strcat(backupfilename2, ".dat");

		MoveFileA(backupfilename1, backupfilename2);

		if(GetTickCount() - start_time >= SHOW_PROGRESS_TIME)
			ShowWindow(progress_dialog, SW_SHOW);

		while(PeekMessage(&msg, progress_dialog, 0, 0, PM_REMOVE) != 0) {
			if(!IsDialogMessage(progress_dialog, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		SendMessage(prog, PBM_SETPOS, (WPARAM)(int)(100  * (options.num_backups - i) / options.num_backups), 0);	
		UpdateWindow(progress_dialog);
		
	}

	return 0;
}

int Backup() {
	HANDLE hBackupFile;
	char backup_filename[MAX_PATH];
	char buff[8192];
	DWORD bytes_read, bytes_written, file_size, total_bytes_copied = 0;
	HWND progress_dialog = 0, prog;
	MSG msg;

	DWORD start_time = GetTickCount();

	// ensure the backup folder exists (either create it or return non-zero signifying error)
	if(!CreateDirectoryA(options.folder, 0)) {
		DWORD err = GetLastError();
		if(err != ERROR_ALREADY_EXISTS) {
			return 1;
		}
	}

	if(!options.disable_progress) {
		progress_dialog = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, DlgProcProgress);
		prog = GetDlgItem(progress_dialog, IDC_PROGRESS);
		SetDlgItemText(progress_dialog, IDC_PROGRESSMESSAGE, TranslateT("Rotating backup files..."));
	}


	RotateBackups(progress_dialog, start_time);

	strcpy(backup_filename, options.folder);
	strcat(backup_filename, "\\db_backup0.dat");

	hBackupFile = CreateFileA(backup_filename, GENERIC_WRITE, FILE_SHARE_WRITE, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if(hBackupFile != INVALID_HANDLE_VALUE) {
		if(!options.disable_progress) {
			SetDlgItemText(progress_dialog, IDC_PROGRESSMESSAGE, TranslateT("Copying database file..."));
			SendMessage(prog, PBM_SETPOS, (WPARAM)(int)(0), 0);	
			UpdateWindow(progress_dialog);
		}

		EnterCriticalSection(&csDbAccess);

		UnMap();
		
		file_size = GetFileSize(hDbFile, 0);

		if(file_size == 0) {
			Map();
			LeaveCriticalSection(&csDbAccess);
			CloseHandle(hBackupFile);
			DestroyWindow(progress_dialog);
			return 1;
		}

		SetFilePointer(hDbFile, 0, 0, FILE_BEGIN);

		while(ReadFile(hDbFile, buff, sizeof(buff), &bytes_read, 0) == TRUE && bytes_read > 0
			&& GetWindowLong(progress_dialog, GWL_USERDATA) == 0)
		{
			if(!WriteFile(hBackupFile, buff, bytes_read, &bytes_written, 0)) {
				Map();
				LeaveCriticalSection(&csDbAccess);
				CloseHandle(hBackupFile);
				DestroyWindow(progress_dialog);
				return 1;
			}
			total_bytes_copied += bytes_written;

			if(!options.disable_progress) {

				if(GetTickCount() - start_time >= SHOW_PROGRESS_TIME)
					ShowWindow(progress_dialog, SW_SHOW);


				while(PeekMessage(&msg, progress_dialog, 0, 0, PM_REMOVE) != 0) {
					if(!IsDialogMessage(progress_dialog, &msg)) {
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}

				SendMessage(prog, PBM_SETPOS, (WPARAM)(int)(100.0 * total_bytes_copied / file_size), 0);	
				UpdateWindow(progress_dialog);
			}
		}
		Map();
		LeaveCriticalSection(&csDbAccess);

		CloseHandle(hBackupFile);

		if(!options.disable_progress && GetWindowLong(progress_dialog, GWL_USERDATA) != 0)
			DeleteFileA(backup_filename);
		else
			DBWriteContactSettingDword(0, "db3x", "LastBackupTimestamp", (DWORD)time(0));

		if(!options.disable_progress)
			DestroyWindow(progress_dialog);

		return 0;
	}

	if(!options.disable_progress)
		DestroyWindow(progress_dialog);
	return 1;
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
	time_t t = time(0), diff = t - (time_t)DBGetContactSettingDword(0, "db3x", "LastBackupTimestamp", (DWORD)t);
	if(diff > (time_t)(options.period * (options.period_type == PT_HOURS ? 60 * 60 : 60 * 60 * 24)))
		Backup();
}

int SetBackupTimer(void) {

	if(options.backup_types & BT_PERIODIC) {
		if(timer_id == 0) {
			time_t t = time(0), diff = t - (time_t)DBGetContactSettingDword(0, "db3x", "LastBackupTimestamp", (DWORD)t);
			if(diff > (time_t)(options.period * (options.period_type == PT_HOURS ? 60 * 60 : 60 * 60 * 24)))
				Backup();

			timer_id = SetTimer(0, 0, 1000 * 60 * 60, TimerProc);
		}
	} else {
		if(timer_id != 0) {
			KillTimer(0, timer_id);
			timer_id = 0;
		}
	}


	return 0;
}

int BackupService(WPARAM wParam, LPARAM lParam) {
	return Backup();
}