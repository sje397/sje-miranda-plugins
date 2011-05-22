#ifndef _UTILS_INC
#define _UTILS_INC

extern HICON hProtoIcon, hLockIcon, hUnlockIcon;

void InitUtils();
void DeinitUtils();

void ShowPopup( const char* line1, const char* line2, int timeout);
void ShowWarning(char *msg);
void ShowError(char *msg);

bool CreatePath(const char *path);

#endif
