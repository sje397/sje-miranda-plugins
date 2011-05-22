#include "common.h"
#include "services.h"

#include <algorithm>

int code_page = CP_ACP;

void ConvertFontSettings(FontSettings *fs, FontSettingsW *fsw) {
	fsw->colour = fs->colour;
	fsw->size = fs->size;
	fsw->style = fs->style;
	fsw->charset = fs->charset;

	MultiByteToWideChar(code_page, 0, fs->szFace, -1, fsw->szFace, LF_FACESIZE);
}

void ConvertFontID(FontID *fid, FontIDW *fidw) {
	fidw->cbSize = sizeof(FontIDW);
	strcpy(fidw->dbSettingsGroup, fid->dbSettingsGroup);
	strcpy(fidw->prefix, fid->prefix);
	fidw->flags = fid->flags;
	fidw->order = fid->order;
	ConvertFontSettings(&fid->deffontsettings, &fidw->deffontsettings);

	MultiByteToWideChar(code_page, 0, fid->group, -1, fidw->group, 64);
	MultiByteToWideChar(code_page, 0, fid->name, -1, fidw->name, 64);
}

void ConvertColourID(ColourID *cid, ColourIDW *cidw) {
	cidw->cbSize = sizeof(ColourIDW);

	strcpy(cidw->dbSettingsGroup, cid->dbSettingsGroup);
	strcpy(cidw->setting, cid->setting);
	cidw->flags = cid->flags;
	cidw->defcolour = cid->defcolour;
	cidw->order = cid->order;

	MultiByteToWideChar(code_page, 0, cid->group, -1, cidw->group, 64);
	MultiByteToWideChar(code_page, 0, cid->name, -1, cidw->name, 64);
}

void ConvertLOGFONT(LOGFONTW *lfw, LOGFONTA *lfa) {
    lfa->lfHeight = lfw->lfHeight;
    lfa->lfWidth = lfw->lfWidth;
    lfa->lfEscapement = lfw->lfEscapement;
    lfa->lfOrientation = lfw->lfOrientation;
    lfa->lfWeight = lfw->lfWeight;
    lfa->lfItalic = lfw->lfItalic;
    lfa->lfUnderline = lfw->lfUnderline;
    lfa->lfStrikeOut = lfw->lfStrikeOut;
    lfa->lfCharSet = lfw->lfCharSet;
    lfa->lfOutPrecision = lfw->lfOutPrecision;
    lfa->lfClipPrecision = lfw->lfClipPrecision;
    lfa->lfQuality = lfw->lfQuality;
    lfa->lfPitchAndFamily = lfw->lfPitchAndFamily;

	WideCharToMultiByte(code_page, 0, lfw->lfFaceName, -1, lfa->lfFaceName, LF_FACESIZE, 0, 0);
}

bool operator <(const FontIDW &id1, const FontIDW &id2) {
	int db_cmp = strncmp(id1.dbSettingsGroup, id2.dbSettingsGroup, sizeof(id1.dbSettingsGroup));
	if(db_cmp) {
		return db_cmp < 0;
	}
	
	return wcsncmp(id1.group, id2.group, sizeof(id1.group)) < 0 
	  || (wcsncmp(id1.group, id2.group, sizeof(id1.group)) == 0 && id1.order < id2.order)
	  || (wcsncmp(id1.group, id2.group, sizeof(id1.group)) == 0 && id1.order == id2.order && wcsncmp(id1.name, id2.name, sizeof(id1.name)) < 0);
}

bool operator <(const ColourIDW &id1, const ColourIDW &id2) {
	int db_cmp = strncmp(id1.dbSettingsGroup, id2.dbSettingsGroup, sizeof(id1.dbSettingsGroup));
	if(db_cmp) {
		return db_cmp < 0;
	}
	
	return wcsncmp(id1.group, id2.group, sizeof(id1.group)) < 0 
	  || (wcsncmp(id1.group, id2.group, sizeof(id1.group)) == 0 && id1.order < id2.order)
	  || (wcsncmp(id1.group, id2.group, sizeof(id1.group)) == 0 && id1.order == id2.order && wcsncmp(id1.name, id2.name, sizeof(id1.name)) < 0);
}

