#include "common.h"
#include "icons.h"
#include "resource.h"

HICON hIconProto, hIconRing;
HANDLE hIcoLibIconsChanged = 0;

int ReloadIcons(WPARAM wParam, LPARAM lParam) {
	hIconProto = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)MODULE "_proto");
	hIconRing = (HICON)CallService(MS_SKIN2_GETICON, 0, (LPARAM)MODULE "_ring");

	return 0;
}

void InitIcons() {
	if(ServiceExists(MS_SKIN2_ADDICON)) {
		SKINICONDESC sid = {0};
		sid.cbSize = sizeof(SKINICONDESC);

		sid.pszSection = MODULE;

#define AddIcon(x,y,z)																				\
		sid.pszDescription = Translate(x);															\
		sid.pszName = y;																			\
		sid.pszDefaultFile = MODULE ".dll";															\
		sid.hDefaultIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(z), IMAGE_ICON, 0, 0, 0);		\
		CallService(MS_SKIN2_ADDICON, 0, (LPARAM)&sid);												\
		sid.iDefaultIndex++;																		

		AddIcon("Protocol", MODULE "_proto", IDI_PROTO);
		AddIcon("Ring", MODULE "_ring", IDI_PROTO);

#undef	AddIcon

		ReloadIcons(0, 0);

		hIcoLibIconsChanged = HookEvent(ME_SKIN2_ICONSCHANGED, ReloadIcons);
	} else {
		hIconProto = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_PROTO), IMAGE_ICON, 0, 0, 0);//LR_LOADTRANSPARENT | LR_LOADMAP3DCOLORS );
		hIconRing = hIconProto;
	}

}

void DeinitIcons() {
	if(hIcoLibIconsChanged) UnhookEvent(hIcoLibIconsChanged);
	else {
		DestroyIcon(hIconProto);
	}
}
