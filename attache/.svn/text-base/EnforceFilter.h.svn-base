#ifndef _ENFORCE_FILTER_INC
#define _ENFORCE_FILTER_INC

////////////////////////////////////////////////////////////////////////////////
// Function declarations
//

	// EnforceFilter function 
	// 
	// If bEnforce is "true", the function overwrites the beginning of 
	// SetUnhandledExceptionFilter function with a patch which rejects  
	// all subsequent attempts to register a filter.
	// If bEnforce is "false", the original functionality of 
	// SetUnhandledExceptionFilter is restored. 
	// 
bool EnforceFilter( bool bEnforce );

	// WriteMemory function 
	// 
	// This function writes the specified sequence of bytes from 
	// the source memory into the target memory. In addition, the function 
	// modifies virtual memory protection attributes of the target memory page 
	// to make sure that it is writeable.
	// 
bool WriteMemory( BYTE* pTarget, const BYTE* pSource, DWORD Size );

#endif
