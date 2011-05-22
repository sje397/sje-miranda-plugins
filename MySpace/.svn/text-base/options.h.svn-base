#ifndef _OPTIONS_INC
#define _OPTIONS_INC

typedef enum {PM_ANYONE, PM_CLIST} PrivacyMode;
typedef enum {OMM_EVERYONE, OMM_CLIST, OMM_NOONE} OfflineMessageMode;


typedef struct {
	TCHAR email[256];
	TCHAR pw[256];

	// im options
	bool sound;
	PrivacyMode privacy_mode;
	bool show_only_to_list;
	OfflineMessageMode offline_message_mode;
	bool show_avatar;
	char im_name[256];
	bool handle_links;
} Options;

extern Options options;

void InitOptions();
void DeinitOptions();

#endif
