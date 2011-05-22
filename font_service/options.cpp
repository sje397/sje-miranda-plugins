#include "common.h"
#include "services.h"

FontIDWList font_id_list_w2, font_id_list_w3;
ColourIDWList colour_id_list_w2, colour_id_list_w3;

#define M_SETFONTGROUP		(WM_USER + 101)
#define TIMER_ID				11015

extern void UpdateFontSettingsW(FontIDW *font_id, FontSettingsW *fontsettings);
extern void UpdateColourSettingsW(ColourIDW *colour_id, COLORREF *colour);

void WriteLine(HANDLE fhand, char *line) {
	DWORD wrote;
	strcat(line, "\r\n");
	WriteFile(fhand, line, strlen(line), &wrote, 0);
}

bool ExportSettings(HWND hwndDlg, TCHAR *filename, FontIDWList flist, ColourIDWList clist) {
	HANDLE fhand = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(fhand == INVALID_HANDLE_VALUE) {
		MessageBox(hwndDlg, filename, _T("Failed to create file"), MB_ICONWARNING | MB_OK);
		return false;
	}

	char header[512], buff[1024], abuff[1024];
	header[0] = 0;

	strcpy(buff, "SETTINGS:\r\n");
	WriteLine(fhand, buff);
	FontIDSettingsWPair *it;
	for (FontIDWList::Iterator lit = flist.start(); lit.has_val(); lit.next()) {
		it = &lit.val();

		strcpy(buff, "[");
		strcat(buff, it->first.dbSettingsGroup);
		strcat(buff, "]");
		if(strcmp(buff, header) != 0) {
			strcpy(header, buff);
			WriteLine(fhand, buff);
		}

		if(it->first.flags & FIDF_APPENDNAME) {
			wsprintfA(buff, "%sName", it->first.prefix);
		} else {
			wsprintfA(buff, "%s", it->first.prefix);
		}
		strcat(buff, "=s");
		WideCharToMultiByte(code_page, 0, it->second.szFace, -1, abuff, 1024, 0, 0);
		abuff[1023]=0;
		strcat(buff, abuff);
		WriteLine(fhand, buff);
		
		wsprintfA(buff, "%sSize=b", it->first.prefix);
		if(it->first.flags & FIDF_SAVEACTUALHEIGHT) {
			HDC hdc;
			SIZE size;
			HFONT hFont, hOldFont;
			LOGFONTW lf;
			CreateFromFontSettingsW(&it->second, &lf, it->first.flags);
			hFont = CreateFontIndirectW(&lf);
			//hFont = CreateFontA(it->second.size, 0, 0, 0,
			//					it->second.style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL,
			//					it->second.style & DBFONTF_ITALIC ? 1 : 0, 0, 0, it->second.charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, it->second.szFace);
			hdc = GetDC(hwndDlg);
			hOldFont = (HFONT)SelectObject(hdc, hFont);
			GetTextExtentPoint32(hdc, _T("_W"), 2, &size);
			ReleaseDC(hwndDlg, hdc);
			SelectObject(hdc, hOldFont);
			DeleteObject(hFont);

			strcat(buff, _itoa((BYTE)size.cy, abuff, 10));
	
		} else if(it->first.flags & FIDF_SAVEPOINTSIZE) {
			HDC hdc = GetDC(hwndDlg);
			//lf->lfHeight = -MulDiv(lf->lfHeight,GetDeviceCaps(hdc, LOGPIXELSY), 72);
			strcat(buff, _itoa((BYTE)-MulDiv(it->second.size, 72, GetDeviceCaps(hdc, LOGPIXELSY)), abuff, 10));
			ReleaseDC(hwndDlg, hdc);
		} else {
			strcat(buff, _itoa((BYTE)it->second.size, abuff, 10));
		}
		WriteLine(fhand, buff);

		wsprintfA(buff, "%sSty=b%d", it->first.prefix, (BYTE)it->second.style);
		WriteLine(fhand, buff);
		wsprintfA(buff, "%sSet=b%d", it->first.prefix, (BYTE)it->second.charset);
		WriteLine(fhand, buff);
		wsprintfA(buff, "%sCol=d%d", it->first.prefix, (DWORD)it->second.colour);
		WriteLine(fhand, buff);
		if(it->first.flags & FIDF_NOAS) {
			wsprintfA(buff, "%sAs=w%d", it->first.prefix, (WORD)0x00FF);
			WriteLine(fhand, buff);
		}
		wsprintfA(buff, "%sFlags=w%d", it->first.prefix, (WORD)it->first.flags);
		WriteLine(fhand, buff);
	}

	header[0] = 0;
	ColourIDColourWPair *cit;
	for (ColourIDWList::Iterator lcit = clist.start(); lcit.has_val(); lcit.next()) {
		cit = &lcit.val();
		strcpy(buff, "[");
		strcat(buff, cit->first.dbSettingsGroup);
		strcat(buff, "]");
		if(strcmp(buff, header) != 0) {
			strcpy(header, buff);
			WriteLine(fhand, buff);
		}
		wsprintfA(buff, "%s=d%d", cit->first.setting, (DWORD)cit->second);
		WriteLine(fhand, buff);
	}


	CloseHandle(fhand);
	return true;
}

