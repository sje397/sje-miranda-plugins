#ifndef SERVICES_INC
#define SERVICES_INC

#include "m_fontservice.h"

#include "collection.h"

int RegisterFont(WPARAM wParam, LPARAM lParam);
int GetFont(WPARAM wParam, LPARAM lParam);

int RegisterColour(WPARAM wParam, LPARAM lParam);
int GetColour(WPARAM wParam, LPARAM lParam);

typedef Pair<FontID, FontSettings> FontIDSettingsPair;
typedef Pair<ColourID, COLORREF> ColourIDColourPair;

typedef Pair<FontIDW, FontSettingsW> FontIDSettingsWPair;
typedef Pair<ColourIDW, COLORREF> ColourIDColourWPair;

typedef SortedDynamicArray< FontIDSettingsWPair > FontIDWList;
typedef SortedDynamicArray< ColourIDColourWPair > ColourIDWList;

bool operator <(const FontIDW &id1, const FontIDW &id2);
bool operator <(const ColourIDW &id1, const ColourIDW &id2);
bool operator ==(const FontIDW &id1, const FontIDW &id2);
bool operator ==(const ColourIDW &id1, const ColourIDW &id2);

//bool operator <(const FontIDSettingsWPair &id1, const FontIDSettingsWPair &id2);
//bool operator <(const ColourIDColourWPair &id1, const ColourIDColourWPair &id2);

extern FontIDWList font_id_list_w;
extern ColourIDWList colour_id_list_w;

int CreateFromFontSettingsW(FontSettingsW *fs, LOGFONTW *lf, DWORD flags);

int RegisterFontW(WPARAM wParam, LPARAM lParam);
int GetFontW(WPARAM wParam, LPARAM lParam);

int RegisterColourW(WPARAM wParam, LPARAM lParam);
int GetColourW(WPARAM wParam, LPARAM lParam);

extern int code_page;

#endif
