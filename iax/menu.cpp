#include "common.h"
#include "menu.h"
#include "icons.h"
#include "iax_interface.h"
#include "resource.h"
#include "utils.h"
#include "services.h"
#include "dial_dlg.h"

HANDLE hMenuAnswer, hMenuDial, hMenuCall, hMenuAddContact, hServiceAnswer, hServiceDial, hServiceCall, hServiceAddContact;

void EnableMainMenuItem(bool enable, HANDLE hMenu) {
	CLISTMENUITEM menu;
	ZeroMemory(&menu,sizeof(menu));
	menu.cbSize=sizeof(menu);

	menu.flags = CMIM_FLAGS | (enable ? 0 : CMIF_GRAYED);
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenu, (LPARAM)&menu);
}

void SetAnswerEnable(bool enable) {
	EnableMainMenuItem(enable, hMenuAnswer);
}

void SetDialEnable(bool enable) {
	EnableMainMenuItem(enable, hMenuDial);
}

int AnswerCallService(WPARAM wParam, LPARAM lParam) {
	AnswerCall(0);
	return 0;
}

int ShowDialWindowService(WPARAM wParam, LPARAM lParam) {
	ShowDialDialog(true);
	return 0;
}

int CallContactService(WPARAM wParam, LPARAM lParam) {
	HANDLE hContact = (HANDLE)wParam;

	DBVARIANT dbv;
	if(!DBGetContactSettingTString(hContact, MODULE, "Number", &dbv)) {
		Call(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	return 0;
}

static BOOL CALLBACK AddContactDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		SetFocus(GetDlgItem(hwndDlg, IDC_ED_NAME));
		return FALSE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED) {
			switch(LOWORD(wParam)) {
				case IDOK:
					{
						char name[256], number[256];
						if(GetDlgItemText(hwndDlg, IDC_ED_NAME, name, 256) && GetDlgItemText(hwndDlg, IDC_ED_NUM, number, 256))
							CreateContact(number, name, false);
						else {
							MessageBox(hwndDlg, Translate("You must enter a number and a name."), Translate("IAX Input Error"), MB_OK | MB_ICONWARNING);
							return TRUE;
						}
					}
					// drop through
				case IDCANCEL:
					SendMessage(hwndDlg, WM_CLOSE, 0, 0);
					return TRUE;
			}
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	}

	return 0;
}
int AddContactService(WPARAM wParam, LPARAM lParam) {
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDCONTACT), 0, AddContactDialogProc, 0); 
	return 0;
}

void InitMenu() {
	InitDialDialog();
	hServiceAnswer = CreateServiceFunction(MODULE "/AnswerCall", AnswerCallService);
	hServiceDial = CreateServiceFunction(MODULE "/DialNumber", ShowDialWindowService);
	hServiceCall = CreateServiceFunction(MODULE "/CallContact", CallContactService);
	hServiceAddContact = CreateServiceFunction(MODULE "/AddContact", AddContactService);

	CLISTMENUITEM menu;
	ZeroMemory(&menu,sizeof(menu));
	menu.cbSize=sizeof(menu);

	// main menu
	menu.flags = 0;
	menu.popupPosition = 500099900;
	menu.pszPopupName = Translate( MODULE );
	menu.cbSize = sizeof( menu );
	menu.position = 2000060000;
	menu.hIcon = hIconProto;
	menu.pszName = Translate( "Answer Call" );
	menu.pszService = MODULE "/AnswerCall";
	hMenuAnswer = (HANDLE)CallService( MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu );

	menu.flags = 0;
	menu.popupPosition = 500299901;
	menu.pszPopupName = Translate( MODULE );
	menu.cbSize = sizeof( menu );
	menu.position = 2000060001;
	menu.popupPosition = 0;
	menu.hIcon = hIconProto;
	menu.pszName = Translate( "Dial Number" );
	menu.pszService = MODULE "/DialNumber";
	hMenuDial = (HANDLE)CallService( MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu );

	menu.flags = 0;
	menu.popupPosition = 500299901;
	menu.pszPopupName = Translate( MODULE );
	menu.cbSize = sizeof( menu );
	menu.position = 2000060001;
	menu.popupPosition = 0;
	menu.hIcon = hIconProto;
	menu.pszName = Translate( "Add Contact" );
	menu.pszService = MODULE "/AddContact";
	hMenuAddContact = (HANDLE)CallService( MS_CLIST_ADDMAINMENUITEM, 0, (LPARAM)&menu );

	// contact menu
	menu.flags = CMIF_NOTOFFLINE;
	menu.popupPosition = 0;
	menu.cbSize = sizeof( menu );
	menu.position = 0;
	menu.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	menu.pszName = Translate( "Call" );
	menu.pszService = MODULE "/CallContact";
	menu.pszContactOwner = MODULE;
	hMenuCall = (HANDLE)CallService( MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&menu );
}

void DeinitMenu() {
	DeinitDialDialog();
	DestroyServiceFunction(hServiceAnswer);
	DestroyServiceFunction(hServiceDial);
	DestroyServiceFunction(hServiceCall);
	DestroyServiceFunction(hServiceAddContact);
}
