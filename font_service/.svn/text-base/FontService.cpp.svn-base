// Copyright Scott Ellis (mail@scottellis.com.au) 2005
// This software is licenced under the GPL (General Public Licence)
// available at http://www.gnu.org/copyleft/gpl.html

#include "common.h"
#include "FontService.h"
#include "m_fontservice.h"

#include "module_fonts.h"

PLUGININFO pluginInfo={
		sizeof(PLUGININFO),
#ifndef _UNICODE
		"FontService (ANSI)",
#else
		"FontService",
#endif
	    PLUGIN_MAKE_VERSION(0, 2, 0, 2),
		"Provides a UI for font settings, and services to plugins for font control.",
		"S. Ellis",
		"mail@scottellis.com.au",
		"© 2005 S. Ellis",
		"www.scottellis.com.au",
		0,
		0
};

HINSTANCE hInstance;	
PLUGINLINK *pluginLink;	
struct MM_INTERFACE memoryManagerInterface;

HANDLE hFontReloadEvent, hColourReloadEvent;

extern "C" FONTSERVICE_API PLUGININFO* MirandaPluginInfo(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	hInstance = (HINSTANCE)hModule;
	DisableThreadLibraryCalls(hInstance);
    return TRUE;
}


int ModulesLoaded(WPARAM wParam, LPARAM lParam) {
	if(!GetModuleHandle(_T("CLIST_MODERN")))
		RegisterCListFonts();

	if(GetModuleHandle(_T("SRMM")))
		RegisterSRMMFonts();
	if(GetModuleHandle(_T("CHAT")))
		RegisterChatFonts();

	return 0;
}

extern "C" FONTSERVICE_API int Load(PLUGINLINK *link)
{
	pluginLink=link;
    memset(&memoryManagerInterface, 0, sizeof(memoryManagerInterface));
    memoryManagerInterface.cbSize = sizeof(memoryManagerInterface);
    CallService(MS_SYSTEM_GET_MMI, 0, (LPARAM) & memoryManagerInterface);

	HookEvent(ME_OPT_INITIALISE, OptInit);

	CreateServiceFunction(MS_FONT_REGISTER, RegisterFont);
	CreateServiceFunction(MS_FONT_GET, GetFont);

	CreateServiceFunction(MS_COLOUR_REGISTER, RegisterColour);
	CreateServiceFunction(MS_COLOUR_GET, GetColour);

	CreateServiceFunction(MS_FONT_REGISTERW, RegisterFontW);
	CreateServiceFunction(MS_FONT_GETW, GetFontW);

	CreateServiceFunction(MS_COLOUR_REGISTERW, RegisterColourW);
	CreateServiceFunction(MS_COLOUR_GETW, GetColourW);

	hFontReloadEvent = CreateHookableEvent(ME_FONT_RELOAD);
	hColourReloadEvent = CreateHookableEvent(ME_COLOUR_RELOAD);

	if(ServiceExists(MS_LANGPACK_GETCODEPAGE))
		code_page = CallService(MS_LANGPACK_GETCODEPAGE, 0, 0);

	// do last for silly dyna plugin
	HookEvent(ME_SYSTEM_MODULESLOADED, ModulesLoaded);
	return 0;
}

extern "C" FONTSERVICE_API int Unload(PLUGINLINK *link)
{
	DestroyHookableEvent(hFontReloadEvent);
	DestroyHookableEvent(hColourReloadEvent);
	return 0;
}
