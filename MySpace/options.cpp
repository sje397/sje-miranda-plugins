#include "common.h"
#include "options.h"
#include "resource.h"
#include "links.h"
#include "menu.h"

Options options = {0};

#define URL_NEW_ACCOUNT		"http://signup.myspace.com/index.cfm?fuseaction=join"

void LoadOptions() {
	DBVARIANT dbv;
	if(!DBGetContactSettingTString(0, MODULE, "email", &dbv)) {
		_tcsncpy(options.email, dbv.ptszVal, 256);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingStringUtf(0, MODULE, "password", &dbv)) {
		CallService(MS_DB_CRYPT_DECODESTRING, (WPARAM)strlen(dbv.pszVal), (LPARAM)dbv.pszVal);
#ifdef _UNICODE
		MultiByteToWideChar(CP_UTF8, 0, dbv.pszVal, -1, options.pw, 256);
#else
		_tcsncpy(options.pw, dbv.pszVal, 256);
#endif
		DBFreeVariant(&dbv);
	} else
	if(!DBGetContactSettingTString(0, MODULE, "pw", &dbv)) { 
		// remove old cleartext password and convert to encrypted form
		_tcsncpy(options.pw, dbv.ptszVal, 256);
		DBFreeVariant(&dbv);

		char buff[256];
#ifdef _UNICODE
		WideCharToMultiByte(CP_UTF8, 0, options.pw, -1, buff, 256, 0, 0);
#else
		_tcsncpy(buff, options.pw, 256);
#endif
		CallService(MS_DB_CRYPT_ENCODESTRING, (WPARAM)256, (LPARAM)buff);
		DBWriteContactSettingStringUtf(0, MODULE, "password", buff);

		DBDeleteContactSetting(0, MODULE, "pw");
	}

	options.sound = (DBGetContactSettingByte(0, MODULE, "sound", 0) != 0);
	options.privacy_mode = (PrivacyMode)DBGetContactSettingByte(0, MODULE, "privacy_mode", PM_ANYONE);
	options.show_only_to_list = (DBGetContactSettingByte(0, MODULE, "show_only_to_list", 0) != 0);
	options.offline_message_mode = (OfflineMessageMode)DBGetContactSettingByte(0, MODULE, "offline_message_mode", OMM_CLIST);;
	options.show_avatar = (DBGetContactSettingByte(0, MODULE, "show_avatar", 0) != 0);
	if(!DBGetContactSettingStringUtf(0, MODULE, "Nick", &dbv)) {
		strncpy(options.im_name, dbv.pszVal, 256);
		DBFreeVariant(&dbv);
	}
	options.handle_links = (DBGetContactSettingByte(0, MODULE, "EnableLinkHandling", 0) != 0);
}

void SaveOptions() {
	DBWriteContactSettingTString(0, MODULE, "email", options.email);
	
	//DBWriteContactSettingTString(0, MODULE, "pw", options.pw);
	char buff[256];
#ifdef _UNICODE
		WideCharToMultiByte(CP_UTF8, 0, options.pw, -1, buff, 256, 0, 0);
#else
		_tcsncpy(buff, options.pw, 256);
#endif
	CallService(MS_DB_CRYPT_ENCODESTRING, (WPARAM)256, (LPARAM)buff);
	DBWriteContactSettingStringUtf(0, MODULE, "password", buff);

	DBWriteContactSettingByte(0, MODULE, "sound", options.sound ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "privacy_mode", (BYTE)options.privacy_mode);
	DBWriteContactSettingByte(0, MODULE, "show_only_to_list", options.show_only_to_list ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "offline_message_mode", (BYTE)options.offline_message_mode);
	DBWriteContactSettingByte(0, MODULE, "show_avatar", options.show_avatar ? 1 : 0);
	DBWriteContactSettingStringUtf(0, MODULE, "Nick", options.im_name);
	DBWriteContactSettingByte(0, MODULE, "EnableLinkHandling", options.handle_links ? 1 : 0);
}

