//---------------------------------------------------------------------------
#include "common.h"
#include "options.h"

PLUGINLINK *pluginLink;
HINSTANCE hInst;

PLUGININFO pluginInfo={
	sizeof(PLUGININFO),
	"Random Message Changer",
	PLUGIN_MAKE_VERSION(0,0,2,2),
	"Loads a random Status-Message from a file.",
	"Christian Weihs & Scott Ellis",
	"mail@scottellis.com.au",
	"2003 Christian Weihs, 2006 Scott Ellis",
	"http://www.scottellis.com.au",
	0,
	0
};

//---------------------------------------------------------------------------
extern "C" BOOL APIENTRY DllMain(HMODULE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	hInst=hinstDLL;
	return TRUE;
}

//---------------------------------------------------------------------------
extern "C" __declspec(dllexport) PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion) {
	return &pluginInfo;
}

UINT TimerId;

void ChangeAllProtoMessages(int statusMode, char *msg) {
	int protoCount,i;
	PROTOCOLDESCRIPTOR **proto;

	CallService(MS_PROTO_ENUMPROTOCOLS,(WPARAM)&protoCount,(LPARAM)&proto);
	for(i=0;i<protoCount;i++)
		CallProtoService(proto[i]->szName,PS_SETAWAYMSG,statusMode,(LPARAM)msg);
}

// sucht eine neue Message heraus.
char *GetRandomLine(char *file, char *buff, int bufflen) {
	if (file == 0 || file[0] == 0 || file[0] == ':') return 0;

	FILE *inFile = fopen(file, "rt");
	if (!inFile){
		KillTimer(NULL, TimerId);
		MessageBox(NULL, Translate("Can't open file!"), Translate("RMC Error"), MB_OK | MB_ICONERROR);
		return 0;
	}
	
	char *line[1000];
	char zeile[2048];
	int i=0;
	while( i < 1000 && !feof(inFile)) {
		fgets(zeile, sizeof(zeile), inFile);
		
		unsigned int x=0;
		bool test=false;
		
		while(x < sizeof(zeile) && zeile[x]!=0) {
			if(zeile[x] == '#' || zeile[x] == '\r' || zeile[x] == '\n') {
				zeile[x] = 0;
				x = sizeof(zeile);
			} else {
				if(zeile[x]!= ' ') test=true;

				if (zeile[x] == '\\' && zeile[x+1] == 'n') {
					zeile[x+1]= '\n';
					zeile[x]= '\r';
				}
			}
			x++;
		}

		if(test) line[i++]=strdup(zeile);
	}
	fclose(inFile);

	if(i != 0) {
		int num = rand()%i;
		strncpy(buff,line[num],bufflen);
		for(int x=0;x<i;x++)
			free(line[x]);
		return buff;
	}
	
	return 0;
}

void CALLBACK TimerCall(HWND hwnd, UINT message, UINT idEvent, DWORD dwTime) {
	char *file;
	char *mode;
	char *mode2;
	bool enabled;
	int mirandaStatus=CallService(MS_CLIST_GETSTATUSMODE,0,0);
	
	switch(mirandaStatus) {
		case ID_STATUS_AWAY:
			file = options.file_away;
			enabled = options.cAway;
			mode="AwayDefault";
			mode2="AwayMsg";
			break;
		case ID_STATUS_NA:
			file = options.file_na;
			enabled = options.cNa;
			mode="NaDefault";
			mode2="NaMsg";
			break;
		case ID_STATUS_OCCUPIED:
			file = options.file_occ;
			enabled = options.cOcc;
			mode="OccupiedDefault";
			mode2="OccupiedMsg";
			break;
		case ID_STATUS_DND:
			file = options.file_dnd;
			enabled = options.cDnd;
			mode="DndDefault";
			mode2="DndMsg";
			break;
		case ID_STATUS_FREECHAT:
			file = options.file_ffc;
			enabled = options.cFfc;
			mode="FreeChatDefault";
			mode2="FreeChatMsg";
			break;
		default:
			enabled=false;
	}

	if (enabled) {
		char msg[2048];
		GetRandomLine(file, msg, sizeof(msg));
		if (msg!=0) {
			ChangeAllProtoMessages(mirandaStatus, msg);
			DBWriteContactSettingString(NULL, "SRAway", mode, msg);
			DBWriteContactSettingString(NULL, "SRAway", mode2, msg);
		}
	}
}

//setzt den Timer neu
void SetChangingTime() {
	if(TimerId) KillTimer(NULL, TimerId);
	if(options.cTime) TimerId = SetTimer(0, 0, (UINT)(options.iTimeOut*1000*60), TimerCall);
}

// Was tun, wenn der Status sich ändert?
int StatusChanged(WPARAM wParam,LPARAM lParam) {
	SetChangingTime();
	if(options.cStatus) TimerCall(0, 0, 0, 0);	
	return 0;
}

//---------------------------------------------------------------------------
HANDLE hEventStsChange = 0;
extern "C" int __declspec(dllexport) Load(PLUGINLINK *link) {
	pluginLink=link;

	// Events verbinden
	hEventStsChange = HookEvent(ME_CLIST_STATUSMODECHANGE,StatusChanged);

	srand((unsigned int)time(0));
	//Die Werte aus der Datenbank laden
	LoadOptions();

	// test crash for attache
	//int *i = 0;
	//*i = 0;
	
	return 0;
}

//---------------------------------------------------------------------------
extern "C" int __declspec(dllexport) Unload() {
	UnhookEvent(hEventStsChange);
	KillTimer(NULL, TimerId);
	return 0;
}
//---------------------------------------------------------------------------
