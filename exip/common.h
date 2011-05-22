#ifndef _COMMON_H
#define _COMMON_H

#define MODULE		"ExIP"

#include <windows.h>
#include <stdio.h>

#include "../../include/newpluginapi.h"
#include "../../include/m_options.h"
#include "../../include/m_langpack.h"
#include "../../include/m_system.h"
#include "../../include/m_netlib.h"
#include "../../include/m_database.h"

#include "../../include/m_clui.h"

extern HANDLE hNetlibUser;
extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;

extern HANDLE mainThread;

#endif
