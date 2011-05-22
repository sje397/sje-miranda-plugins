#include "common.h"
#include "dial_dlg.h"
#include "resource.h"
#include "utils.h"
#include "options.h"
#include "iax_interface.h"

HWND hWnd = 0;
ATOM hkid;

#define WMU_REFRESHLINEINFO		(WM_USER + 0x10)
#define WMU_SETLEVELS			(WM_USER + 0x11)

bool first_show = true;

HBRUSH hBrushActive, hBrushFree, hBrushActiveSelect, hBrushFreeSelect, hBrushRingIn, hBrushRingOut, hBrushRingInSelect, hBrushRingOutSelect;

struct CallHist {
	char num[256];
	char name[256];
	CallHist *next;
};

CallHist *hist = 0;

void list_insert(char *buff, char *name_buff) {
	bool in_list = false;
	CallHist *node = hist;
	while(node && !in_list) {
		if(strcmp(node->num, buff) == 0) {
			in_list = true;
			break;
		}
		node = node->next;
	}
	if(!in_list) {
		node = (CallHist *)malloc(sizeof(CallHist));
		node->next = hist;
		strncpy(node->num, buff, 256);
		strncpy(node->name, name_buff, 256);
		hist = node;
	} else {
		if(name_buff[0])
			strncpy(node->name, name_buff, 256);
	}
}

