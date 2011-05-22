#include <m_metacontacts.h>

INT_PTR MetaAPI_GetMeta(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetDefault(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetDefaultNum(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetMostOnline(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetActiveContact(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetNumContacts(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetContact(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_SetDefaultContactNum(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_SetDefaultContact(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_ForceSendContactNum(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_ForceSendContact(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_UnforceSendContact(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_ForceDefault(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetForceState(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_GetProtoName(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_ConvertToMeta(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_AddToMeta(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_RemoveFromMeta(WPARAM wParam, LPARAM lParam);
INT_PTR MetaAPI_DisableHiddenGroup(WPARAM wParam, LPARAM lParam);


void FireSubcontactsChanged(HANDLE hMeta);

extern bool meta_group_hack_disabled;
void InitAPI();
void DeinitAPI();

