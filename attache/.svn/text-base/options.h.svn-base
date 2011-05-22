#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#include "version.h"

typedef struct Options_tag {
	bool upload;
	bool confirm_upload;
	bool delete_uploaded;
	TCHAR reporter_id[512];
} Options;

extern Options options;

int OptInit(WPARAM wParam, LPARAM lParam);

void LoadOptions();
void SaveOptions();

#endif