bool operator ==(const FontIDW &id1, const FontIDW &id2) {
	int db_cmp = strncmp(id1.dbSettingsGroup, id2.dbSettingsGroup, sizeof(id1.dbSettingsGroup));
	if(db_cmp != 0) {
		return false;
	}
	
	return wcsncmp(id1.group, id2.group, sizeof(id1.group)) == 0 && id1.order == id2.order && wcsncmp(id1.name, id2.name, sizeof(id1.name)) == 0;
}

bool operator ==(const ColourIDW &id1, const ColourIDW &id2) {
	int db_cmp = strncmp(id1.dbSettingsGroup, id2.dbSettingsGroup, sizeof(id1.dbSettingsGroup));
	if(db_cmp != 0) {
		return false;
	}
	
	return wcsncmp(id1.group, id2.group, sizeof(id1.group)) == 0 && id1.order == id2.order && wcsncmp(id1.name, id2.name, sizeof(id1.name)) == 0;
}

/*
bool operator <(const FontIDSettingsWPair &id1, const FontIDSettingsWPair &id2) {
	return id1.first < id2.first;
}

bool operator <(const ColourIDColourWPair &id1, const ColourIDColourWPair &id2) {
	return id1.first < id2.first;
}
*/

FontIDWList font_id_list_w;
ColourIDWList colour_id_list_w;

static void GetDefaultFontSettingW(LOGFONTW * lf, COLORREF * colour)
{
    SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(LOGFONTW), lf, FALSE);
    if(colour) *colour = GetSysColor(COLOR_WINDOWTEXT);
    lf->lfHeight = 10;

	HDC hdc = GetDC(0);
	lf->lfHeight = -MulDiv(lf->lfHeight,GetDeviceCaps(hdc, LOGPIXELSY), 72);
	ReleaseDC(0, hdc);
}

int GetFontSettingFromDBW(char *settings_group, char *prefix, LOGFONTW * lf, COLORREF * colour, DWORD flags)
{
    DBVARIANT dbv;
    char idstr[256];
    BYTE style;
	int retval = 0;

    GetDefaultFontSettingW(lf, colour);
   
	if(flags & FIDF_APPENDNAME) sprintf(idstr, "%sName", prefix);
    else sprintf(idstr, "%s", prefix);

	if (!DBGetContactSettingWString(NULL, settings_group, idstr, &dbv)) {
		wcscpy(lf->lfFaceName, dbv.pwszVal);
		mir_free(dbv.pwszVal);
	} else if(!DBGetContactSetting(NULL, settings_group, idstr, &dbv)) {
		if(dbv.type == DBVT_ASCIIZ) {
			wchar_t buff[1024];
			MultiByteToWideChar(code_page, 0, dbv.pszVal, -1, buff, 1024);
			wcscpy(lf->lfFaceName, buff);
		} else
			retval = 1;
		DBFreeVariant(&dbv);
	} else 
		retval = 1;

	if(colour) {
	    sprintf(idstr, "%sCol", prefix);
		*colour = DBGetContactSettingDword(NULL, settings_group, idstr, *colour);
	}

    sprintf(idstr, "%sSize", prefix);
    lf->lfHeight = (char)DBGetContactSettingByte(NULL, settings_group, idstr, lf->lfHeight);
	
    
	//wsprintf(idstr, "%sFlags", prefix);
	//if(DBGetContactSettingDword(NULL, settings_group, idstr, 0) & FIDF_SAVEACTUALHEIGHT) {
	//	HDC hdc = GetDC(0);
	//	lf->lfHeight = -lf->lfHeight;
	//	ReleaseDC(0, hdc);
	//}

    sprintf(idstr, "%sSty", prefix);
    style = (BYTE) DBGetContactSettingByte(NULL, settings_group, idstr, 
		(lf->lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD) | (lf->lfItalic ? DBFONTF_ITALIC : 0) | (lf->lfUnderline ? DBFONTF_UNDERLINE : 0) | lf->lfStrikeOut ? DBFONTF_STRIKEOUT : 0);

    lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
    lf->lfWeight = style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
    lf->lfItalic = (style & DBFONTF_ITALIC) != 0;
    lf->lfUnderline = (style & DBFONTF_UNDERLINE) != 0;
    lf->lfStrikeOut = (style & DBFONTF_STRIKEOUT) != 0;
    
	sprintf(idstr, "%sSet", prefix);
    lf->lfCharSet = DBGetContactSettingByte(NULL, settings_group, idstr, lf->lfCharSet);
    
	lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf->lfQuality = DEFAULT_QUALITY;
    lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	if(lf->lfHeight > 0) {
		HDC hdc = GetDC(0);
		if(flags & FIDF_SAVEPOINTSIZE) {
			lf->lfHeight = -MulDiv(lf->lfHeight,GetDeviceCaps(hdc, LOGPIXELSY), 72);
		} else { // assume SAVEACTUALHEIGHT
			TEXTMETRIC tm;
			HFONT hFont = CreateFontIndirectW(lf);
			HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

			GetTextMetrics(hdc, &tm);

			lf->lfHeight = -(lf->lfHeight - tm.tmInternalLeading);

			SelectObject(hdc, hOldFont);
			DeleteObject(hFont);
		}
		//lf->lfHeight = -MulDiv(lf->lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
		ReleaseDC(0, hdc);
	}
	
	return retval;
}

