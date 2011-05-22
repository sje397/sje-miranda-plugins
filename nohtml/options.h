#ifndef _OPTIONS_INC
#define _OPTIONS_INC

// flags for 'apply_to' value below
#define ATF_ALL		0x01
#define ATF_OTR		0x02

typedef struct {
	bool filter_in, filter_out, bbcodes, html;
	int apply_to; // bitwize 'or' of ATF_* flags above - 'ALL' overrides all others
} Options;

extern Options options;

void InitOptions();
void DeinitOptions();

#endif
