#include "common.h"
#include "EnforceFilter.h"

////////////////////////////////////////////////////////////////////////////////
//
// EnforceFilter.cpp 
//
// Author: Oleg Starodumov (www.debuginfo.com)
//


////////////////////////////////////////////////////////////////////////////////
//
// This example demonstrates how to ensure that nobody else can overwrite 
// our custom filter for unhandler exceptions. This is achieved by patching 
// SetUnhandledExceptionFilter function. 
// 
//


////////////////////////////////////////////////////////////////////////////////
// Include files 
//

#include <windows.h>
#include <tchar.h>
#include <crtdbg.h>
#include <stdio.h>
#include "EnforceFilter.h"

////////////////////////////////////////////////////////////////////////////////
// Global variables 
//

	// Patch for SetUnhandledExceptionFilter 
const BYTE PatchBytes[5] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };

	// Original bytes at the beginning of SetUnhandledExceptionFilter 
BYTE OriginalBytes[5] = {0};


////////////////////////////////////////////////////////////////////////////////
// EnforceFilter function 
// 

bool EnforceFilter( bool bEnforce )
{
	DWORD ErrCode = 0;

	
	// Obtain the address of SetUnhandledExceptionFilter 

	HMODULE hLib = GetModuleHandle( _T("kernel32.dll") );

	if( hLib == NULL )
	{
		ErrCode = GetLastError();
		_ASSERTE( !_T("GetModuleHandle(kernel32.dll) failed.") );
		return false;
	}

	BYTE* pTarget = (BYTE*)GetProcAddress( hLib, "SetUnhandledExceptionFilter" );

	if( pTarget == 0 )
	{
		ErrCode = GetLastError();
		_ASSERTE( !_T("GetProcAddress(SetUnhandledExceptionFilter) failed.") );
		return false;
	}

	if( IsBadReadPtr( pTarget, sizeof(OriginalBytes) ) )
	{
		_ASSERTE( !_T("Target is unreadable.") );
		return false;
	}


	if( bEnforce )
	{
		// Save the original contents of SetUnhandledExceptionFilter 

		memcpy( OriginalBytes, pTarget, sizeof(OriginalBytes) );


		// Patch SetUnhandledExceptionFilter 

		if( !WriteMemory( pTarget, PatchBytes, sizeof(PatchBytes) ) )
			return false;

	}
	else
	{
		// Restore the original behavior of SetUnhandledExceptionFilter 

		if( !WriteMemory( pTarget, OriginalBytes, sizeof(OriginalBytes) ) )
			return false;

	}


	// Success 

	return true;

}


////////////////////////////////////////////////////////////////////////////////
// WriteMemory function 
// 

bool WriteMemory( BYTE* pTarget, const BYTE* pSource, DWORD Size )
{
	DWORD ErrCode = 0;


	// Check parameters 

	if( pTarget == 0 )
	{
		_ASSERTE( !_T("Target address is null.") );
		return false;
	}

	if( pSource == 0 )
	{
		_ASSERTE( !_T("Source address is null.") );
		return false;
	}

	if( Size == 0 )
	{
		_ASSERTE( !_T("Source size is null.") );
		return false;
	}

	if( IsBadReadPtr( pSource, Size ) )
	{
		_ASSERTE( !_T("Source is unreadable.") );
		return false;
	}


	// Modify protection attributes of the target memory page 

	DWORD OldProtect = 0;

	if( !VirtualProtect( pTarget, Size, PAGE_EXECUTE_READWRITE, &OldProtect ) )
	{
		ErrCode = GetLastError();
		_ASSERTE( !_T("VirtualProtect() failed.") );
		return false;
	}


	// Write memory 

	memcpy( pTarget, pSource, Size );


	// Restore memory protection attributes of the target memory page 

	DWORD Temp = 0;

	if( !VirtualProtect( pTarget, Size, OldProtect, &Temp ) )
	{
		ErrCode = GetLastError();
		_ASSERTE( !_T("VirtualProtect() failed.") );
		return false;
	}


	// Success 

	return true;

}
