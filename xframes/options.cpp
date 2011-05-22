#include "common.h"
#include "options.h"
#include "frames.h"

Options options;

#define WMU_SETFRAMEDATA			(WM_USER + 0x100)

struct FrameListNode *temp_frame_list = 0;

static BOOL CALLBACK DlgProcOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		// copy original frame list
		{
			EnterCriticalSection(&cs);
			struct FrameListNode *current = frame_list, *newnode;
			while(current) {
				newnode = (struct FrameListNode *)malloc(sizeof(struct FrameListNode));
				*newnode = *current;
				newnode->next = temp_frame_list;
				temp_frame_list = newnode;

				// fill list with frame names while we're here
				int index = SendDlgItemMessage(hwndDlg, IDC_LST_FRAMES, LB_ADDSTRING, 0, (LPARAM)newnode->frame_data.name);
				SendDlgItemMessage(hwndDlg, IDC_LST_FRAMES, LB_SETITEMDATA, (WPARAM)index, (LPARAM)newnode);

				current = current->next;
			}
			LeaveCriticalSection(&cs);
		}

		if(temp_frame_list) {
			SendDlgItemMessage(hwndDlg, IDC_LST_FRAMES, LB_SETCURSEL, 0, 0);
			SendMessage(hwndDlg, WMU_SETFRAMEDATA, 0, 0);
		}

		return FALSE;
	case WMU_SETFRAMEDATA:
		{
			int sel = SendDlgItemMessage(hwndDlg, IDC_LST_FRAMES, LB_GETCURSEL, 0, 0);
			if(sel >= 0) {
				struct FrameListNode *node = (struct FrameListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_FRAMES, LB_GETITEMDATA, (WPARAM)sel, 0);
				if(node) {
					CheckDlgButton(hwndDlg, IDC_CHK_VIS, (node->frame_data.Flags & F_VISIBLE) != 0);
					CheckDlgButton(hwndDlg, IDC_CHK_MOVE, node->moving ? TRUE : FALSE);
					CheckDlgButton(hwndDlg, IDC_CHK_HIDE, node->hiding ? TRUE : FALSE);

					int rad;
					if(!node->style_override) rad = 0;
					else if(node->overrideExStyle & WS_EX_TOOLWINDOW) rad = 1;
					else if((node->overrideStyle & WS_CAPTION) == WS_CAPTION) rad = 2;
					else if(node->overrideStyle & WS_BORDER) rad = 3;
					else rad = 4;

					CheckDlgButton(hwndDlg, IDC_RAD_BCOPY, rad == 0 ? TRUE : FALSE);
					CheckDlgButton(hwndDlg, IDC_RAD_BTOOL, rad == 1 ? TRUE : FALSE);
					CheckDlgButton(hwndDlg, IDC_RAD_BNORM, rad == 2 ? TRUE : FALSE);
					CheckDlgButton(hwndDlg, IDC_RAD_BTHIN, rad == 3 ? TRUE : FALSE);
					CheckDlgButton(hwndDlg, IDC_RAD_BNONE, rad == 4 ? TRUE : FALSE);
				}
			}
		}
		return TRUE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == LBN_SELCHANGE ) {
			SendMessage(hwndDlg, WMU_SETFRAMEDATA, 0, 0);
		} else if ( HIWORD( wParam ) == BN_CLICKED ) {
			int sel = SendDlgItemMessage(hwndDlg, IDC_LST_FRAMES, LB_GETCURSEL, 0, 0);
			if(sel >= 0) {
				struct FrameListNode *node = (struct FrameListNode *)SendDlgItemMessage(hwndDlg, IDC_LST_FRAMES, LB_GETITEMDATA, (WPARAM)sel, 0);

				switch( LOWORD( wParam )) {
					case IDC_CHK_VIS:
						if(IsDlgButtonChecked(hwndDlg, IDC_CHK_VIS))
							node->frame_data.Flags |= F_VISIBLE;
						else
							node->frame_data.Flags &= ~F_VISIBLE;
						break;
					case IDC_CHK_MOVE:
						node->moving = (IsDlgButtonChecked(hwndDlg, IDC_CHK_MOVE) ? true : false);
						break;
					case IDC_CHK_HIDE:
						node->hiding = (IsDlgButtonChecked(hwndDlg, IDC_CHK_HIDE) ? true : false);
						break;

					case IDC_RAD_BCOPY:
						node->style_override = false;
						break;
					case IDC_RAD_BTOOL:
						node->style_override = true;
						node->overrideStyle = WS_CAPTION  | WS_SYSMENU | WS_THICKFRAME;
						node->overrideExStyle = WS_EX_TOOLWINDOW | WS_EX_LAYERED;
						break;
					case IDC_RAD_BNORM:
						node->style_override = true;
						node->overrideStyle = WS_CAPTION  | WS_SYSMENU | WS_THICKFRAME;
						node->overrideExStyle = WS_EX_LAYERED;
						break;
					case IDC_RAD_BTHIN:
						node->style_override = true;
						node->overrideStyle = WS_BORDER;
						node->overrideExStyle = WS_EX_LAYERED;
						break;
					case IDC_RAD_BNONE:
						node->style_override = true;
						node->overrideStyle = 0;
						node->overrideExStyle = WS_EX_LAYERED;
						break;
				}
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				return TRUE;
			}
		}
		break;
	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->code == PSN_APPLY ) {

			EnterCriticalSection(&cs);
			// save old frame list
			struct FrameListNode *old_frame_list = frame_list;

			frame_list = 0;
			// copy temp frame list to real frame list
			if(temp_frame_list) {
				struct FrameListNode *current = temp_frame_list, *newnode, *oldnode;
				while(current) {
					newnode = (struct FrameListNode *)malloc(sizeof(struct FrameListNode));
					*newnode = *current;
					newnode->next = frame_list;
					frame_list = newnode;

					// keep old frame size info
					oldnode = (struct FrameListNode *)GetWindowLong(frame_list->hwndParent, GWL_USERDATA);
					newnode->height = oldnode->height;
					newnode->width = oldnode->width;
					newnode->frame_data.height = oldnode->frame_data.height;
					newnode->off_x = oldnode->off_x;
					newnode->off_y = oldnode->off_y;

					SetWindowLong(frame_list->hwndParent, GWL_USERDATA, (LONG)newnode);
					SaveFrameState(newnode);

					current = current->next;
				}
			}

			// delete old frame list
			if(old_frame_list) {
				struct FrameListNode *current;
				while(old_frame_list) {
					current = old_frame_list;
					old_frame_list = old_frame_list->next;
					free(current);
				}
			}

			LeaveCriticalSection(&cs);

			PositionFrames();
			
			return TRUE;
		}
		break;
	case WM_DESTROY:
		if(temp_frame_list) {
			struct FrameListNode *current;
			while(temp_frame_list) {
				current = temp_frame_list;
				temp_frame_list = temp_frame_list->next;
				free(current);
			}
		}

		break;
	}
	return FALSE;

}


int OptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize						= sizeof(odp);
	odp.position					= -790000000;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCEA(IDD_OPT);
	odp.pszTitle					= Translate(MODULE);
	odp.pszGroup					= Translate("Plugins"); // Services? Customize?
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts;
	
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );

	return 0;
}

void LoadOptions() {
}

void SaveOptions() {
}


