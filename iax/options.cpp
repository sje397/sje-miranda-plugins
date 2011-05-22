#include "common.h"
#include "options.h"
#include "resource.h"
#include "icons.h"
#include "dial_dlg.h"
#include "iaxclient/lib/iaxclient.h"

Options options = {0};

static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		SetDlgItemText(hwndDlg, IDC_ED_UNAME, options.username);
		SetDlgItemText(hwndDlg, IDC_ED_PW, options.password);
		SetDlgItemText(hwndDlg, IDC_ED_HOST, options.host);
		SetDlgItemText(hwndDlg, IDC_ED_CIDNAME, options.cid_name);
		SetDlgItemText(hwndDlg, IDC_ED_CIDNUM, options.cid_number);
		SetDlgItemInt(hwndDlg, IDC_ED_PORT, options.port, FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_POPDIALIN, options.pop_dial_in ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_POPDIALOUT, options.pop_dial_out ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_MICBOOST, options.mic_boost ? TRUE : FALSE);

		{
			iaxc_audio_device *dev;
			int nDevs, in, out, ring, ind;
			iaxc_audio_devices_get(&dev, &nDevs, &in, &out, &ring);
			for(int i = 0; i < nDevs; i++) {
				if(dev[i].capabilities & IAXC_AD_INPUT) {
					ind = SendDlgItemMessage(hwndDlg, IDC_CMB_DEVIN, CB_ADDSTRING, 0, (LPARAM)dev[i].name);
					SendDlgItemMessage(hwndDlg, IDC_CMB_DEVIN, CB_SETITEMDATA, ind, i);
					if(i == in) SendDlgItemMessage(hwndDlg, IDC_CMB_DEVIN, CB_SETCURSEL, ind, 0);
				}
				if(dev[i].capabilities & IAXC_AD_OUTPUT) {
					ind = SendDlgItemMessage(hwndDlg, IDC_CMB_DEVOUT, CB_ADDSTRING, 0, (LPARAM)dev[i].name);
					SendDlgItemMessage(hwndDlg, IDC_CMB_DEVOUT, CB_SETITEMDATA, ind, i);
					if(i == out) SendDlgItemMessage(hwndDlg, IDC_CMB_DEVOUT, CB_SETCURSEL, ind, 0);
				}
				/*
				if(dev[i].capabilities & IAXC_AD_RING) {
					ind = SendDlgItemMessage(hwndDlg, IDC_CMB_DEVRING, CB_ADDSTRING, 0, (LPARAM)dev[i].name);
					SendDlgItemMessage(hwndDlg, IDC_CMB_DEVRING, CB_SETITEMDATA, ind, i);
					if(i == ring) SendDlgItemMessage(hwndDlg, IDC_CMB_DEVRING, CB_SETCURSEL, ind, 0);
				}
				*/
			}
		}
		SendDlgItemMessage(hwndDlg, IDC_HK_DLG, HKM_SETHOTKEY, options.dlgHotkey, 0);

		return FALSE;		
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);		
		}
		if ( HIWORD( wParam ) == CBN_SELCHANGE) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);		
		}
		if ( HIWORD( wParam ) == BN_CLICKED) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);		
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
			case PSN_APPLY:
				{
					char buffer[512];

					GetDlgItemText(hwndDlg, IDC_ED_UNAME, buffer, 512);
					if(options.username) free(options.username);
					options.username = strdup(buffer);

					GetDlgItemText(hwndDlg, IDC_ED_PW, buffer, 512);
					if(options.password) free(options.password);
					options.password = strdup(buffer);

					GetDlgItemText(hwndDlg, IDC_ED_HOST, buffer, 512);
					if(options.host) free(options.host);
					options.host = strdup(buffer);

					GetDlgItemText(hwndDlg, IDC_ED_CIDNAME, buffer, 512);
					if(options.cid_name) free(options.cid_name);
					options.cid_name = strdup(buffer);

					GetDlgItemText(hwndDlg, IDC_ED_CIDNUM, buffer, 512);
					if(options.cid_number) free(options.cid_number);
					options.cid_number = strdup(buffer);

					DBWriteContactSettingTString(0, MODULE, "Host", options.host);
					DBWriteContactSettingTString(0, MODULE, "Username", options.username);
					DBWriteContactSettingTString(0, MODULE, "Password", options.password);
					DBWriteContactSettingTString(0, MODULE, "CIDName", options.cid_name);
					DBWriteContactSettingTString(0, MODULE, "CIDNumber", options.cid_number);

					iaxc_audio_device *dev;
					int nDevs, in, out, ring, ind;
					iaxc_audio_devices_get(&dev, &nDevs, &in, &out, &ring);

					ind = SendDlgItemMessage(hwndDlg, IDC_CMB_DEVIN, CB_GETCURSEL, 0, 0);
					in = SendDlgItemMessage(hwndDlg, IDC_CMB_DEVIN, CB_GETITEMDATA, ind, 0);
					ind = SendDlgItemMessage(hwndDlg, IDC_CMB_DEVOUT, CB_GETCURSEL, 0, 0);
					out = SendDlgItemMessage(hwndDlg, IDC_CMB_DEVOUT, CB_GETITEMDATA, ind, 0);
					//ind = SendDlgItemMessage(hwndDlg, IDC_CMB_DEVRING, CB_GETCURSEL, 0, 0);
					//ring = SendDlgItemMessage(hwndDlg, IDC_CMB_DEVRING, CB_GETITEMDATA, ind, 0);

					DBWriteContactSettingTString(0, MODULE, "DeviceIn", dev[in].name);
					DBWriteContactSettingTString(0, MODULE, "DeviceOut", dev[out].name);
					//DBWriteContactSettingTString(0, MODULE, "DeviceRing", dev[ring].name);

					BOOL trans;
					int port = GetDlgItemInt(hwndDlg, IDC_ED_PORT, &trans, FALSE);
					if(trans) options.port = port;
					DBWriteContactSettingDword(0, MODULE, "Port", options.port);

					iaxc_audio_devices_set(in, out, ring);

					options.dlgHotkey = (WORD)SendDlgItemMessage(hwndDlg, IDC_HK_DLG, HKM_GETHOTKEY, 0, 0);
					DBWriteContactSettingWord(0, MODULE, "DlgHotkey", options.dlgHotkey);
					ResetDlgHotkey();

					options.pop_dial_in = IsDlgButtonChecked(hwndDlg, IDC_CHK_POPDIALIN) ? true : false;
					options.pop_dial_out = IsDlgButtonChecked(hwndDlg, IDC_CHK_POPDIALOUT) ? true : false;
					DBWriteContactSettingByte(0, MODULE, "PopupDial", options.pop_dial_in ? 1 : 0);
					DBWriteContactSettingByte(0, MODULE, "PopupDialOut", options.pop_dial_out ? 1 : 0);

					options.mic_boost = IsDlgButtonChecked(hwndDlg, IDC_CHK_MICBOOST) ? true : false;
					DBWriteContactSettingByte(0, MODULE, "MicBoost", options.mic_boost ? 1 : 0);
					iaxc_mic_boost_set(options.mic_boost ? 1 : 0);
				}

				return TRUE;
		}
		break;
	}

	return 0;
}

