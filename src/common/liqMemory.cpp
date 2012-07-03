/*
**
** The contents of this file are subject to the Mozilla Public License Version 1.1 (the 
** "License"); you may not use this file except in compliance with the License. You may 
** obtain a copy of the License at http://www.mozilla.org/MPL/ 
** 
** Software distributed under the License is distributed on an "AS IS" basis, WITHOUT 
** WARRANTY OF ANY KIND, either express or implied. See the License for the specific 
** language governing rights and limitations under the License. 
**
** The Original Code is the Liquid Rendering Toolkit. 
** 
** The Initial Developer of the Original Code is Colin Doncaster. Portions created by 
** Colin Doncaster are Copyright (C) 2002. All Rights Reserved. 
** 
** Contributor(s): Berj Bannayan. 
**
** 
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
*/

/* ______________________________________________________________________
** 
** Liquid Memory Handling Source 
** ______________________________________________________________________
**
** A set of routines to handle error checking and keep track of memory
** usage through out the source.  
** Currently wraps malloc and free.
**
** Warning: This function set breaks away from the standard liquid naming
** convention.  Most other liquid functions start with liquid, this has been
** shortened to just an l, ie.  lmemusage etc.  
**/

// Standard Headers
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

// maya headers
#include <maya/MString.h>
#include <maya/MGlobal.h>

#include <list>

// Error Messages 
MString errorGettingMemoryMessage = "Liquid -> Error Allocating Memory!\n";

extern int debugMode;

struct ALLOC_INFO {
	long     address;
	long     size;
	char      file[64];
	long     line;
};

std::list<ALLOC_INFO> allocList;

void addTrack( long addr,  long asize,  const char *fname, long lnum)
{
	ALLOC_INFO info;
	
	info.address = addr;
	strncpy(info.file, fname, 63);
	info.line = lnum;
	info.size = asize;
	allocList.push_back( info );
};

void removeTrack( long addr )
{
	std::list<ALLOC_INFO>::iterator i = allocList.begin();
	while ( i != allocList.end() ) {
		if( i->address == addr )
		{
			printf( "-> liquidMemory FREE: %ld FILE: %s LINE: %ld\n", i->address, i->file, i->line );
			allocList.erase( i );
			break;
    }
		++i;
	}
}

void ldumpUnfreed()
{
    long totalSize = 0;

    printf("-------------- Liquid - Current Memory Usage --------------\n");

    std::list<ALLOC_INFO>::iterator i = allocList.begin();
    while ( i != allocList.end() ) {
		printf("%-50s:\t\tLINE %ld,\t\tADDRESS %ld\t%ld unfreed\n", i->file, i->line, i->address, i->size);
		totalSize += i->size;
		++i;
    }
    printf("-----------------------------------------------------------\n");
    printf("Total Unfreed: %ld bytes\n", totalSize);
}

int lmemUsage() 
{
    long totalSize = 0;
	
    std::list<ALLOC_INFO>::iterator i = allocList.begin();
    while ( i != allocList.end() ) {
    	totalSize += i->size;
    	++i;
    }

    return totalSize;
}

// wrapper for malloc that keeps track of allocated memory as well as
// checking for enough memory
void *ldmalloc( size_t size, const char *fileName, const long line )
{
	void *ptr = NULL;
	if ( size > 0 ) {
		ptr = malloc( size );
		if ( ptr == NULL ) {
			throw( errorGettingMemoryMessage );
		}
		if ( debugMode ) addTrack( (long)ptr, size, fileName, line );
	}
	return ptr;
}

void *ldcalloc( size_t nelem, size_t elsize, const char *fileName, const long line )
{
	void *ptr = NULL;
	if ( nelem > 0 && elsize > 0 ) {
		ptr = calloc( nelem, elsize );
		if ( ptr == NULL ) {
			throw( errorGettingMemoryMessage );
		}
		if ( debugMode ) addTrack( (long)ptr, ( nelem * elsize ), fileName, line );
	}
	return ptr;
}

// wrapper for free that keeps track of freed memory
void ldfree( void *ptr )
{
	if ( debugMode ) removeTrack( (long)ptr );
	free( ptr );
	ptr = NULL;
}
