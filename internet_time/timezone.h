#ifndef _TIMEZONE_H
#define _TIMEZONE_H

#define TZREG "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones"
#define TZREG_9X "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Time Zones"
//#define TZREG2 "SYSTEM\\CurrentControlSet\\Control\\TimeZoneInformation"
#define MAX_SIZE 512

#include <vector>
#include <algorithm>

#include <wchar.h>

struct REG_TZI {
	long Bias;
	long StandardBias;
	long DaylightBias;
	SYSTEMTIME StandardDate; 
	SYSTEMTIME DaylightDate;
};


struct LS_TZREG {
	TCHAR tcName[MAX_SIZE];
	TCHAR tcDisp[MAX_SIZE];
	TCHAR tcDLT[MAX_SIZE];
	TCHAR tcSTD[MAX_SIZE];
	TCHAR MapID[MAX_SIZE];
	DWORD Index;
	DWORD ActiveTimeBias;
	//TIME_ZONE_INFORMATION TZI;
	REG_TZI TZI;

	unsigned int list_index;

	bool operator<(const LS_TZREG &other);
};

typedef std::vector< LS_TZREG > TimeList;

extern TimeList timezone_list, geo_timezone_list;
bool build_timezone_list();

bool get_reg_tzi_for_gmt(REG_TZI &reg_tzi);
bool get_reg_tzi_for_central_european(REG_TZI &reg_tzi);

#endif
