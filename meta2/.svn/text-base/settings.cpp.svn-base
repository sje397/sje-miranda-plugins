#include "common.h"
#include "settings.h"
#include "core_functions.h"

MIRANDASERVICE oldGetSetting, oldGetSettingStr, oldGetSettingStatic;

/*
int ServiceFuncRedirect(const char *service,WPARAM wParam, LPARAM lParam) {
	if(wParam ==0 
		|| strncmp(service, "DB/Contact/", 11) != 0
		|| (strcmp(MS_DB_CONTACT_GETSETTING, service) != 0 
			&& strcmp(MS_DB_CONTACT_GETSETTING_STR, service) != 0
			&& strcmp(MS_DB_CONTACT_GETSETTINGSTATIC, service) != 0
			&& strcmp(MS_DB_CONTACT_WRITESETTING, service) != 0
			&& strcmp(MS_DB_CONTACT_DELETESETTING, service) != 0
			//&& strcmp(MS_DB_CONTACT_ENUMSETTINGS, service) != 0
			)
		)
	{
		return coreCallService(service, wParam, lParam);
	}

	HANDLE hContact = (HANDLE)wParam;
	if(strcmp(MS_DB_CONTACT_ENUMSETTINGS, service) == 0) {
		DBCONTACTENUMSETTINGS *ces = (DBCONTACTENUMSETTINGS *)lParam;
		if(IsMetacontact(hContact) && strcmp(ces->szModule, MODULE) != 0) {
			HANDLE hSub = Meta_GetActive(hContact);
			return coreCallService(service, (WPARAM)hSub, lParam);
		} else
			return coreCallService(service, wParam, lParam);
	}
	
	const char *szSetting = 0, *szModule = 0;
	bool read; // true for get setting, false for write setting
	if(strcmp(MS_DB_CONTACT_WRITESETTING, service) == 0 || strcmp(MS_DB_CONTACT_DELETESETTING, service) == 0) {
		read = false;
		DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
		szSetting = cws->szSetting;
		szModule = cws->szModule;
	} else { // get setting
		read = true;
		DBCONTACTGETSETTING *cgs = (DBCONTACTGETSETTING *)lParam;
		szSetting = cgs->szSetting;
		szModule = cgs->szModule;
	}

	if(szModule == 0 || szSetting == 0
		|| strcmp(szModule, "Protocol") == 0
		|| strcmp(szModule, "_Filter") == 0
		|| strcmp(szModule, "CList") == 0
		|| (strcmp(szModule, MODULE) == 0
			&& (strcmp(szSetting, META_ID) == 0 
				|| strcmp(szSetting, "IsSubcontact") == 0 
				|| strcmp(szSetting, "Default") == 0 
				|| strcmp(szSetting, "Handle") == 0 
				|| strcmp(szSetting, "WindowOpen") == 0 
				|| strcmp(szSetting, "ParentMetaID") == 0 
				|| strcmp(szSetting, "Default") == 0 
				|| strcmp(szSetting, "ForceDefault") == 0 
				|| strcmp(szSetting, "ForceSend") == 0 
				|| strcmp(szSetting, "TempDefault") == 0 
				|| strcmp(szSetting, "Status") == 0))
		|| !IsMetacontact(hContact))
	{
		return coreCallService(service, wParam, lParam);
	}

	HANDLE hMeta = hContact;
	HANDLE hSub = Meta_GetActive(hMeta);
	char *subProto;
	if(hSub == 0 || (subProto = ContactProto(hSub)) == 0) // no most online - fail
		return coreCallService(service, wParam, lParam);

	int ret = 1;
	if(read) {
		DBCONTACTGETSETTING *cgs = (DBCONTACTGETSETTING *)lParam;
		DBCONTACTGETSETTING cgs_save = *cgs;
		if((ret = coreCallService(service, (WPARAM)hMeta, lParam)) != 0) {
			*cgs = cgs_save;
			// if the setting does not exist in the metacontact, get it from the most online subcontact
			if((ret = coreCallService(service, (WPARAM)hSub, lParam)) != 0) {
				// if it does not exist in the subcontact and we're using the meta proto module, try changing the module to the subcontact proto module
				if(strcmp(szModule, MODULE) == 0) {
					if(subProto) {
						*cgs = cgs_save;
						cgs->szModule = subProto;
						ret = coreCallService(service, (WPARAM)hSub, lParam);
					}
				}
			}
		}
	} else {
		// write to the sub?
		
		//DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)lParam;
		//if(strcmp(szModule, MODULE) == 0)
		//	cws->szModule = subProto;
		//ret = coreCallService(service, (WPARAM)hSub, lParam);
	}

	return ret;
}
*/

