#ifndef _OPTIONS_INC
#define _OPTIONS_INC

typedef struct {
	int dummy;
} Options;

extern Options options;

void InitOptions();
void DeinitOptions();

#endif