void OptionsChanged() {
#define INTM_RELOADOPTIONS   (WM_USER+21)
	HWND hWnd = FindWindowEx((HWND)CallService(MS_CLUI_GETHWND, 0, 0), 0, CLISTCONTROL_CLASS, 0);
	if(hWnd) SendMessage(hWnd, INTM_RELOADOPTIONS, 0, 0);
	NotifyEventHooks(hFontReloadEvent, 0, 0);
	NotifyEventHooks(hColourReloadEvent, 0, 0);
}

TOOLINFO ti;
int x, y;

UINT_PTR CALLBACK CFHookProc(HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam) {
	switch(uiMsg) {
		case WM_INITDIALOG:
			TranslateDialogDefault(hdlg);
			return 0;
	}

	return 0;
}


static BOOL CALLBACK DlgProcLogOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HBRUSH hBkgColourBrush = 0;

	switch (msg) {
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			/*
			// create tooltip tool
			ti.cbSize = sizeof(TOOLINFO);
			ti.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
			ti.hwnd   = GetDlgItem(hwndDlg, IDC_TOOLTIP);
			ti.uId    = (UINT)hwndDlg;
			ti.hinst  = hInstance;
			ti.lpszText  = L"hi there";
			ti.rect.left = ti.rect.top = ti.rect.bottom = ti.rect.right = 0; 

			SendMessage(GetDlgItem(hwndDlg, IDC_TOOLTIP), TTM_ADDTOOL,0, (LPARAM)&ti);
			*/

			font_id_list_w3 = font_id_list_w2 = font_id_list_w;
			colour_id_list_w3 = colour_id_list_w2 = colour_id_list_w;

			FontIDSettingsWPair *it;
			for (FontIDWList::Iterator lit = font_id_list_w2.start(); lit.has_val(); lit.next()) {
				it = &lit.val();
				// sync settings with database
				UpdateFontSettingsW(&it->first, &it->second);
				if(SendDlgItemMessageW(hwndDlg, IDC_FONTGROUP, LB_FINDSTRINGEXACT, (WPARAM)-1, (WPARAM)it->first.group) == CB_ERR) {
					SendDlgItemMessageW(hwndDlg, IDC_FONTGROUP, LB_ADDSTRING, (WPARAM)-1, (WPARAM)it->first.group);
				}

			}

			ColourIDColourWPair *cit;
			for (ColourIDWList::Iterator lcit = colour_id_list_w2.start(); lcit.has_val(); lcit.next()) {
				cit = &lcit.val();
				// sync settings with database
				UpdateColourSettingsW(&cit->first, &cit->second);
			}

			SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, (LPARAM)GetSysColor(COLOR_WINDOW));

			if(font_id_list_w2.size()) SendDlgItemMessage(hwndDlg, IDC_FONTGROUP, LB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, M_SETFONTGROUP, 0, 0);

			return TRUE;
		}
		case M_SETFONTGROUP:
		{
			int sel = SendDlgItemMessage(hwndDlg, IDC_FONTGROUP, LB_GETCURSEL, 0, 0);
			if(sel != -1) {
				wchar_t group_buff[64];
				SendDlgItemMessage(hwndDlg, IDC_FONTGROUP, LB_GETTEXT, sel, (LPARAM)group_buff);

				SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_RESETCONTENT, 0, 0);
				SendDlgItemMessage(hwndDlg, IDC_COLOURLIST, CB_RESETCONTENT, 0, 0);
				{
					BOOL need_restart = false;
					int fontId = 0, itemId;
					int first_font_index = -1;
					FontIDSettingsWPair *it;
					for (FontIDWList::Iterator lit = font_id_list_w2.start(); lit.has_val(); lit.next(), fontId++) {
						it = &lit.val();
						if(wcsncmp(it->first.group, group_buff, 64) == 0) {						
							if(first_font_index == -1)
								first_font_index = fontId;
							//itemId = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_ADDSTRING, -1, (LPARAM)"");
							//SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETITEMDATA, itemId, fontId);
							itemId = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_ADDSTRING, (WPARAM)-1, fontId);
							need_restart |= (it->first.flags & FIDF_NEEDRESTART);
							//SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETITEMDATA, itemId, fontId);
						}
					}

					ShowWindow(GetDlgItem(hwndDlg, IDC_STAT_RESTART), (need_restart ? SW_SHOW : SW_HIDE));

					if(hBkgColourBrush) {
						DeleteObject(hBkgColourBrush);
						hBkgColourBrush = 0;
					}
					int colourId = 0;
					int first_colour_index = -1;
					ColourIDColourWPair *cit;
					for (ColourIDWList::Iterator lcit = colour_id_list_w2.start(); lcit.has_val(); lcit.next(), colourId++) {
						cit = &lcit.val();
						if(wcsncmp(cit->first.group, group_buff, 64) == 0) {
							if(first_colour_index == -1)
								first_colour_index = colourId;
							itemId = SendDlgItemMessage(hwndDlg, IDC_COLOURLIST, CB_ADDSTRING, (WPARAM)-1, (LPARAM) TranslateW(cit->first.name));
							SendDlgItemMessage(hwndDlg, IDC_COLOURLIST, CB_SETITEMDATA, itemId, colourId);

							if(wcscmp(cit->first.name, TranslateW(L"Background")) == 0) {
								hBkgColourBrush = CreateSolidBrush(cit->second);
							}
						}
					}
					if(!hBkgColourBrush)
						hBkgColourBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
					
					SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETSEL, TRUE, 0);
					SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETCOLOUR, 0, (LPARAM)font_id_list_w2[first_font_index].second.colour);
					SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETDEFAULTCOLOUR, 0, (LPARAM)font_id_list_w2[first_font_index].first.deffontsettings.colour);

					if(font_id_list_w2[first_font_index].first.flags & FIDF_DEFAULTVALID) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), TRUE);
					} else
						EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), FALSE);

					if(first_colour_index == -1) {
						HWND hw = GetDlgItem(hwndDlg, IDC_COLOURLIST);
						EnableWindow(hw, FALSE);
						hw = GetDlgItem(hwndDlg, IDC_BKGCOLOUR);
						EnableWindow(hw, FALSE);

					} else {
						HWND hw = GetDlgItem(hwndDlg, IDC_COLOURLIST);
						EnableWindow(hw, TRUE);
						hw = GetDlgItem(hwndDlg, IDC_BKGCOLOUR);
						EnableWindow(hw, TRUE);

						SendDlgItemMessage(hwndDlg, IDC_COLOURLIST, CB_SETCURSEL, 0, 0);
						SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, (LPARAM)colour_id_list_w2[first_colour_index].second);
						SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, colour_id_list_w2[first_colour_index].first.defcolour);

					}
				}
			}
			return TRUE;
		}
		case WM_CTLCOLORLISTBOX:
			{
				if((HWND)lParam == GetDlgItem(hwndDlg, IDC_FONTLIST)) {
					return (BOOL) hBkgColourBrush;
				} 
			}
			break;
		case WM_MEASUREITEM:
		{
			MEASUREITEMSTRUCT *mis = (MEASUREITEMSTRUCT *) lParam;
			if(mis->CtlID == IDC_FONTLIST && font_id_list_w2.size()) {
				HFONT hFont, hoFont;
				SIZE fontSize;
				int iItem = mis->itemData;
				//int iItem = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, mis->itemID, 0);
				HDC hdc = GetDC(GetDlgItem(hwndDlg, mis->CtlID));
				LOGFONTW lf;
				CreateFromFontSettingsW(&font_id_list_w2[iItem].second, &lf, font_id_list_w2[iItem].first.flags);
				hFont = CreateFontIndirectW(&lf);

				//hFont = CreateFontA(font_id_list2[iItem].second.size, 0, 0, 0,
				//					font_id_list2[iItem].second.style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL,
				//					font_id_list2[iItem].second.style & DBFONTF_ITALIC ? 1 : 0, 0, 0, font_id_list2[iItem].second.charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font_id_list2[iItem].second.szFace);
				hoFont = (HFONT) SelectObject(hdc, hFont);
				GetTextExtentPoint32W(hdc, font_id_list_w2[iItem].first.name, wcslen(font_id_list_w2[iItem].first.name), &fontSize);
				SelectObject(hdc, hoFont);
				ReleaseDC(GetDlgItem(hwndDlg, mis->CtlID), hdc);
				DeleteObject(hFont);
				mis->itemWidth = fontSize.cx;
				mis->itemHeight = fontSize.cy;
				return TRUE;
			}
			break;
		}
		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *) lParam;
			if(dis->CtlID == IDC_FONTLIST && font_id_list_w2.size()) {
				HFONT hFont, hoFont;
				wchar_t *pszText;
				int iItem = dis->itemData;
				LOGFONTW lf;
				CreateFromFontSettingsW(&font_id_list_w2[iItem].second, &lf, font_id_list_w2[iItem].first.flags);
				hFont = CreateFontIndirectW(&lf);
				//hFont = CreateFontA(font_id_list2[iItem].second.size, 0, 0, 0,
				//					font_id_list2[iItem].second.style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL,
				//					font_id_list2[iItem].second.style & DBFONTF_ITALIC ? 1 : 0, 0, 0, font_id_list2[iItem].second.charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font_id_list2[iItem].second.szFace);
				hoFont = (HFONT) SelectObject(dis->hDC, hFont);
				SetBkMode(dis->hDC, TRANSPARENT);
				FillRect(dis->hDC, &dis->rcItem, hBkgColourBrush);
				if (dis->itemState & ODS_SELECTED)
					FrameRect(dis->hDC, &dis->rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
				SetTextColor(dis->hDC, font_id_list_w2[iItem].second.colour);
				pszText = TranslateW(font_id_list_w2[iItem].first.name);
				TextOutW(dis->hDC, dis->rcItem.left, dis->rcItem.top, pszText, wcslen(pszText));
				SelectObject(dis->hDC, hoFont);
				DeleteObject(hFont);
				return TRUE;
			}
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDC_COLOURLIST:
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						int sel = SendDlgItemMessage(hwndDlg, IDC_COLOURLIST, CB_GETCURSEL, 0, 0);
						if(sel != -1) {
							int i = SendDlgItemMessage(hwndDlg, IDC_COLOURLIST, CB_GETITEMDATA, sel, 0);
							SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, colour_id_list_w2[i].second);
							SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, colour_id_list_w2[i].first.defcolour);
						}
					}
					if (HIWORD(wParam) != LBN_DBLCLK)
						return TRUE;
					//fall through
				case IDC_BKGCOLOUR:

					{
						int sel = SendDlgItemMessage(hwndDlg, IDC_COLOURLIST, CB_GETCURSEL, 0, 0);
						if(sel != -1) {
							int i = SendDlgItemMessage(hwndDlg, IDC_COLOURLIST, CB_GETITEMDATA, sel, 0);
							colour_id_list_w2[i].second = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0);
							if(wcscmp(colour_id_list_w2[i].first.name, TranslateW(L"Background")) == 0) {
								DeleteObject(hBkgColourBrush);
								hBkgColourBrush = CreateSolidBrush(SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR, 0, 0));
							}
						}
					}
					InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), NULL, TRUE);
					break;
				case IDC_FONTGROUP:
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						SendMessage(hwndDlg, M_SETFONTGROUP, 0, 0);
					}
					return TRUE;
				case IDC_FONTLIST:
					if (HIWORD(wParam) == LBN_SELCHANGE) {
						if(font_id_list_w2.size()) {
							int *selItems = (int *)mir_alloc(font_id_list_w2.size() * sizeof(int));
							int sel, selCount, i;

							selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)font_id_list_w2.size(), (LPARAM) selItems);

							if (selCount > 1) {
								SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETCOLOUR, 0, GetSysColor(COLOR_3DFACE));
								SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOWTEXT));
								BOOL show_default = FALSE;
								for (sel = 0; sel < selCount; sel++) {
									i = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[sel], 0);
									show_default |= (font_id_list_w2[i].first.flags & FIDF_DEFAULTVALID);
								}
								if(show_default) {
									EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), TRUE);
								} else
									EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), FALSE);
							}
							else {
								int i = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA,
														   SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETCURSEL, 0, 0), 0);
								SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETCOLOUR, 0, font_id_list_w2[i].second.colour);
								SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETDEFAULTCOLOUR, 0, (LPARAM)font_id_list_w2[i].first.deffontsettings.colour);

								if(font_id_list_w2[i].first.flags & FIDF_DEFAULTVALID) {
									EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), TRUE);
								} else
									EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_RESET), FALSE);
							}
							mir_free(selItems);
						}
						return TRUE;
					}
					if (HIWORD(wParam) != LBN_DBLCLK)
						return TRUE;
					//fall through
				case IDC_CHOOSEFONT:
				{
					LOGFONTW lf = { 0 };
					if(SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETCOUNT, 0, 0)) {
						int sel = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETCURSEL, 0, 0);
						int i = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, sel, 0);
						
						CreateFromFontSettingsW(&font_id_list_w2[i].second, &lf, font_id_list_w2[i].first.flags);
						CreateFontIndirectW(&lf);

						CHOOSEFONTW cf = { 0 };
						cf.lStructSize = sizeof(cf);
						cf.hwndOwner = hwndDlg;
						cf.lpLogFont = &lf;
						if(font_id_list_w2[i].first.flags & FIDF_ALLOWEFFECTS) {
							cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS | CF_ENABLETEMPLATE | CF_ENABLEHOOK;
							// use custom font dialog to disable colour selection
							cf.hInstance = hInstance;
							cf.lpTemplateName = MAKEINTRESOURCEW(IDD_CUSTOM_FONT);
							cf.lpfnHook = CFHookProc;
						} else {
							cf.Flags = CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
						}

						if (ChooseFontW(&cf)) {
							int *selItems = (int *)mir_alloc(font_id_list_w2.size() * sizeof(int));
							int sel, selCount;

							selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)font_id_list_w2.size(), (LPARAM) selItems);
							for (sel = 0; sel < selCount; sel++) {
								i = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[sel], 0);
								font_id_list_w2[i].second.size = (char)lf.lfHeight;
								font_id_list_w2[i].second.style = (lf.lfWeight >= FW_BOLD ? DBFONTF_BOLD : 0) | (lf.lfItalic ? DBFONTF_ITALIC : 0) | (lf.lfUnderline ? DBFONTF_UNDERLINE : 0) | (lf.lfStrikeOut ? DBFONTF_STRIKEOUT : 0);
								font_id_list_w2[i].second.charset = lf.lfCharSet;
								wcscpy(font_id_list_w2[i].second.szFace, lf.lfFaceName);
								{
									MEASUREITEMSTRUCT mis = { 0 };
									mis.CtlID = IDC_FONTLIST;
									mis.itemData = i;
									SendMessage(hwndDlg, WM_MEASUREITEM, 0, (LPARAM) & mis);
									SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETITEMHEIGHT, selItems[sel], mis.itemHeight);
								}
							}
							mir_free(selItems);
							InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), NULL, TRUE);
							break;
						}
					}
					return TRUE;
				}
				case IDC_FONTCOLOUR:
				{
					if(font_id_list_w2.size()) {
						int *selItems = (int *)mir_alloc(font_id_list_w2.size() * sizeof(int));
						int sel, selCount, i;

						selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)font_id_list_w2.size(), (LPARAM) selItems);
						for (sel = 0; sel < selCount; sel++) {
							i = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[sel], 0);
							font_id_list_w2[i].second.colour = SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_GETCOLOUR, 0, 0);
						}
						mir_free(selItems);
						InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), NULL, FALSE);
					}
					break;
				}
				case IDC_BTN_RESET:
				{
					if(font_id_list_w2.size()) {
						int *selItems = (int *)mir_alloc(font_id_list_w2.size() * sizeof(int));
						int sel, selCount, i;

						selCount = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETSELITEMS, (WPARAM)font_id_list_w2.size(), (LPARAM) selItems);
						for (sel = 0; sel < selCount; sel++) {
							i = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA, selItems[sel], 0);
							if(font_id_list_w2[i].first.flags & FIDF_DEFAULTVALID) {
								font_id_list_w2[i].second = font_id_list_w2[i].first.deffontsettings;
								{
									MEASUREITEMSTRUCT mis = { 0 };
									mis.CtlID = IDC_FONTLIST;
									mis.itemData = i;
									SendMessage(hwndDlg, WM_MEASUREITEM, 0, (LPARAM) & mis);
									SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_SETITEMHEIGHT, selItems[sel], mis.itemHeight);
								}
							}
						}
						if (selCount > 1) {
							SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETCOLOUR, 0, GetSysColor(COLOR_3DFACE));
							SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOWTEXT));
						}
						else {
							int i = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMDATA,
													   SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETCURSEL, 0, 0), 0);
							SendDlgItemMessage(hwndDlg, IDC_FONTCOLOUR, CPM_SETCOLOUR, 0, font_id_list_w2[i].second.colour);
						}
						mir_free(selItems);
						InvalidateRect(GetDlgItem(hwndDlg, IDC_FONTLIST), NULL, FALSE);
					}
					break;
				}
				case IDC_BTN_UNDO:
				{
					font_id_list_w2 = font_id_list_w3;
					colour_id_list_w2 = colour_id_list_w3;
					EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), FALSE);
			
					SendDlgItemMessage(hwndDlg, IDC_FONTGROUP, LB_SETCURSEL, 0, 0);
					SendMessage(hwndDlg, M_SETFONTGROUP, 0, 0);
					break;
				}
				case IDC_BTN_EXPORT:
				{
					TCHAR fname_buff[MAX_PATH];
					OPENFILENAME ofn = {0};
					ofn.lStructSize = sizeof(ofn);
					ofn.lpstrFile = fname_buff;
					ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = MAX_PATH;
					ofn.hwndOwner = hwndDlg;
					ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;
					ofn.lpstrFilter = _T("INI\0*.ini\0Text\0*.TXT\0All\0*.*\0");
					ofn.nFilterIndex = 1;

					ofn.lpstrDefExt = _T("ini");

					if(GetSaveFileName(&ofn) == TRUE) {
						if(!ExportSettings(hwndDlg, ofn.lpstrFile, font_id_list_w, colour_id_list_w))
							MessageBox(hwndDlg, _T("Error writing file"), _T("Error"), MB_ICONWARNING | MB_OK);
					}

					return TRUE;
				}
			}
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;
		case WM_NOTIFY:
			switch (((LPNMHDR) lParam)->idFrom) {
				case 0:
					switch (((LPNMHDR) lParam)->code) {
						case PSN_APPLY:

							{
								font_id_list_w3 = font_id_list_w;
								colour_id_list_w3 = colour_id_list_w;
								EnableWindow(GetDlgItem(hwndDlg, IDC_BTN_UNDO), TRUE);

								font_id_list_w = font_id_list_w2;
								colour_id_list_w = colour_id_list_w2;

								char str[32];
								FontIDSettingsWPair *it;
								for (FontIDWList::Iterator lit = font_id_list_w2.start(); lit.has_val(); lit.next()) {
									it = &lit.val();
									if(it->first.flags & FIDF_APPENDNAME) wsprintfA(str, "%sName", it->first.prefix);
									else wsprintfA(str, "%s", it->first.prefix);

									if(DBWriteContactSettingWString(NULL, it->first.dbSettingsGroup, str, it->second.szFace)) {
										char buff[1024];
										WideCharToMultiByte(code_page, 0, it->second.szFace, -1, buff, 1024, 0, 0);
										DBWriteContactSettingString(NULL, it->first.dbSettingsGroup, str, buff);
									}
									
									wsprintfA(str, "%sSize", it->first.prefix);
									if(it->first.flags & FIDF_SAVEACTUALHEIGHT) {
										HDC hdc;
										SIZE size;
										HFONT hFont, hOldFont;
										LOGFONTW lf;
										CreateFromFontSettingsW(&it->second, &lf, it->first.flags);
										hFont = CreateFontIndirectW(&lf);
										//hFont = CreateFontA(it->second.size, 0, 0, 0,
										//					it->second.style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL,
										//					it->second.style & DBFONTF_ITALIC ? 1 : 0, 0, 0, it->second.charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, it->second.szFace);
										hdc = GetDC(hwndDlg);
										hOldFont = (HFONT)SelectObject(hdc, hFont);
										GetTextExtentPoint32(hdc, _T("_W"), 2, &size);
										ReleaseDC(hwndDlg, hdc);
										SelectObject(hdc, hOldFont);
										DeleteObject(hFont);

										DBWriteContactSettingByte(NULL, it->first.dbSettingsGroup, str, (char)size.cy);
									} else if(it->first.flags & FIDF_SAVEPOINTSIZE) {
										HDC hdc = GetDC(hwndDlg);
										//lf->lfHeight = -MulDiv(lf->lfHeight,GetDeviceCaps(hdc, LOGPIXELSY), 72);
										DBWriteContactSettingByte(NULL, it->first.dbSettingsGroup, str, (BYTE)-MulDiv(it->second.size, 72, GetDeviceCaps(hdc, LOGPIXELSY)));
										ReleaseDC(hwndDlg, hdc);
									} else {
										DBWriteContactSettingByte(NULL, it->first.dbSettingsGroup, str, it->second.size);
									}
									wsprintfA(str, "%sSty", it->first.prefix);
									DBWriteContactSettingByte(NULL, it->first.dbSettingsGroup, str, it->second.style);
									wsprintfA(str, "%sSet", it->first.prefix);
									DBWriteContactSettingByte(NULL, it->first.dbSettingsGroup, str, it->second.charset);
									wsprintfA(str, "%sCol", it->first.prefix);
									DBWriteContactSettingDword(NULL, it->first.dbSettingsGroup, str, it->second.colour);
									if(it->first.flags & FIDF_NOAS) {
										wsprintfA(str, "%sAs", it->first.prefix);
										DBWriteContactSettingWord(NULL, it->first.dbSettingsGroup, str, (WORD)0x00FF);
									}
									wsprintfA(str, "%sFlags", it->first.prefix);
									DBWriteContactSettingWord(NULL, it->first.dbSettingsGroup, str, (WORD)it->first.flags);
								}

								ColourIDColourWPair *cit;
								for(ColourIDWList::Iterator lcit = colour_id_list_w2.start(); lcit.has_val(); lcit.next()) {
									cit = &lcit.val();
									wsprintfA(str, "%s", cit->first.setting);
									DBWriteContactSettingDword(NULL, cit->first.dbSettingsGroup, str, cit->second);
								}
	
							}
							
							OptionsChanged();
							return TRUE;
					}
					break;
			}
			break;
