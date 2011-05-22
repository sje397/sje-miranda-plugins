#include "common.h"
#include "options.h"
#include "utils.h"

Options options;

#define WMU_REFRESHLIST			(WM_USER + 241)
#define CONTACT_DEFAULT_POLICY 	0xFFFF

#include <map>
typedef std::map<HANDLE, OtrlPolicy> ContactPolicyMap;
typedef std::map<HANDLE, char *> ContactFingerprintMap;

HANDLE hFolderPath = 0;

void SetFilenames(const char *path) {
	strcpy(fingerprint_store_filename, path);	
	strcpy(private_key_filename, path);	
	strcat(fingerprint_store_filename, "\\");		
	strcat(private_key_filename, "\\");		

	char mod_prefix[128];
	CallService(MS_DB_GETPROFILENAME, 128, (LPARAM)mod_prefix);
	char *p = strrchr(mod_prefix, '.');	// cut off extention if present
	if(p) *p = 0;
	
	strcat(fingerprint_store_filename, mod_prefix);
	strcat(private_key_filename, mod_prefix);
	strcat(fingerprint_store_filename, "_");
	strcat(private_key_filename, "_");

	strcat(fingerprint_store_filename, FINGERPRINT_STORE_FILENAME);
	strcat(private_key_filename, PRIVATE_KEY_FILENAME);
}

int FoldersChanged(WPARAM wParam, LPARAM lParam) {
	char path[MAX_PATH];
	
	FOLDERSGETDATA fgd = {0};
	fgd.cbSize = sizeof(FOLDERSGETDATA);
	fgd.nMaxPathSize = sizeof(path);
	fgd.szPath = path;
	
	CallService(MS_FOLDERS_GET_PATH, (LPARAM)hFolderPath, (LPARAM)&fgd);

	SetFilenames(path);
	ReadPrivkeyFiles();
	return 0;
}

void LoadFilenames() {
	char path[MAX_PATH];
	if(ServiceExists(MS_FOLDERS_REGISTER_PATH)) {
		FOLDERSDATA fd = {0};
		fd.cbSize = sizeof(FOLDERSDATA);
		strncpy(fd.szSection, "OTR",FOLDERS_NAME_MAX_SIZE);
		strncpy(fd.szName, "Private Data", FOLDERS_NAME_MAX_SIZE);
		fd.szFormat = PROFILE_PATH "\\OTR_data";
		
		hFolderPath = (HANDLE)CallService(MS_FOLDERS_REGISTER_PATH, 0, (LPARAM)&fd);
		HookEvent(ME_FOLDERS_PATH_CHANGED, FoldersChanged);
		
		// get the path - above are only defaults - there may be a different value in the db
		FoldersChanged(0, 0);
	} else {
		//////////////
		/// create secure data filenames, prefixed with profile name (minus extention) and an underscore	
		if(CallService(MS_DB_GETPROFILEPATH, (WPARAM)MAX_PATH, (LPARAM)path)) {
			GetModuleFileName(0, path, MAX_PATH);
			char *p = strrchr(path, '\\');
			if(p) *p = 0;
		}
	
		strcat(path, "\\OTR_data");
		CreatePath(path);
		
		SetFilenames(path);
		ReadPrivkeyFiles();
	}
}