int CreateFromFontSettingsW(FontSettingsW *fs, LOGFONTW *lf, DWORD flags) {
	GetDefaultFontSettingW(lf, 0);

	wcscpy(lf->lfFaceName, fs->szFace);

    lf->lfWidth = lf->lfEscapement = lf->lfOrientation = 0;
    lf->lfWeight = fs->style & DBFONTF_BOLD ? FW_BOLD : FW_NORMAL;
    lf->lfItalic = (fs->style & DBFONTF_ITALIC) != 0;
    lf->lfUnderline = (fs->style & DBFONTF_UNDERLINE) != 0;
    lf->lfStrikeOut = (fs->style & DBFONTF_STRIKEOUT) != 0;;
    lf->lfCharSet = fs->charset;
    lf->lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf->lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf->lfQuality = DEFAULT_QUALITY;
    lf->lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	lf->lfHeight = fs->size;
	//if(flags & FIDF_SAVEACTUALHEIGHT) {
	//	HDC hdc = GetDC((HWND)CallService(MS_CLUI_GETHWND, 0, 0));
	//	lf->lfHeight = -MulDiv(lf->lfHeight, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	//}

	return 0;
}

void UpdateFontSettingsW(FontIDW *font_id, FontSettingsW *fontsettings) {
	LOGFONTW lf;
	COLORREF colour;
	if(GetFontSettingFromDBW(font_id->dbSettingsGroup, font_id->prefix, &lf, &colour, font_id->flags) && (font_id->flags & FIDF_DEFAULTVALID)) {
		CreateFromFontSettingsW(&font_id->deffontsettings, &lf, font_id->flags);// & ~FIDF_SAVEACTUALHEIGHT);
		colour = font_id->deffontsettings.colour;
	}
	
	fontsettings->style =
		(lf.lfWeight == FW_NORMAL ? 0 : DBFONTF_BOLD) | (lf.lfItalic ? DBFONTF_ITALIC : 0) | (lf.lfUnderline ? DBFONTF_UNDERLINE : 0) | (lf.lfStrikeOut ? DBFONTF_STRIKEOUT : 0);

	fontsettings->size = (char)lf.lfHeight;

	fontsettings->charset = lf.lfCharSet;
	fontsettings->colour = colour;
	wcscpy(fontsettings->szFace, lf.lfFaceName);
}

int RegisterFontW(WPARAM wParam, LPARAM lParam) {
	FontIDW *font_id = (FontIDW *)wParam;
	FontSettingsW fontsettings;

	for(FontIDWList::Iterator i = font_id_list_w.start(); i.has_val(); i.next()) {
		if(wcscmp(i.val().first.group, font_id->group) == 0 && wcscmp(i.val().first.name, font_id->name) == 0)
			if((i.val().first.flags & FIDF_ALLOWREREGISTER) == 0)
				return 1;
	}

    char idstr[256];
	sprintf(idstr, "%sFlags", font_id->prefix);
	DBWriteContactSettingDword(0, font_id->dbSettingsGroup, idstr, font_id->flags);

	UpdateFontSettingsW(font_id, &fontsettings);

	FontIDSettingsWPair p(*font_id, fontsettings);
	font_id_list_w.add(p);

	return 0;
}

