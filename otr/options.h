#ifndef _OPTIONS_INC
#define _OPTIONS_INC

#define CONTACT_DEFAULT_POLICY 	0xFFFF

typedef enum {ED_POP, ED_BAL, ED_MB} ErrorDisplay;

typedef struct {
	OtrlPolicy default_policy;
	ErrorDisplay err_method;
	bool prefix_messages;
	bool msg_inline;
	char prefix[64];
	
	bool delete_history;
	bool timeout_finished;
	
	bool end_offline, end_window_close;
} Options;

extern Options options;

int OptInit(WPARAM wParam, LPARAM lParam);

void LoadOptions();

#endif
