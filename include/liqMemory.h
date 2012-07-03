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

#ifdef DEBUG
//#define _DEBUGMEMORYSYSTEM
#endif

#ifndef liquidMemory_H
#define liquidMemory_H

/* ______________________________________________________________________
** 
** Liquid Memory Handling Header File
** ______________________________________________________________________
*/
#include <sys/types.h>
#ifdef _DEBUGMEMORYSYSTEM
#define lmalloc( size ) ldmalloc( size, __FILE__, __LINE__ )
#define lfree( block )	ldfree( block, __FILE__, __LINE__)
#define lcalloc( nelem, elsize ) ldcalloc( nelem, elsize, __FILE__, __LINE__)
#else
#define lmalloc( size ) malloc( size )
#define lfree( block ) free( block )
#define lcalloc( nelem, elsize ) calloc( nelem, elsize ) 
#endif

int lmemUsage();
void ldumpUnfreed();

void *ldmalloc( size_t size, const char *fileName, const long line );
void *ldcalloc( size_t nelem, size_t elsize, const char *fileName, const long line );
void  ldfree( void *ptr );

#endif
