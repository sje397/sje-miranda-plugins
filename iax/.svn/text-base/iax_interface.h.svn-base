#ifndef _IAX_INTERFACE_INC
#define _IAX_INTERFACE_INC

#include "iaxclient/lib/iaxclient.h"


void SendDTMF(char c);
void Call(char *number);
void HangUp();
void SelectLine(int line);
void AnswerCall(int callNo);
void SetLineNo(int line);
int GetLineNo();

int GetLineState(int line);

char *GetLineCIDNum(int callNo);
char *GetLineCIDName(int callNo);

void Connect();
void Disconnect();

bool InitIAXInterface();
void DeinitIAXInterface();

#endif