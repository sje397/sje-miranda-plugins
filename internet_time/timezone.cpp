#include "stdafx.h"
#include "timezone.h"

TimeList timezone_list, geo_timezone_list;

bool LS_TZREG::operator<(const LS_TZREG &other) {
	//return Index < other.Index;
	return TZI.Bias < other.TZI.Bias;
}

bool build_timezone_list() {
	HKEY HKlmtz;
	HKEY KKtz;
	DWORD dwIndex = 0;
	CHAR tcName[MAX_SIZE];
	DWORD dwcbName = MAX_SIZE;
	DWORD dwcbValue;
	DWORD dwcbSTD;
	DWORD dwcbDLT;
	LS_TZREG Temp;
	FILETIME ftLastWrite;
	unsigned int list_index = 0;
	bool win9x = false;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TZREG, 0, KEY_ENUMERATE_SUB_KEYS, &HKlmtz) != ERROR_SUCCESS) {
		win9x = true;
		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TZREG_9X, 0, KEY_ENUMERATE_SUB_KEYS, &HKlmtz) != ERROR_SUCCESS)
			return false;
	}

	while(RegEnumKeyEx(HKlmtz, dwIndex++, tcName, &dwcbName, NULL, NULL, NULL, &ftLastWrite) != ERROR_NO_MORE_ITEMS) {

		if(RegOpenKeyEx(HKlmtz, tcName, 0,KEY_QUERY_VALUE, &KKtz) != ERROR_SUCCESS) {
			RegCloseKey(HKlmtz);
			return false;
		}

		strncpy(Temp.tcName, tcName, MAX_SIZE);
		dwcbValue = MAX_SIZE;
		RegQueryValueEx(KKtz,"Display",NULL,NULL,(BYTE*)Temp.tcDisp,&dwcbValue);
		dwcbDLT = MAX_SIZE;
		RegQueryValueEx(KKtz,"Dlt",NULL,NULL,(BYTE*)Temp.tcDLT,&dwcbDLT);
		dwcbSTD = MAX_SIZE;
		RegQueryValueEx(KKtz,"Std",NULL,NULL,(BYTE*)Temp.tcSTD,&dwcbSTD);
		dwcbValue = MAX_SIZE;
		RegQueryValueEx(KKtz,"MapID",NULL,NULL,(BYTE*)Temp.MapID,&dwcbValue);
		if(!win9x) {
			dwcbValue = sizeof(DWORD);
			RegQueryValueEx(KKtz,"Index",NULL,NULL,(BYTE*)&Temp.Index,&dwcbValue);
		}
		dwcbValue = sizeof(Temp.TZI);
		RegQueryValueEx(KKtz,"TZI",NULL,NULL,(BYTE*)&Temp.TZI,&dwcbValue);

		RegCloseKey(KKtz);

		Temp.list_index = list_index;
		timezone_list.push_back(Temp);

		dwcbName = MAX_SIZE;
		list_index++;
	}

	RegCloseKey(HKlmtz);

	geo_timezone_list = timezone_list;
	std::sort(geo_timezone_list.begin(), geo_timezone_list.end());
	
	return true;
}

bool get_reg_tzi_for_gmt(REG_TZI &reg_tzi) {
	for(TimeList::iterator i = timezone_list.begin(); i != timezone_list.end(); i++) {
		if(!strcmp(i->tcName, "GMT Standard Time")) {
			reg_tzi = i->TZI;
			return true;
		}
	}
	
	return false;
}

bool get_reg_tzi_for_central_european(REG_TZI &reg_tzi) {
	// for language independece, look for a timezone with Bias == -60 - daylight savings is ignored anyway
	for(TimeList::iterator i = timezone_list.begin(); i != timezone_list.end(); i++) {
		if(i->TZI.Bias == -60) {
			reg_tzi = i->TZI;
			return true;
		}
	}

	return false;
}
