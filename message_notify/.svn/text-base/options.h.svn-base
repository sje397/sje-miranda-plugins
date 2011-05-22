#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#define NOTIFY_NONE			0
#define NOTIFY_CLOSED		1
#define NOTIFY_NFORE		2
#define NOTIFY_ALWAYS		3
/*
#define ID_STATUS_ONLINE                40072
#define ID_STATUS_AWAY                  40073
#define ID_STATUS_DND                   40074
#define ID_STATUS_NA                    40075
#define ID_STATUS_OCCUPIED              40076
#define ID_STATUS_FREECHAT              40077
#define ID_STATUS_INVISIBLE             40078
#define ID_STATUS_ONTHEPHONE            40079
#define ID_STATUS_OUTTOLUNCH            40080
*/

#define MNNF_POPUP		1
#define MNNF_SPEAK		2

typedef struct Options_tag {
	int notify_when;
	bool consider_tabs;
	bool close_win;
	bool disable_status[9];
	bool show_msg;
	int flags;
} Options;

extern Options options;

int OptInit(WPARAM wParam, LPARAM lParam);

void LoadOptions();
void SaveOptions();

#endif
