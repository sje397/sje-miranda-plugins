#ifndef _OPTIONS_INC
#define _OPTIONS_INC

typedef struct {
	bool show_titlebar;
	bool tab_icon;
	bool ontop;
} Options;

extern Options options;

void InitOptions();

void EnableTitleBar();

#endif