int MyOverride(WPARAM wParam, LPARAM lParam, MIRANDASERVICE oldService) {
	HANDLE hMeta = (HANDLE)wParam;
	DBCONTACTGETSETTING *cgs = (DBCONTACTGETSETTING *)lParam;
	if(cgs->szModule == 0 || cgs->szSetting == 0
		|| strcmp(cgs->szModule, "Protocol") == 0
		|| strcmp(cgs->szModule, "_Filter") == 0
		|| strcmp(cgs->szModule, "CList") == 0
		|| (strcmp(cgs->szModule, MODULE) == 0
			&& (strcmp(cgs->szSetting, META_ID) == 0 
				|| strcmp(cgs->szSetting, "IsSubcontact") == 0 
				|| strcmp(cgs->szSetting, "Default") == 0 
				|| strcmp(cgs->szSetting, "Handle") == 0 
				|| strcmp(cgs->szSetting, "WindowOpen") == 0 
				|| strcmp(cgs->szSetting, "ParentMetaID") == 0 
				|| strcmp(cgs->szSetting, "Default") == 0 
				|| strcmp(cgs->szSetting, "ForceDefault") == 0 
				|| strcmp(cgs->szSetting, "ForceSend") == 0 
				|| strcmp(cgs->szSetting, "TempDefault") == 0 
				|| strcmp(cgs->szSetting, "Status") == 0))
		|| !IsMetacontact(hMeta))
	{
		return oldService(wParam, lParam);
	}

	HANDLE hSub = Meta_GetActive(hMeta);
	char *subProto;
	if(hSub == 0 || (subProto = ContactProto(hSub)) == 0) // no most online - fail
		return oldService(wParam, lParam);

	int ret;
	DBCONTACTGETSETTING cgs_save = *cgs;
	if((ret = oldService((WPARAM)hMeta, lParam)) != 0) {
		*cgs = cgs_save;
		// if the setting does not exist in the metacontact, get it from the most online subcontact
		if((ret = oldService((WPARAM)hSub, lParam)) != 0) {
			// if it does not exist in the subcontact and we're using the meta proto module, try changing the module to the subcontact proto module
			if(strcmp(cgs->szModule, MODULE) == 0) {
				if(subProto) {
					*cgs = cgs_save;
					cgs->szModule = subProto;
					ret = oldService((WPARAM)hSub, lParam);
				}
			}
		}
	}

	return ret;
}

int MyGetSetting(WPARAM wParam, LPARAM lParam) {
	return MyOverride(wParam, lParam, oldGetSetting);
}

int MyGetSettingStr(WPARAM wParam, LPARAM lParam) {
	return MyOverride(wParam, lParam, oldGetSettingStr);
}

int MyGetSettingStatic(WPARAM wParam, LPARAM lParam) {
	return MyOverride(wParam, lParam, oldGetSettingStatic);
}

void InitSettings() {
	oldGetSetting = (MIRANDASERVICE)CallService(MS_PLUGINS_HOOKSERVICE, (WPARAM)MS_DB_CONTACT_GETSETTING, (LPARAM)MyGetSetting);
	oldGetSettingStr = (MIRANDASERVICE)CallService(MS_PLUGINS_HOOKSERVICE, (WPARAM)MS_DB_CONTACT_GETSETTING_STR, (LPARAM)MyGetSettingStr);
	oldGetSettingStatic = (MIRANDASERVICE)CallService(MS_PLUGINS_HOOKSERVICE, (WPARAM)MS_DB_CONTACT_GETSETTINGSTATIC, (LPARAM)MyGetSettingStatic);
}

void DeinitSettings() {
	CallService(MS_PLUGINS_HOOKSERVICE, (WPARAM)MS_DB_CONTACT_GETSETTING, (LPARAM)oldGetSetting);
	CallService(MS_PLUGINS_HOOKSERVICE, (WPARAM)MS_DB_CONTACT_GETSETTING_STR, (LPARAM)oldGetSettingStr);
	CallService(MS_PLUGINS_HOOKSERVICE, (WPARAM)MS_DB_CONTACT_GETSETTINGSTATIC, (LPARAM)oldGetSettingStatic);
}