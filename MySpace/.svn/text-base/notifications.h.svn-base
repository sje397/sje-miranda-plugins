#ifndef _NOTIFICATIONS_INC
#define _NOTIFICATIONS_INC

void NotifyMail();
void NotifyBlogComment();
void NotifyProfileComment();
void NotifyFriendRequest();
void NotifyPictureComment();
void NotifyBlogSubscriptPost();

void NotifyUnknown(char *name, char *value);

void InitNotifications();
void DeinitNotifications();

extern char *zap_array[];

void NotifyZapRecv(HANDLE hContact, int zap_num);
void NotifyZapSend(HANDLE hContact, int zap_num);

extern char popup_class_name[];

#endif

