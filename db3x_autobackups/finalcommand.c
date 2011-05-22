/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2003 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "commonheaders.h"

char exit_command[MAX_PATH],
	exit_command_dir[MAX_PATH],
	exit_command_params[MAX_PATH];

BOOL command_valid = FALSE,
	command_dir_valid = FALSE,
	command_params_valid = FALSE;

void FinalCommandSetup(void) {
	DBVARIANT dbv;

	if(!DBGetContactSetting(0, "db3x", "ExitCommand", &dbv) && dbv.type == DBVT_ASCIIZ) {
		strncpy(exit_command, dbv.pszVal, MAX_PATH);
		command_valid = TRUE;
		DBFreeVariant(&dbv);

		if(!DBGetContactSetting(0, "db3x", "ExitCommandParams", &dbv) && dbv.type == DBVT_ASCIIZ) {
			strncpy(exit_command_params, dbv.pszVal, MAX_PATH);
			command_params_valid = TRUE;
			DBFreeVariant(&dbv);
		}
		if(!DBGetContactSetting(0, "db3x", "ExitCommandDir", &dbv) && dbv.type == DBVT_ASCIIZ) {
			strncpy(exit_command_dir, dbv.pszVal, MAX_PATH);
			command_dir_valid = TRUE;
			DBFreeVariant(&dbv);
		}
	}
}

void FinalCommandExecute(void) {
	if(command_valid)
		ShellExecuteA(0, 0, exit_command, command_params_valid ? exit_command_params : 0, command_dir_valid ? exit_command_dir : 0, SW_SHOWNORMAL);
}
