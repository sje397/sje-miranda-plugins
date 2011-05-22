#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#include "common.h"

typedef struct Options_tag {
	BYTE dummy;
} Options;

extern Options options;

int OptInit(WPARAM wParam, LPARAM lParam);

void LoadOptions();
void SaveOptions();

#endif
