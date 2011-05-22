#ifndef _PING_OPTIONS
#define _PING_OPTIONS

#include "common.h"
#include "resource.h"
#include <commctrl.h>

typedef struct {
	int dummy;
} Options;

extern Options options;

int OptInit(WPARAM wParam,LPARAM lParam);

void LoadOptions();
void SaveOptions();

#endif