void LoadOptions() {
	options.default_policy = DBGetContactSettingWord(0, MODULE, "DefaultPolicy", OTRL_POLICY_OPPORTUNISTIC);
	// deal with changed flags in proto.h and new interpretation of 'manual' mode (see common.h)
	switch(options.default_policy) {
	case OTRL_POLICY_MANUAL:
		options.default_policy = OTRL_POLICY_MANUAL_MOD;
		break;
	case OTRL_POLICY_OPPORTUNISTIC:
	case OTRL_POLICY_MANUAL_MOD:
	case OTRL_POLICY_ALWAYS:
	case OTRL_POLICY_NEVER:
		break;
	default:
		options.default_policy = OTRL_POLICY_OPPORTUNISTIC;
		break;
	}
	if(options.default_policy == OTRL_POLICY_MANUAL)
		options.default_policy = OTRL_POLICY_MANUAL_MOD;
	options.err_method = (ErrorDisplay)DBGetContactSettingWord(0, MODULE, "ErrorDisplay", ED_POP);
	options.prefix_messages = (DBGetContactSettingByte(0, MODULE, "PrefixMessages", 0) == 1);
	options.msg_inline = (DBGetContactSettingByte(0, MODULE, "MsgInline", 0) == 1);
	
	DBVARIANT dbv;
	if(!DBGetContactSetting(0, MODULE, "Prefix", &dbv)) {
		strncpy(options.prefix, dbv.pszVal, 64);
		options.prefix[63] = 0;
		DBFreeVariant(&dbv);
	} else
		strcpy(options.prefix, "OTR: ");

	options.delete_history = (DBGetContactSettingByte(0, MODULE, "DeleteHistory", 1) == 1);
	options.timeout_finished = (DBGetContactSettingByte(0, MODULE, "TimeoutFinished", 0) == 1);
	
	options.end_offline = (DBGetContactSettingByte(0, MODULE, "EndOffline", 1) == 1);
	options.end_window_close = (DBGetContactSettingByte(0, MODULE, "EndWindowClose", 0) == 1);
	
	LoadFilenames();
}

void SaveOptions(ContactPolicyMap *contact_policies) {
	DBWriteContactSettingWord(0, MODULE, "DefaultPolicy", options.default_policy);
	DBWriteContactSettingWord(0, MODULE, "ErrorDisplay", (int)options.err_method);
	DBWriteContactSettingByte(0, MODULE, "PrefixMessages", options.prefix_messages ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "MsgInline", options.msg_inline ? 1 : 0);

	for(ContactPolicyMap::iterator i = contact_policies->begin(); i != contact_policies->end(); i++) {
		DBWriteContactSettingWord(i->first, MODULE, "Policy", i->second);
	}
	
	DBWriteContactSettingStringUtf(0, MODULE, "Prefix", options.prefix);	

	DBWriteContactSettingByte(0, MODULE, "DeleteHistory", options.delete_history ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "TimeoutFinished", options.timeout_finished ? 1 : 0);

	DBWriteContactSettingByte(0, MODULE, "EndOffline", options.end_offline ? 1 : 0);
	DBWriteContactSettingByte(0, MODULE, "EndWindowClose", options.end_window_close ? 1 : 0);
}

