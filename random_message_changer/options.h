#ifndef _OPTIONS_INC
#define _OPTIONS_INC

typedef struct {
	int iTimeOut;
	bool cStatus;
	bool cTime;
	bool cAway;
	bool cNa;
	bool cOcc;
	bool cDnd;
	bool cFfc;

	char file_away[STRING_LENGTH];
	char file_na[STRING_LENGTH];
	char file_occ[STRING_LENGTH];
	char file_dnd[STRING_LENGTH];
	char file_ffc[STRING_LENGTH];
} Options;

extern Options options;

void LoadOptions();
void SaveOptions();

#endif
