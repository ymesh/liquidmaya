//////////////////////////////////////////////////////////////////////
//
//                             Pixie
//
// Copyright © 1999 - 2003, Okan Arikan
//
// Contact: okan@cs.berkeley.edu
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//
//  File				:	variable.h
//  Classes				:	CVariable
//  Description			:	This class holds variable information
//
////////////////////////////////////////////////////////////////////////
#ifndef VARIABLE_H
#define VARIABLE_H

#include "common.h"		// The global header file

// Possible renderer blocks (Used by sfBegin - sfEnd)
typedef enum {
    BLOCK_OUTSIDE   =   0,          // The initial block before the renderer is
    BLOCK_RENDERER,                 // The renderer has been initialized
    BLOCK_FRAME,                    // Inside a frame block (equals to a renderm    BLOCK_XFORM,                    // Inside an xform block
    BLOCK_ATTRIBUTES,               // Inside an attributes block
    BLOCK_OPTIONS                   // inside an options block
} ERendererBlock;


// Possible types for a variable (Used by CVariable class)
typedef enum {
    TYPE_FLOAT,                         // "u","v","s","t","Pz" ...
    TYPE_COLOR,                         // "Cs"
    TYPE_VECTOR,
    TYPE_NORMAL,
    TYPE_POINT,
    TYPE_MATRIX,
    TYPE_QUAD,                          // For "Pw"
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_INTEGER,
    TYPE_BOOLEAN
} EVariableType;

// Possible container classes for a variable (Used by CVariable class)
typedef enum {
    CONTAINER_UNIFORM,
    CONTAINER_VERTEX,
    CONTAINER_VARYING,
    CONTAINER_FACEVARYING,
    CONTAINER_CONSTANT
} EVariableClass;


class CVariable {
public:
    string              name;           // Name as it is referenced
    EVariableType       type;           // Type
    EVariableClass      container;      // Container type
    int                 numItems;       // Number of items if this is an array
    int                 numFloats;      // Number of floats per variable (1 for
};
 


int	parseVariable(CVariable *,const char *,const char *);

#endif
