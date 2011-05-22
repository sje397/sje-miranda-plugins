#ifndef _OPTIONS_INC
#define _OPTIONS_INC

typedef struct {
	char *username, *password, *host;
	int port;
	bool auto_answer;
	char *cid_name, *cid_number;
	WORD dlgHotkey;
	bool pop_dial_in, pop_dial_out;
	DWORD codecs_preferred, codecs_allowed;
	bool mic_boost;
} Options;

extern Options options;

void PreInitOptions();
void InitOptions();
void DeinitOptions();

#endif