/*
// tooltip stuff

		case WM_MOUSEMOVE: 
#define X_OFFSET 15
#define Y_OFFSET 15
			SendMessage(GetDlgItem(hwndDlg, IDC_TOOLTIP),TTM_TRACKACTIVATE,(WPARAM)FALSE,(LPARAM)&ti);
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			SendMessage(GetDlgItem(hwndDlg, IDC_TOOLTIP), TTM_TRACKPOSITION, 0, (LPARAM)MAKELPARAM(LOWORD(lParam) + X_OFFSET, HIWORD(lParam) + Y_OFFSET));
			KillTimer(hwndDlg, TIMER_ID);
			SetTimer(hwndDlg, TIMER_ID, 1000, 0);
			break;
		case WM_TIMER:
			{
*/
				/*
				//POINT p;
				RECT r;
				GetClientRect(GetDlgItem(hwndDlg, IDC_FONTLIST), &r);
				DWORD lParam = MAKELPARAM(x - r.left, y - r.bottom);
				DWORD res = SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_ITEMFROMPOINT, 0, lParam);
				if(HIWORD(res) == 0 && HIWORD(res) >= 0) {
					char buf[256];
					int sel = LOWORD(res);
					//SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETITEMRECT, (WPARAM)sel, (LPARAM)&r);
					SendDlgItemMessage(hwndDlg, IDC_FONTLIST, LB_GETTEXT, (WPARAM)sel, (LPARAM)buf);
				*/
/*

					KillTimer(hwndDlg, TIMER_ID);
					SendMessage(GetDlgItem(hwndDlg, IDC_TOOLTIP),TTM_TRACKACTIVATE,(WPARAM)TRUE,(LPARAM)&ti);
					MessageBox(hwndDlg, "Hi", "blah", MB_OK);
				//}
				break;
			}
*/
		case WM_DESTROY:
			KillTimer(hwndDlg, TIMER_ID);
			DeleteObject(hBkgColourBrush);
			break;
	}
	return FALSE;
}



int OptInit(WPARAM wParam, LPARAM lParam) {
	OPTIONSDIALOGPAGE odp = {0};
#define OPTIONPAGE_OLD_SIZE2		60

	//odp.cbSize						= sizeof(odp);
	odp.cbSize						= OPTIONPAGE_OLD_SIZE2;
	odp.position					= -790000000;
	odp.hInstance					= hInstance;
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT2);
	odp.pszTitle					= Translate("Fonts");
	odp.pszGroup					= Translate("Customize");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcLogOptions;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );	

	return 0;
}

