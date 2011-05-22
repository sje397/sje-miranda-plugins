#ifndef _OPTIONS_INC
#define _OPTIONS_INC

typedef struct {
	int vk;
	bool mod_shift, mod_alt, mod_ctrl, mod_win;

	bool hide_if_visible;
} Options;

extern Options options;

void InitOptions();

#endif