static BOOL CALLBACK DlgProcOptsCodec(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		CheckDlgButton(hwndDlg, IDC_CHK_ULAW_P, options.codecs_preferred & IAXC_FORMAT_ULAW ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_ALAW_P, options.codecs_preferred & IAXC_FORMAT_ALAW ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_GSM_P, options.codecs_preferred & IAXC_FORMAT_GSM ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_SPEEX_P, options.codecs_preferred & IAXC_FORMAT_SPEEX ? TRUE : FALSE);

		CheckDlgButton(hwndDlg, IDC_CHK_ULAW, options.codecs_allowed & IAXC_FORMAT_ULAW ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_ALAW, options.codecs_allowed & IAXC_FORMAT_ALAW ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_GSM, options.codecs_allowed & IAXC_FORMAT_GSM ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_SPEEX, options.codecs_allowed & IAXC_FORMAT_SPEEX ? TRUE : FALSE);

#ifdef CODEC_ILBC
		CheckDlgButton(hwndDlg, IDC_CHK_ILBC_P, options.codecs_preferred & IAXC_FORMAT_ILBC ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_ILBC, options.codecs_allowed & IAXC_FORMAT_ILBC ? TRUE : FALSE);
#else
		{
			HWND hw = GetDlgItem(hwndDlg, IDC_CHK_ILBC);
			EnableWindow(hw, FALSE);
			hw = GetDlgItem(hwndDlg, IDC_CHK_ILBC_P);
			EnableWindow(hw, FALSE);
		}
#endif
		return FALSE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);		
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				{
					options.codecs_preferred = options.codecs_allowed = 0;

					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_ULAW_P)) options.codecs_preferred |= IAXC_FORMAT_ULAW;
					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_ALAW_P)) options.codecs_preferred |= IAXC_FORMAT_ALAW;
					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_GSM_P)) options.codecs_preferred |= IAXC_FORMAT_GSM;
					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_SPEEX_P)) options.codecs_preferred |= IAXC_FORMAT_SPEEX;

					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_ULAW)) options.codecs_allowed |= IAXC_FORMAT_ULAW;
					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_ALAW)) options.codecs_allowed |= IAXC_FORMAT_ALAW;
					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_GSM)) options.codecs_allowed |= IAXC_FORMAT_GSM;
					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_SPEEX)) options.codecs_allowed |= IAXC_FORMAT_SPEEX;

