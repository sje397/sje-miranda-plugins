#include "common.h"
#include "import.h"
#include "core_functions.h"
#include "proto.h"
#include "resource.h"
#include <time.h>

void CreateNewMetas() {
	HANDLE hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0);
	HANDLE hMeta;
	while(hContact != NULL) {
		DWORD id = DBGetContactSettingDword(hContact, "MetaContacts", "MetaLink", (DWORD)-1);
		if(id != (DWORD)-1) {
			if(id > next_meta_id) next_meta_id = id + 1;
			hMeta = GetMetaHandle(id);
			if(hMeta) {
				Meta_Assign(hContact, hMeta);
			} else {
				hMeta = NewMetaContact();
				DBWriteContactSettingDword(hMeta, MODULE, META_ID, id);

				DBWriteContactSettingByte(hMeta, MODULE, "Default", 0);
				Meta_Assign(hContact, hMeta);

				DBVARIANT dbv;
				if(!DBGetContactSettingUTF8String(hContact, "CList", "Group", &dbv)) {
					DBWriteContactSettingUTF8String(hMeta, "CList", "Group", dbv.pszVal);
					DBFreeVariant(&dbv);
				}
				if(!DBGetContactSettingUTF8String(hContact, "CList", "MyHandle", &dbv)) {
					DBWriteContactSettingUTF8String(hMeta, "CList", "MyHandle", dbv.pszVal);
					DBFreeVariant(&dbv);
				}
				
				char *subProto = ContactProto(hContact);
				if(subProto) {
					if(!DBGetContactSettingUTF8String(hContact, subProto, "Nick", &dbv)) {
						DBWriteContactSettingUTF8String(hMeta, MODULE, "Nick", dbv.pszVal);
						DBFreeVariant(&dbv);
					}
				}
			}
		}

		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}	
}

INT_PTR CALLBACK DlgProcImport(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		CheckDlgButton(hwndDlg, IDC_RAD_HISTMETA, TRUE);
		break;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch(LOWORD(wParam)) {
				case IDOK:
					if(IsDlgButtonChecked(hwndDlg, IDC_RAD_HISTMETA))
						EndDialog(hwndDlg, IDC_RAD_HISTMETA);
					else if(IsDlgButtonChecked(hwndDlg, IDC_RAD_HISTSUB))
						EndDialog(hwndDlg, IDC_RAD_HISTSUB);
					else if(IsDlgButtonChecked(hwndDlg, IDC_RAD_HISTNONE))
						EndDialog(hwndDlg, IDC_RAD_HISTNONE);
					return TRUE;
				case IDCANCEL:
					EndDialog(hwndDlg, IDCANCEL);
					return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

#define WMU_SETPERCENT			(WM_USER + 0x100)
#define WMU_SETCONTACT			(WM_USER + 0x101)
INT_PTR CALLBACK DlgProcProgress(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		{			
			HWND prog = GetDlgItem(hwndDlg, IDC_PROG);
			SendMessage(prog, PBM_SETPOS, 0, 0);
		}
		break;
	case WMU_SETPERCENT:
		{
			int percent = (int)wParam;
			HWND prog = GetDlgItem(hwndDlg, IDC_PROG);
			SendMessage(prog, PBM_SETPOS, (WPARAM)percent, 0);	
			TCHAR buff[1024];
			mir_sntprintf(buff, 1024, TranslateT("History Copy - %d%%"), percent);
			SetWindowText(hwndDlg, buff);
		}
		return TRUE;
	case WMU_SETCONTACT:
		{
			HWND ed = GetDlgItem(hwndDlg, IDC_ED_MESSAGE);
			TCHAR buff[1024];
			mir_sntprintf(buff, 1024, TranslateT("Current contact: %s"), ContactName((HANDLE)wParam));
			SetWindowText(ed, buff);
		}
		return TRUE;
	}
	return FALSE;
}


// function to copy history from one contact to another - courtesy JdGordon (thx)
// days == 0 implies copy all history
void CopyHistory(HANDLE hContactFrom, HANDLE hContactTo, int days) { 
	HANDLE hDbEvent; 
	DBEVENTINFO dbei; 
	DWORD time_now = time(0);
	DWORD earliest_time = time_now - days * 24 * 60 * 60;
	BYTE *buffer = 0;

	if (!hContactFrom || !hContactTo) return; 

	CallService(MS_DB_SETSAFETYMODE, (WPARAM)FALSE, 0);
	for (hDbEvent = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST,(WPARAM)hContactFrom, 0); 
		hDbEvent; 
		hDbEvent=(HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hDbEvent,0)) 
	{ 
		// get the event 
		ZeroMemory(&dbei, sizeof(dbei)); 
		dbei.cbSize = sizeof(dbei); 

		if((dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hDbEvent, 0)) == -1)
			break;

		buffer = (PBYTE)mir_realloc(buffer, dbei.cbBlob);// + id_length);
		dbei.pBlob = buffer; 

		if (CallService(MS_DB_EVENT_GET,(WPARAM)hDbEvent,(LPARAM)&dbei)) 
			break; 

		// i.e. optoins.days_history == 0;
		if(time_now == earliest_time) earliest_time = dbei.timestamp;

		if(dbei.timestamp < earliest_time)
			continue;

		dbei.flags &= ~DBEF_FIRST;
		CallService(MS_DB_EVENT_ADD, (WPARAM)hContactTo, (LPARAM)&dbei); 
	}		
	if(buffer) mir_free(buffer);
	CallService(MS_DB_SETSAFETYMODE, (WPARAM)TRUE, 0);
}


