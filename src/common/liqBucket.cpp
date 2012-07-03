/*
**
** The contents of this file are subject to the Mozilla Public License Version
** 1.1 (the "License"); you may not use this file except in compliance with
** the License. You may obtain a copy of the License at
** http://www.mozilla.org/MPL/
**
** Software distributed under the License is distributed on an "AS IS" basis,
** WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
** for the specific language governing rights and limitations under the
** License.
**
** The Original Code is the Liquid Rendering Toolkit.
**
** The Initial Developer of the Original Code is Colin Doncaster. Portions
** created by Colin Doncaster are Copyright (C) 2002. All Rights Reserved.
**
** Contributor(s): Samy Ben Rabah.
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
** Liquid bucket handling Source
** ______________________________________________________________________
*/
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "liqBucket.h"
#include <cstring>

bucket::bucket() 
{
	m_pixels = NULL;
}
bucket::~bucket() 
{
	free();
}
void bucket::free() 
{
	if ( m_pixels ) delete[] m_pixels;
	m_pixels = NULL;
}
int bucket::set(const bucketInfo &info, const BUCKETDATATYPE *pixels ) 
{
	m_info.left			= info.left;
	m_info.right		= info.right;
	m_info.bottom		= info.bottom;
	m_info.top			= info.top;
	m_info.channels	= info.channels;
	m_info.channels	= info.channels;
	unsigned size = ( info.right - info.left ) * abs( (long double)(info.top - info.bottom) ) * info.channels * sizeof( BUCKETDATATYPE );
	m_pixels		= new BUCKETDATATYPE[size];
  if ( !m_pixels ) return 1;
	memcpy( (void *)m_pixels, (const void *)pixels, (size_t)size );
  return 0;
}