ContactFingerprintMap strlist; // temp storage for fingerprint string pointers
INT_PTR CALLBACK DlgProcOpts1(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );
		{
			// set contact to policy map
			ContactPolicyMap *contact_policies = new ContactPolicyMap();
			SetWindowLong(hwndDlg, GWL_USERDATA, (LONG)contact_policies);
			HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
			char *proto;
			while ( hContact != NULL ) {
				proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
				if(proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) == 0 && CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)MODULE)) {
					WORD pol = DBGetContactSettingWord(hContact, MODULE, "Policy", CONTACT_DEFAULT_POLICY);
					// deal with changed flags in proto.h and new interpretation of 'manual' mode (see common.h)
					switch(pol) {
					case OTRL_POLICY_MANUAL:
						pol = OTRL_POLICY_MANUAL_MOD;
						break;
					case OTRL_POLICY_OPPORTUNISTIC:
					case OTRL_POLICY_MANUAL_MOD:
					case OTRL_POLICY_ALWAYS:
					case OTRL_POLICY_NEVER:
						break;
					default:
						pol = CONTACT_DEFAULT_POLICY;
						break;
					}
						
					contact_policies->operator[](hContact) = pol;
				}
				hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
			}
		}
		switch(options.default_policy) {
			case OTRL_POLICY_OPPORTUNISTIC:
				CheckDlgButton(hwndDlg, IDC_RAD_OPP, TRUE);
				break;
			case OTRL_POLICY_MANUAL_MOD:
				CheckDlgButton(hwndDlg, IDC_RAD_MANUAL, TRUE);
				break;
			case OTRL_POLICY_ALWAYS:
				CheckDlgButton(hwndDlg, IDC_RAD_ALWAYS, TRUE);
				break;
			case OTRL_POLICY_NEVER:
				CheckDlgButton(hwndDlg, IDC_RAD_NEVER, TRUE);
				break;
		}

		SendMessage(GetDlgItem(hwndDlg, IDC_CLIST),LVM_SETEXTENDEDLISTVIEWSTYLE, 0,LVS_EX_FULLROWSELECT);// | LVS_EX_CHECKBOXES);

		{
			// add list columns
			LVCOLUMN lvc; 
			// Initialize the LVCOLUMN structure.
			// The mask specifies that the format, width, text, and
			// subitem members of the structure are valid. 
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			lvc.fmt = LVCFMT_LEFT;
	  
			lvc.iSubItem = 0;
			lvc.pszText = Translate("Contact");	
			lvc.cx = 150;     // width of column in pixels
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_CLIST), 0, &lvc);
			
			lvc.iSubItem = 1;
			lvc.pszText = Translate("Protocol");	
			lvc.cx = 100;     // width of column in pixels
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_CLIST), 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = Translate("Policy");	
			lvc.cx = 130;     // width of column in pixels
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_CLIST), 2, &lvc);

			lvc.iSubItem = 3;
			lvc.pszText = Translate("Fingerprint");	
			lvc.cx = 300;     // width of column in pixels
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_CLIST), 3, &lvc);
		}
		SendMessage(hwndDlg, WMU_REFRESHLIST, 0, 0);

		// fill proto list box
		{
			int num_protocols;
			PROTOCOLDESCRIPTOR **pppDesc;
	
			CallService(MS_PROTO_ENUMPROTOCOLS, (LPARAM)&num_protocols, (WPARAM)&pppDesc);
			HWND hw = GetDlgItem(hwndDlg, IDC_LIST_PROTOS);
			for(int i = 0; i < num_protocols; i++) {
				if(pppDesc[i]->type == PROTOTYPE_PROTOCOL && (metaproto == 0 || strcmp(pppDesc[i]->szName, metaproto) != 0)
					&& CallProtoService(pppDesc[i]->szName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM == PF1_IM)
				{
					SendMessage(hw, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)pppDesc[i]->szName);
				}
			}
		}
	
		CheckDlgButton(hwndDlg, IDC_CHK_PREFIX, options.prefix_messages ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_MSGINLINE, options.msg_inline ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_TIMEFINISH, options.timeout_finished ? TRUE : FALSE);

		CheckDlgButton(hwndDlg, IDC_CHK_ENDOFFLINE, options.end_offline ? TRUE : FALSE);
		CheckDlgButton(hwndDlg, IDC_CHK_ENDCLOSE, options.end_window_close ? TRUE : FALSE);
		
		SetDlgItemText(hwndDlg, IDC_ED_PREFIX, options.prefix);
		return TRUE;
		
	case WMU_REFRESHLIST:
		//enumerate contacts, fill in list
		{
			ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_CLIST));

			LVITEM lvI = {0};

			// Some code to create the list-view control.
			// Initialize LVITEM members that are common to all
			// items. 
			lvI.mask = LVIF_TEXT | LVIF_PARAM;// | LVIF_NORECOMPUTE;// | LVIF_IMAGE; 

			char *proto;
			HANDLE hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDFIRST, 0, 0 );
			while ( hContact != NULL )
			{
				proto = ( char* )CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM )hContact,0 );
				if(proto && DBGetContactSettingByte(hContact, proto, "ChatRoom", 0) == 0 && CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)MODULE) // ignore chatrooms
					&& (metaproto  == 0 || strcmp(proto, metaproto) != 0)) // and MetaContacts
				{
					lvI.iSubItem = 0;
					lvI.lParam = (LPARAM)hContact;
					lvI.pszText = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, 0);
					lvI.iItem = ListView_InsertItem(GetDlgItem(hwndDlg, IDC_CLIST), &lvI);

					lvI.iSubItem = 1;
					lvI.pszText = LPSTR_TEXTCALLBACK;
					ListView_SetItem(GetDlgItem(hwndDlg, IDC_CLIST), &lvI);

					lvI.iSubItem = 2;
					lvI.pszText = LPSTR_TEXTCALLBACK;
					ListView_SetItem(GetDlgItem(hwndDlg, IDC_CLIST), &lvI);

					lvI.iSubItem = 3;
					lvI.pszText = LPSTR_TEXTCALLBACK;
					ListView_SetItem(GetDlgItem(hwndDlg, IDC_CLIST), &lvI);

					// fill fingerprint map
					if(strlist.find(hContact) != strlist.end()) free(strlist[hContact]);
					char fp[45];
					if(GetFingerprint(hContact, fp))
							strlist[hContact] = strdup(fp);
				}

				
				hContact = ( HANDLE )CallService( MS_DB_CONTACT_FINDNEXT,( WPARAM )hContact, 0 );
			}
		}
		
		return TRUE;
	case WM_COMMAND:
		if ( HIWORD( wParam ) == EN_CHANGE && ( HWND )lParam == GetFocus()) {
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		} else
		if (HIWORD( wParam ) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LIST_PROTOS) {
			HWND hw = GetDlgItem(hwndDlg, IDC_LIST_PROTOS);
			int sel = -1;
			if((sel = SendMessage(hw, LB_GETCURSEL, 0, 0)) != -1) {
				char proto[512];
				SendMessage(hw, LB_GETTEXT, (WPARAM)sel, (LPARAM)proto);
				
				char fingerprint[45];
				if(otrl_privkey_fingerprint(otr_user_state, fingerprint, MODULE, proto))
					SetDlgItemText(hwndDlg, IDC_ED_FINGERPRINT, fingerprint);
				else
					SetDlgItemText(hwndDlg, IDC_ED_FINGERPRINT, "");

				hw = GetDlgItem(hwndDlg, IDC_BUT_NEWKEY);
				EnableWindow(hw, TRUE);
				
				return TRUE;
			} else {
				hw = GetDlgItem(hwndDlg, IDC_BUT_NEWKEY);
				EnableWindow(hw, FALSE);
			}
			return FALSE;
		} else
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
				case IDC_BUT_NEWKEY:
					{
						HWND hw = GetDlgItem(hwndDlg, IDC_LIST_PROTOS);
						int sel = -1;
						if((sel = SendMessage(hw, LB_GETCURSEL, 0, 0)) != -1) {
							char proto[512];
							SendMessage(hw, LB_GETTEXT, (WPARAM)sel, (LPARAM)proto);
							
							SetDlgItemText(hwndDlg, IDC_ED_FINGERPRINT, Translate("Generating new key...please wait"));
							otrl_privkey_generate(otr_user_state, private_key_filename, MODULE, proto);
	
							char fingerprint[45];
							if(otrl_privkey_fingerprint(otr_user_state, fingerprint, MODULE, proto))
								SetDlgItemText(hwndDlg, IDC_ED_FINGERPRINT, fingerprint);
							else
								SetDlgItemText(hwndDlg, IDC_ED_FINGERPRINT, "");
							
							return TRUE;
						}
					}
					
					break;
				
				case IDC_RAD_OPP:
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					return TRUE;
				case IDC_RAD_MANUAL:
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					return TRUE;
				case IDC_RAD_ALWAYS:
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					return TRUE;
				case IDC_RAD_NEVER:
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					return TRUE;
				case IDC_CHK_PREFIX:
				case IDC_CHK_TIMEFINISH:
				case IDC_CHK_MSGINLINE:
				case IDC_CHK_ENDOFFLINE:
				case IDC_CHK_ENDCLOSE:
					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
					return TRUE;
			}
		}		
		break;
	case WM_NOTIFY:
		if(((LPNMHDR) lParam)->hwndFrom == GetDlgItem(hwndDlg, IDC_CLIST)) {
			switch (((LPNMHDR) lParam)->code) {
			case NM_CLICK:
				if (((LPNMLISTVIEW)lParam)->iSubItem == 2) {
					LVITEM lvi;
					lvi.mask = LVIF_PARAM;
					lvi.iItem = ((LPNMLISTVIEW)lParam)->iItem;
					lvi.iSubItem = 0;
					SendMessage(GetDlgItem(hwndDlg, IDC_CLIST), LVM_GETITEM, 0, (LPARAM)&lvi);

					HANDLE hContact = (HANDLE)lvi.lParam;
					ContactPolicyMap *contact_policies = (ContactPolicyMap *)GetWindowLong(hwndDlg, GWL_USERDATA);
					ContactPolicyMap::iterator i = contact_policies->find(hContact);
					WORD pol = CONTACT_DEFAULT_POLICY;
					if(i != contact_policies->end())
						pol = i->second;
					
					switch(pol) {
						case CONTACT_DEFAULT_POLICY:
							pol = OTRL_POLICY_MANUAL_MOD;
							break;
						case OTRL_POLICY_OPPORTUNISTIC:
							pol = OTRL_POLICY_ALWAYS;
							break;
						case OTRL_POLICY_MANUAL_MOD:
							pol = OTRL_POLICY_OPPORTUNISTIC;
							break;
						case OTRL_POLICY_ALWAYS:
							pol = OTRL_POLICY_NEVER;
							break;
						case OTRL_POLICY_NEVER:
							pol = CONTACT_DEFAULT_POLICY;
							break;
					}			
					
					contact_policies->operator[](hContact) = pol;
					ListView_RedrawItems(GetDlgItem(hwndDlg, IDC_CLIST), lvi.iItem, lvi.iItem);

					SendMessage( GetParent( hwndDlg ), PSM_CHANGED, 0, 0 );
				} else if(((LPNMLISTVIEW)lParam)->iSubItem == 3) {
					// forget fingerprint?
				}
				break;
			case LVN_GETDISPINFO:
				{
					LVITEM lvi;
					lvi.mask = LVIF_PARAM;
					lvi.iItem = ((NMLVDISPINFO *)lParam)->item.iItem;
					lvi.iSubItem = 0;
					SendMessage(GetDlgItem(hwndDlg, IDC_CLIST), LVM_GETITEM, 0, (LPARAM)&lvi);
					switch (((NMLVDISPINFO *)lParam)->item.iSubItem) {
					case 0:
						//MessageBox(0, "LVN_GETDISPINFO (0)", "msg", MB_OK);
						break;
					case 1:
						{
							HANDLE hContact = (HANDLE)lvi.lParam;
							((NMLVDISPINFO *)lParam)->item.pszText = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
						}
						break;
					case 2:
						{
							
							char *text = 0;
							HANDLE hContact = (HANDLE)lvi.lParam;
							ContactPolicyMap *contact_policies = (ContactPolicyMap *)GetWindowLong(hwndDlg, GWL_USERDATA);
							ContactPolicyMap::iterator i = contact_policies->find(hContact);
							WORD pol = CONTACT_DEFAULT_POLICY;
							if(i != contact_policies->end())
								pol = i->second;
							switch(pol) {
								case CONTACT_DEFAULT_POLICY:
									text = Translate("Default");
									break;
								case OTRL_POLICY_OPPORTUNISTIC:
									text = Translate("Opportunistic");
									break;
								case OTRL_POLICY_MANUAL_MOD:
									text = Translate("Manual");
									break;
								case OTRL_POLICY_ALWAYS:
									text = Translate("Always");
									break;
								case OTRL_POLICY_NEVER:
									text = Translate("Never");
									break;
							}								
							((NMLVDISPINFO *)lParam)->item.pszText = text;
							
						}
						break;
					case 3:
						{
							HANDLE hContact = (HANDLE)lvi.lParam;
							if(strlist.find(hContact) != strlist.end())
								((NMLVDISPINFO *)lParam)->item.pszText = strlist[hContact];
							else 
								((NMLVDISPINFO *)lParam)->item.pszText = Translate("None");
						}
						break;
					}
					
				}
				break;
			}
			
			return FALSE;
		} else 
		if (((LPNMHDR)lParam)->code == (unsigned)PSN_APPLY ) {
			if(IsDlgButtonChecked(hwndDlg, IDC_RAD_OPP))
				options.default_policy = OTRL_POLICY_OPPORTUNISTIC;
			else if(IsDlgButtonChecked(hwndDlg, IDC_RAD_MANUAL))
				options.default_policy = OTRL_POLICY_MANUAL_MOD;
			else if(IsDlgButtonChecked(hwndDlg, IDC_RAD_ALWAYS))
				options.default_policy = OTRL_POLICY_ALWAYS;
			else if(IsDlgButtonChecked(hwndDlg, IDC_RAD_NEVER))
				options.default_policy = OTRL_POLICY_NEVER;
			
			options.prefix_messages = IsDlgButtonChecked(hwndDlg, IDC_CHK_PREFIX) ? true : false;
			options.msg_inline = IsDlgButtonChecked(hwndDlg, IDC_CHK_MSGINLINE) ? true : false;
			options.timeout_finished = IsDlgButtonChecked(hwndDlg, IDC_CHK_TIMEFINISH) ? true : false;

			options.end_offline = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENDOFFLINE) ? true : false;
			options.end_window_close = IsDlgButtonChecked(hwndDlg, IDC_CHK_ENDCLOSE) ? true : false;

			GetDlgItemText(hwndDlg, IDC_ED_PREFIX, options.prefix, 64);
			
			ContactPolicyMap *contact_policies = (ContactPolicyMap *)GetWindowLong(hwndDlg, GWL_USERDATA);
			
			SaveOptions(contact_policies);
			return TRUE;
		}
		break;
		
	case WM_DESTROY:
		{
			ContactPolicyMap *contact_policies = (ContactPolicyMap *)GetWindowLong(hwndDlg, GWL_USERDATA);
			SetWindowLong(hwndDlg, GWL_USERDATA, 0);
			delete contact_policies;
			
			for(std::map<HANDLE, char *>::iterator i = strlist.begin(); i != strlist.end(); i++) 
				free(i->second);
			strlist.clear();
		}
	}

	return FALSE;	
}

int OptInit(WPARAM wParam, LPARAM lParam) {	
	OPTIONSDIALOGPAGE odp = { 0 };
#define OPTIONPAGE_OLD_SIZE2 60
	//odp.cbSize						= sizeof(odp);
	odp.cbSize						= OPTIONPAGE_OLD_SIZE2;
	odp.position					= 100;
	odp.hInstance					= hInst;
	odp.pszTemplate					= MAKEINTRESOURCE(IDD_OPT1);
	odp.pszTitle					= Translate(MODULE);
	odp.pszGroup					= Translate("Plugins");
	odp.flags						= ODPF_BOLDGROUPS;
	odp.nIDBottomSimpleControl		= 0;
	odp.pfnDlgProc					= DlgProcOpts1;
	CallService( MS_OPT_ADDPAGE, wParam,( LPARAM )&odp );
	
	return 0;
}

void OptDeinit() {
}