#ifdef CODEC_ILBC
					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_ILBC_P)) options.codecs_preferred |= IAXC_FORMAT_ILBC;
					if(IsDlgButtonChecked(hwndDlg, IDC_CHK_ILBC)) options.codecs_allowed |= IAXC_FORMAT_ILBC;
#endif

					DBWriteContactSettingDword(0, MODULE, "CodecsPreferred", options.codecs_preferred);
					DBWriteContactSettingDword(0, MODULE, "CodecsAllowed", options.codecs_allowed);

					iaxc_set_formats(options.codecs_preferred, options.codecs_allowed);
				}
				return TRUE;
		}
		break;
	}

	return 0;
}

int OptInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize						= sizeof(odp);
	odp.flags						= ODPF_BOLDGROUPS;
	//odp.flags |= ODPF_UNICODE;
	odp.position					= -790000000;
	odp.hInstance					= hInst;

	odp.pszTemplate				= MAKEINTRESOURCEA(IDD_OPT);
	odp.pszTitle					= Translate("IAX");
	odp.pszGroup					= Translate("Network");
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts;
	odp.pszTab						= Translate("Configuration");
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	odp.pszTemplate				= MAKEINTRESOURCEA(IDD_OPT_CODEC);
	odp.pszTitle					= Translate("IAX");
	odp.pszGroup					= Translate("Network");
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOptsCodec;
	odp.pszTab						= Translate("Codecs");
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

void PreInitOptions() {
	options.codecs_preferred = DBGetContactSettingDword(0, MODULE, "CodecsPreferred", IAXC_FORMAT_SPEEX);
	options.codecs_allowed = DBGetContactSettingDword(0, MODULE, "CodecsAllowed", IAXC_FORMAT_ILBC | IAXC_FORMAT_ULAW | IAXC_FORMAT_ALAW | IAXC_FORMAT_GSM | IAXC_FORMAT_SPEEX);
}

HANDLE hEventOptInit;
void InitOptions() {
	hEventOptInit = HookEvent(ME_OPT_INITIALISE, OptInit);
	DBVARIANT dbv;
	if(!DBGetContactSettingTString(0, MODULE, "Host", &dbv)) { 
		options.host = strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(0, MODULE, "Username", &dbv)) {
		options.username = strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(0, MODULE, "Password", &dbv)) {
		options.password = strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(0, MODULE, "CIDName", &dbv)) {
		options.cid_name = strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(0, MODULE, "CIDNumber", &dbv)) {
		options.cid_number = strdup(dbv.pszVal);
		DBFreeVariant(&dbv);
	}

	iaxc_audio_device *dev;
	int nDevs, in, out, ring;
	iaxc_audio_devices_get(&dev, &nDevs, &in, &out, &ring);
	if(!DBGetContactSettingTString(0, MODULE, "DeviceIn", &dbv)) {
		for(int i = 0; i < nDevs; i++) {
			if((dev[i].capabilities & IAXC_AD_INPUT) && strcmp(dbv.pszVal, dev[i].name) == 0) in = i;
		}
		DBFreeVariant(&dbv);
	}
	if(!DBGetContactSettingTString(0, MODULE, "DeviceOut", &dbv)) {
		for(int i = 0; i < nDevs; i++) {
			if((dev[i].capabilities & IAXC_AD_OUTPUT) && strcmp(dbv.pszVal, dev[i].name) == 0) out = i;
		}
		DBFreeVariant(&dbv);
	}
	/*
	if(!DBGetContactSettingTString(0, MODULE, "DeviceRing", &dbv)) {
		for(int i = 0; i < nDevs; i++) {
			if((dev[i].capabilities & IAXC_AD_RING) && strcmp(dbv.pszVal, dev[i].name) == 0) ring = i;
		}
		DBFreeVariant(&dbv);
	}
	*/

	options.port = DBGetContactSettingDword(0, MODULE, "Port", 4569);
	iaxc_audio_devices_set(in, out, ring);

	options.dlgHotkey = DBGetContactSettingWord(0, MODULE, "DlgHotkey", 0);
	options.pop_dial_in = (DBGetContactSettingByte(0, MODULE, "PopupDial", 1) == 1);
	options.pop_dial_out = (DBGetContactSettingByte(0, MODULE, "PopupDialOut", 0) == 1);
	
	options.mic_boost = (DBGetContactSettingByte(0, MODULE, "MicBoost", 0) == 1);
	iaxc_mic_boost_set(options.mic_boost ? 1 : 0);
	//options.mic_boost = (iaxc_mic_boost_get() != 0);
}

void DeinitOptions() {
	UnhookEvent(hEventOptInit);
	if(options.host) free(options.host);
	if(options.password) free(options.password);
	if(options.username) free(options.username);
	if(options.cid_name) free(options.cid_name);
	if(options.cid_number) free(options.cid_number);
}