static BOOL CALLBACK DlgProcDial(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch ( msg ) {
	case WM_INITDIALOG:
		hWnd = hwndDlg;

		TranslateDialogDefault( hwndDlg );
		{
			CallHist *node = hist;
			while(node) {
				SendDlgItemMessage(hwndDlg, IDC_CMB_NUM, CB_ADDSTRING, 0, (LPARAM)node->num);
				node = node->next;
			}
		}
		SendMessage(hwndDlg, WMU_REFRESHLINEINFO, 0, 0);
		SendMessage(hwndDlg, WMU_SETLEVELS, 0, 0);

		SendDlgItemMessage(hwndDlg, IDC_SLD_IN, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)(int)(iaxc_input_level_get() * 100));
		SendDlgItemMessage(hwndDlg, IDC_SLD_OUT, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)(int)(iaxc_output_level_get() * 100));

		ResetDlgHotkey();

		SetFocus(GetDlgItem(hwndDlg, IDC_CMB_NUM));
		return TRUE;

	case WM_DRAWITEM:
		{
			bool active, select, ringing, outgoing;
			int line_no = 0;
			if(wParam == IDC_L1) line_no = 0;
			else if(wParam == IDC_L2) line_no = 1;
			else if(wParam == IDC_L3) line_no = 2;
			else break;

			ringing = (GetLineState(line_no) & IAXC_CALL_STATE_RINGING) != 0;
			outgoing = (GetLineState(line_no) & IAXC_CALL_STATE_OUTGOING) != 0;
			active = (GetLineState(line_no) & IAXC_CALL_STATE_ACTIVE) != 0;
			select = (GetLineNo() == line_no);

			HBRUSH bgBrush = 0;
			if(ringing) {
				if(outgoing) {
					if(select) bgBrush = hBrushRingOutSelect;
					else bgBrush = hBrushRingOut;
				} else {
					if(select) bgBrush = hBrushRingInSelect;
					else bgBrush = hBrushRingIn;
				}
			} else {
				if(active) {
					if(select) bgBrush = hBrushActiveSelect;
					else bgBrush = hBrushActive;
				} else {
					if(select) bgBrush = hBrushFreeSelect;
					else bgBrush = hBrushFree;
				}
			}

			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)lParam;
			//FillRect(dis->hDC, &dis->rcItem, bgBrush);
			HBRUSH oldBrush = (HBRUSH)SelectObject(dis->hDC, bgBrush);
			RoundRect(dis->hDC, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom, 3, 3);
			SelectObject(dis->hDC, oldBrush);

			char buff[2];
			GetWindowText(GetDlgItem(hwndDlg, wParam), buff, 2);
			SetBkMode(dis->hDC, TRANSPARENT);
			DrawText(dis->hDC, buff, 1, &dis->rcItem, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

			return TRUE;

		}
		break;

	case WM_SHOWWINDOW:
		if(wParam && first_show) {
			first_show = false;

			SAVEWINDOWPOS swp;
			swp.hwnd=hwndDlg; swp.hContact=0; swp.szModule=MODULE; swp.szNamePrefix="CallWindow";
			CallService(MS_UTILS_RESTOREWINDOWPOSITION, RWPF_NOSIZE | RWPF_NOACTIVATE, (LPARAM)&swp);
		}
		break;

	case WM_HOTKEY:
		if(IsWindowVisible(hwndDlg))
			ShowWindow(hwndDlg, SW_HIDE);
		else {
			ShowWindow(hwndDlg, SW_SHOW);
			SetForegroundWindow(hWnd);
		}
		return TRUE;
	case WM_HSCROLL:
		//if (LOWORD(wParam) == TB_PAGEUP || LOWORD(wParam) == TB_PAGEDOWN) 
		{
			int input = SendDlgItemMessage(hwndDlg, IDC_SLD_IN, TBM_GETPOS, 0, 0),
				output = SendDlgItemMessage(hwndDlg, IDC_SLD_OUT, TBM_GETPOS, 0, 0);

			iaxc_input_level_set(input / 100.0f);
			iaxc_output_level_set(output / 100.0f);
		}
		break;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == CBN_SELCHANGE) {
			SetDlgItemText(hwndDlg, IDC_ED_NAME, "");
		} else if ( HIWORD( wParam ) == BN_CLICKED ) {
			char c = 0;
			switch( LOWORD( wParam )) {
				case IDC_BTN_CALL:
					{
						char buff[256];
						GetDlgItemText(hwndDlg, IDC_CMB_NUM, buff, 256);
						if(buff[0]) {
							Call(buff);
							SetFocus(GetDlgItem(hwndDlg, IDC_CMB_NUM));
							SendMessage(hwndDlg, WMU_REFRESHLINEINFO, 0, 0);
						}
					}
					return TRUE;

				case IDC_BTN_HANGUP:
					{
						HangUp();
						SetFocus(GetDlgItem(hwndDlg, IDC_CMB_NUM));
					}
					return TRUE;

				case IDC_BTN_ANSWER:
					{
						AnswerCall(GetLineNo());
						SetFocus(GetDlgItem(hwndDlg, IDC_CMB_NUM));
					}
					return TRUE;

				case IDC_L1: SelectLine(0); return TRUE;
				case IDC_L2: SelectLine(1); return TRUE;
				case IDC_L3: SelectLine(2); return TRUE;

				case IDC_B0: c = '0'; break;
				case IDC_B1: c = '1'; break;
				case IDC_B2: c = '2'; break;
				case IDC_B3: c = '3'; break;
				case IDC_B4: c = '4'; break;
				case IDC_B5: c = '5'; break;
				case IDC_B6: c = '6'; break;
				case IDC_B7: c = '7'; break;
				case IDC_B8: c = '8'; break;
				case IDC_B9: c = '9'; break;
				case IDC_BSTAR: c = '*'; break;
				case IDC_BHASH: c = '#'; break;
			}

			SkinPlaySound("iax_keypad");

			char buff[512];
			GetDlgItemText(hwndDlg, IDC_CMB_NUM, buff, 512);
			if(strlen(buff) < 511) {
				strncat(buff, &c, 1);
				SetDlgItemText(hwndDlg, IDC_CMB_NUM, buff);
			}

			SendDTMF(c);
		}
		break;
	case WMU_REFRESHLINEINFO:
		{
			char *num = GetLineCIDNum(GetLineNo()), *name = GetLineCIDName(GetLineNo());
			// trim prefix from name if present
			char prefix[512];
			mir_snprintf(prefix, 512, "%s:%s@%s", options.username, options.password, options.host);
			int prefix_len = strlen(prefix);
			if(strncmp(name, prefix, prefix_len) == 0)
				name += prefix_len + 1;
			SetDlgItemText(hwndDlg, IDC_CMB_NUM, num);
			SetDlgItemText(hwndDlg, IDC_ED_NAME, name);
			bool active = (GetLineState(GetLineNo()) & IAXC_CALL_STATE_ACTIVE) != 0;
			
			if(active && num[0]) list_insert(num, name);

			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_CALL), !active);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_HANGUP), active);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_ANSWER), (GetLineState(GetLineNo()) & IAXC_CALL_STATE_RINGING) && !(GetLineState(GetLineNo()) & IAXC_CALL_STATE_OUTGOING));
			InvalidateRect(hwndDlg, 0, FALSE);
		}
		return TRUE;
	case WMU_SETLEVELS:
		{
			int in = wParam, out = lParam;
			SendDlgItemMessage(hwndDlg, IDC_PRG_IN, PBM_SETPOS, in, 0);
			SendDlgItemMessage(hwndDlg, IDC_PRG_OUT, PBM_SETPOS, out, 0);
		}
		return TRUE;
	case WM_CLOSE:
		ShowWindow(hwndDlg, SW_HIDE);
		return TRUE;
	case WM_DESTROY:
		Utils_SaveWindowPosition(hwndDlg,0,MODULE,"CallWindow");
		hWnd = 0;
		break;
	}

	return 0;
}