void ImportMetaHistory() {
	HWND progress_dialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, DlgProcProgress);
	ShowWindow(progress_dialog, SW_SHOW);

	HWND prog = GetDlgItem(progress_dialog, IDC_PROG);

	HANDLE hMeta;
	int num_contacts = metaMap.size(), count = 0;
	HANDLE hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact != NULL) {
		DWORD id = DBGetContactSettingDword(hContact, "MetaContacts", "MetaID", (DWORD)-1);
		if(id != (DWORD)-1) {
			hMeta = GetMetaHandle(id);
			if(hMeta) {
				SendMessage(progress_dialog, WMU_SETPERCENT, (WPARAM)(int)(100.0 * count / num_contacts), 0);	
				SendMessage(progress_dialog, WMU_SETCONTACT, (WPARAM)hMeta, 0);	
				UpdateWindow(progress_dialog);
				CopyHistory(hContact, hMeta, 0);
				count++;
			}
		}


		hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}

	DestroyWindow(progress_dialog);
}

// import a metacontacts subcontact history, sorting all subcontact events according to timestamp
void ImportSubHistory() {
	HWND progress_dialog = CreateDialog(hInst, MAKEINTRESOURCE(IDD_COPYPROGRESS), 0, DlgProcProgress);
	ShowWindow(progress_dialog, SW_SHOW);

	HWND prog = GetDlgItem(progress_dialog, IDC_PROG);

	CallService(MS_DB_SETSAFETYMODE, (WPARAM)FALSE, 0);

	HANDLE *hEvent;
	DWORD *times, *sizes;
	int index;
	BYTE *buffer = 0;
	DBEVENTINFO dbei = {0}; 
	dbei.cbSize = sizeof(dbei);
	int subcount, num_contacts = metaMap.size(), count = 0;
	for(MetaMap::Iterator i = metaMap.start(); i.has_val(); i.next()) {
		SendMessage(progress_dialog, WMU_SETPERCENT, (WPARAM)(int)(100.0 * count / num_contacts), 0);	
		SendMessage(progress_dialog, WMU_SETCONTACT, (WPARAM)i.val().first.handle(), 0);	
		UpdateWindow(progress_dialog);

		hEvent = new HANDLE[i.val().second.size()];
		times = new DWORD[i.val().second.size()];
		sizes = new DWORD[i.val().second.size()];
		index = 0;
		subcount = i.val().second.size();
		for(SubcontactList::Iterator j = i.val().second.start(); j.has_val(); j.next()) {
			hEvent[index] = (HANDLE)CallService(MS_DB_EVENT_FINDFIRST,(WPARAM)j.val().handle(), 0);
			dbei.cbBlob = 0;
			if(hEvent[index] && !CallService(MS_DB_EVENT_GET,(WPARAM)hEvent[index],(LPARAM)&dbei)) {
				times[index] = dbei.timestamp;
				sizes[index] = dbei.cbBlob;
			} else
				hEvent[index] = 0;
			index++;
		}

		int min_index;
		do {
			min_index = -1;
			for(index = 0; index < subcount; index++) {
				if(hEvent[index] && (min_index == -1 || times[index] < times[min_index]))
					min_index = index;
			}

			if(min_index != -1) {
				// copy earliest event
				dbei.cbBlob = sizes[min_index];
				dbei.pBlob = (PBYTE)mir_realloc(buffer, dbei.cbBlob);
				if(!CallService(MS_DB_EVENT_GET,(WPARAM)hEvent[min_index],(LPARAM)&dbei)) {
					dbei.flags &= ~DBEF_FIRST;
					CallService(MS_DB_EVENT_ADD, (WPARAM)i.val().first.handle(), (LPARAM)&dbei); 
					
					// update arrays
					hEvent[min_index] = (HANDLE)CallService(MS_DB_EVENT_FINDNEXT,(WPARAM)hEvent[min_index], 0);
					if(hEvent[min_index] == 0 || (dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hEvent[min_index], 0)) == -1)
						hEvent[min_index] = 0;
					dbei.cbBlob = 0;
					if(hEvent[min_index] && !CallService(MS_DB_EVENT_GET,(WPARAM)hEvent[min_index],(LPARAM)&dbei)) {
						times[min_index] = dbei.timestamp;
						sizes[min_index] = dbei.cbBlob;
					} else
						hEvent[min_index] = 0;
				} else
					hEvent[min_index] = 0;
			}	
		} while(min_index != -1);

		delete[] hEvent;
		delete[] times;
		delete[] sizes;

		count++;
	}
	if(buffer) mir_free(buffer);
	CallService(MS_DB_SETSAFETYMODE, (WPARAM)TRUE, 0);
	DestroyWindow(progress_dialog);
}

