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
** Contributor(s): Philippe Leprince.
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
** Liquid Light Drag&Drop Header
** ______________________________________________________________________
*/

#ifndef _liqLightNodeBehavior_H_
#define _liqLightNodeBehavior_H_

#include <maya/MPxDragAndDropBehavior.h>

class liqLightNodeBehavior : public MPxDragAndDropBehavior
{

public:

	liqLightNodeBehavior();
	virtual ~liqLightNodeBehavior();

	virtual bool shouldBeUsedFor( MObject &sourceNode, MObject &destinationNode,
								  MPlug &sourcePlug, MPlug &destinationPlug);

	virtual MStatus connectNodeToNode( MObject &sourceNode, MObject &destinationNode, bool force );

	virtual MStatus connectNodeToAttr( MObject &sourceNode, MPlug &destinationPlug, bool force );

	static void *creator();

protected:

private:

};

#endif