int RegisterFont(WPARAM wParam, LPARAM lParam) {
	FontID *font_id = (FontID *)wParam;
	FontIDW font_id_w;
	ConvertFontID(font_id, &font_id_w);
	return RegisterFontW((WPARAM)&font_id_w, 0);
}

int GetFontW(WPARAM wParam, LPARAM lParam) {		
	FontIDW *font_id = (FontIDW *)wParam;
	LOGFONTW *lf = (LOGFONTW *)lParam;
	COLORREF colour;

	for(FontIDWList::Iterator i = font_id_list_w.start(); i.has_val(); i.next()) {
		if(wcsncmp(i.val().first.name, font_id->name, 64) == 0
			&& wcsncmp(i.val().first.group, font_id->group, 64) == 0)
		{
			if(GetFontSettingFromDBW(i.val().first.dbSettingsGroup, i.val().first.prefix, lf, &colour, i.val().first.flags) && (i.val().first.flags & FIDF_DEFAULTVALID)) {
				CreateFromFontSettingsW(&i.val().first.deffontsettings, lf, i.val().first.flags);
				colour = i.val().first.deffontsettings.colour;
			}

			return (int)colour;
		}
	}

	GetDefaultFontSettingW(lf, &colour);
	return (int)colour;
}

int GetFont(WPARAM wParam, LPARAM lParam) {		
	FontID *font_id = (FontID *)wParam;
	LOGFONTA *lf = (LOGFONTA *)lParam;

	FontIDW font_id_w;
	LOGFONTW lfw;
	ConvertFontID(font_id, &font_id_w);
	int ret = GetFontW((WPARAM)&font_id_w, (LPARAM)&lfw);
	ConvertLOGFONT(&lfw, lf);
	return ret;
}

void UpdateColourSettingsW(ColourIDW *colour_id, COLORREF *colour) {
	*colour = (COLORREF)DBGetContactSettingDword(NULL, colour_id->dbSettingsGroup, colour_id->setting, colour_id->defcolour);
}

int RegisterColourW(WPARAM wParam, LPARAM lParam) {
	ColourIDW *colour_id = (ColourIDW *)wParam;
	COLORREF colour;

	for(ColourIDWList::Iterator i = colour_id_list_w.start(); i.has_val(); i.next()) {
		if(wcscmp(i.val().first.group, colour_id->group) == 0 && wcscmp(i.val().first.name, colour_id->name) == 0)
			return 1;
	}

	UpdateColourSettingsW(colour_id, &colour);

	ColourIDColourWPair p(*colour_id, colour);
	colour_id_list_w.add(p);

	return 0;
}

int RegisterColour(WPARAM wParam, LPARAM lParam) {
	ColourID *colour_id = (ColourID *)wParam;
	ColourIDW colour_id_w;
	ConvertColourID(colour_id, &colour_id_w);
	return RegisterColourW((WPARAM)&colour_id_w, 0);
}

int GetColourW(WPARAM wParam, LPARAM lParam) {		
	ColourIDW *colour_id = (ColourIDW *)wParam;
	for(ColourIDWList::Iterator i = colour_id_list_w.start(); i.has_val(); i.next()) {
		if(wcscmp(i.val().first.group, colour_id->group) == 0 && wcscmp(i.val().first.name, colour_id->name) == 0)
			return (int)DBGetContactSettingDword(NULL, i.val().first.dbSettingsGroup, i.val().first.setting, i.val().first.defcolour);
	}

	return -1;
}


int GetColour(WPARAM wParam, LPARAM lParam) {		
	ColourID *colour_id = (ColourID *)wParam;
	ColourIDW colour_id_w;
	ConvertColourID(colour_id, &colour_id_w);
	return GetColourW((WPARAM)&colour_id_w, 0);
}

