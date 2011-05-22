#ifndef _SERVICES_INC
#define _SERVICES_INC

void SetStatusAndACK(int new_status);
void SetContactStatus(int status);

HANDLE FindContact(char *num);
HANDLE CreateContact(char *num, char *name, bool temp);

void InitServices();
void DeinitServices();


#endif
