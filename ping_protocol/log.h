#ifndef _PING_LOG
#define _PING_LOG

#include <string>
#include <fstream>
#include "common.h"

int Log(WPARAM wParam, LPARAM lParam);
int GetLogFilename(WPARAM wParam, LPARAM lParam);
int SetLogFilename(WPARAM wParam, LPARAM lParam);
int ViewLogData(WPARAM wParam, LPARAM lParam);

#endif