void DeleteAllModuleContacts() {
	HANDLE hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0);
	HANDLE hMeta;
	while(hContact != NULL) {
		if(IsMetacontact(hContact)) {
			hMeta = hContact;
			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hMeta, 0);
		} else {
			// just to be safe
			DBDeleteContactSetting(hContact, MODULE, "ParentMetaID");
			DBWriteContactSettingDword(hContact, MODULE, "Handle", 0);
			DBWriteContactSettingByte(hContact, MODULE, "IsSubcontact", 0);

			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
		}
	}
}

void DeleteOldMetas() {
	HANDLE hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0);
	HANDLE hMeta;
	while(hContact != NULL) {
		if(DBGetContactSettingDword(hContact, "MetaContacts", "MetaID", (DWORD)-1) != (DWORD)-1) {
			hMeta = hContact;
			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hMeta, 0);
		} else
			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}
}

bool OldMetasExist() {
	HANDLE hContact = (HANDLE)CallService( MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact != NULL) {
		if(DBGetContactSettingDword(hContact, "MetaContacts", "MetaID", (DWORD)-1) != (DWORD)-1) {
			return true;
		} else
			hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
	}
	return false;
}

void ImportOldMetas() {
	if(OldMetasExist()) {
		DeleteAllModuleContacts();
		next_meta_id = 0;

		int ret;
		if((ret = DialogBox(hInst, MAKEINTRESOURCE(IDD_HISTIMPORT), 0, DlgProcImport)) != IDCANCEL) {
			CreateNewMetas();
			switch(ret) {
				case IDC_RAD_HISTMETA:
					ImportMetaHistory();
					break;
				case IDC_RAD_HISTSUB:
					ImportSubHistory();
					break;
			}
		}

		// reset ready for modules loaded event
		metaMap.clear();

		if(MessageBox(0, TranslateT("Do you wish to remove your old metacontacts from your profile?"), TranslateT("Delete Old MetaContacts"), MB_YESNO) == IDYES)
			DeleteOldMetas();
	}
}