void ShowDialDialog(bool activate) {
	ShowWindow(hWnd, SW_SHOWNORMAL);
	if(activate) SetForegroundWindow(hWnd);
}

void SetLevels(float in, float out) {
	if(!hWnd) return;

	PostMessage(hWnd, WMU_SETLEVELS, (WPARAM)(int)(99 + in), (LPARAM)(int)(99 + out));
}

void RefreshLineInfo() {
	if(hWnd) PostMessage(hWnd, WMU_REFRESHLINEINFO, 0, 0);
}

void ResetDlgHotkey() {
	//PostMessage(hWnd, WM_SETHOTKEY, options.dlgHotkey, 0);
	WORD vk = LOBYTE(options.dlgHotkey);
	WORD hotkey_f = HIBYTE(options.dlgHotkey),
		mod = 0;
	
	if(hotkey_f & HOTKEYF_ALT) mod |= MOD_ALT;
	if(hotkey_f & HOTKEYF_CONTROL) mod |= MOD_CONTROL;
	if(hotkey_f & HOTKEYF_EXT) mod |= MOD_WIN;
	if(hotkey_f & HOTKEYF_SHIFT) mod |= MOD_SHIFT;

	UnregisterHotKey(hWnd, hkid);
	if(!RegisterHotKey(hWnd, hkid, mod, vk)) {
		ShowWarning("Failed to register hotkey");
	}
}

void InitDialDialog() {
	char buff[256];
	DBVARIANT dbv, dbv2;
	for(int i = 0; i < 10; i++) {
		mir_snprintf(buff, 256, "SavedNum%d", i);
		if(!DBGetContactSettingTString(0, MODULE, buff, &dbv)) {
			mir_snprintf(buff, 256, "SavedName%d", i);
			if(!DBGetContactSettingTString(0, MODULE, buff, &dbv2)) {
				list_insert(dbv.pszVal, dbv2.pszVal);
				DBFreeVariant(&dbv2);
			}else
				list_insert(dbv.pszVal, "");
			DBFreeVariant(&dbv);
		}
	}

	hBrushActive = CreateSolidBrush(RGB(0, 0xff, 0)); // green
	hBrushFree = CreateSolidBrush(RGB(0xD0, 0xD0, 0xD0)); // grey
	hBrushActiveSelect = CreateSolidBrush(RGB(0x80, 0xff, 0x80)); // light green
	hBrushFreeSelect = CreateSolidBrush(RGB(0xff, 0xff, 0xff)); // white

	hBrushRingIn = CreateSolidBrush(RGB(0xff, 0, 0xff)); // purple
	hBrushRingOut = CreateSolidBrush(RGB(0, 0, 0xff)); // blue
	hBrushRingInSelect = CreateSolidBrush(RGB(0xff, 0x80, 0xff)); // light-purple
	hBrushRingOutSelect = CreateSolidBrush(RGB(0x80, 0x80, 0xff)); // light-blue

	hkid = GlobalAddAtom("IAX plugin hk id");
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIAL), 0, DlgProcDial, 0);
}

void DeinitDialDialog() {
	UnregisterHotKey(hWnd, hkid);
	GlobalDeleteAtom(hkid);
	DestroyWindow(hWnd);

	CallHist *node;
	int i = 0;
	char buff[256];
	while(hist) {

		node = hist;
		if(i < 10) {
			mir_snprintf(buff, 256, "SavedNum%d", i);
			DBWriteContactSettingTString(0, MODULE, buff, node->num);
			mir_snprintf(buff, 256, "SavedName%d", i);
			DBWriteContactSettingTString(0, MODULE, buff, node->name);
		}
		hist = hist->next;
		free(node);
		i++;
	}

	DeleteObject(hBrushActive);
	DeleteObject(hBrushFree);
	DeleteObject(hBrushActiveSelect);
	DeleteObject(hBrushFreeSelect);

	DeleteObject(hBrushRingIn);
	DeleteObject(hBrushRingOut);
	DeleteObject(hBrushRingInSelect);
	DeleteObject(hBrushRingOutSelect);
}