BOOL CALLBACK DlgProcOpt1(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	//static HANDLE hItemAll;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SetDlgItemText(hwndDlg, IDC_ED_EMAIL, options.email);
		SetDlgItemText(hwndDlg, IDC_ED_PW, options.pw);
		CheckDlgButton(hwndDlg, IDC_CHK_LINKS, options.handle_links ? TRUE : FALSE);
		return FALSE;		
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) 
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		else if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus())
			SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
			case 0:
				switch (((LPNMHDR)lParam)->code)
				{
					case PSN_APPLY:
						GetDlgItemText(hwndDlg, IDC_ED_EMAIL, options.email, 256);
						GetDlgItemText(hwndDlg, IDC_ED_PW, options.pw, 256);
						options.handle_links = IsDlgButtonChecked(hwndDlg, IDC_CHK_LINKS) ? true : false;

						SaveOptions();

						if(options.handle_links) {
							myspace_links_init();
						} else {
							myspace_links_destroy();
							myspace_links_unregister();
						}
						EnableMainAdd(options.handle_links);
				}
				break;
		}
		break;
	}

	return 0;
}

BOOL CALLBACK DlgProcOptNotify(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	//static HANDLE hItemAll;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		CheckDlgButton(hwndDlg, IDC_CHK_NOTIFYMAIL, DBGetContactSettingByte(0, MODULE, "NotifyMail", 1));
		CheckDlgButton(hwndDlg, IDC_CHK_NOTIFYBLOGCOMMENTS, DBGetContactSettingByte(0, MODULE, "NotifyBlogComment", 1));
		CheckDlgButton(hwndDlg, IDC_CHK_NOTIFYPROFILECOMMENTS, DBGetContactSettingByte(0, MODULE, "NotifyProfileComment", 1));
		CheckDlgButton(hwndDlg, IDC_CHK_NOTIFYFRIENDREQUESTS, DBGetContactSettingByte(0, MODULE, "NotifyFriendRequest", 1));
		CheckDlgButton(hwndDlg, IDC_CHK_NOTIFYPICTURECOMMENTS, DBGetContactSettingByte(0, MODULE, "NotifyPicComment", 1));
		CheckDlgButton(hwndDlg, IDC_CHK_NOTIFYBLOGPOSTS, DBGetContactSettingByte(0, MODULE, "NotifyBlogSubscription", 1));
		CheckDlgButton(hwndDlg, IDC_CHK_NOTIFYUNKNOWN, DBGetContactSettingByte(0, MODULE, "NotifyUnknown", 1));
		return FALSE;		
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) 
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
			case 0:
				switch (((LPNMHDR)lParam)->code)
				{
					case PSN_APPLY:
						DBWriteContactSettingByte(0, MODULE, "NotifyMail", IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTIFYMAIL) ? 1 : 0);
						DBWriteContactSettingByte(0, MODULE, "NotifyBlogComment", IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTIFYBLOGCOMMENTS) ? 1 : 0);
						DBWriteContactSettingByte(0, MODULE, "NotifyProfileComment", IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTIFYPROFILECOMMENTS) ? 1 : 0);
						DBWriteContactSettingByte(0, MODULE, "NotifyFriendRequest", IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTIFYFRIENDREQUESTS) ? 1 : 0);
						DBWriteContactSettingByte(0, MODULE, "NotifyPicComment", IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTIFYPICTURECOMMENTS) ? 1 : 0);
						DBWriteContactSettingByte(0, MODULE, "NotifyBlogSubscription", IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTIFYBLOGPOSTS) ? 1 : 0);
						DBWriteContactSettingByte(0, MODULE, "NotifyUnknown", IsDlgButtonChecked(hwndDlg, IDC_CHK_NOTIFYUNKNOWN) ? 1 : 0);
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
	odp.pszTitle					= Translate(MODULE);
	odp.pszGroup					= Translate("Network");

	odp.pszTab						= Translate("Account");
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT1);
	odp.pfnDlgProc					= DlgProcOpt1;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );
	odp.pszTab						= Translate("Notifications");
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPTNOTIFY);
	odp.pfnDlgProc					= DlgProcOptNotify;
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
