/*
**
** The contents of this file are subject to the Mozilla Public License Version
** 1.1 (the "License"); you may not use this file except in compliance with
** the License.  You may obtain a copy of the License at
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

#ifndef liqPixieRenderer_H
#define liqPixieRenderer_H

#include <liqRenderer.h>

class liqPixieRenderer : public liqRenderer {
public:
    liqPixieRenderer(MString version)
	: liqRenderer(REN_PIXIE, version)
    {
	// nothing else needed
    }

    virtual ~liqPixieRenderer()
    {
	// nothing else needed
    }

    virtual bool	supports(e_capability capability) const;
    virtual bool	requires(e_requirement requirement) const;
};

#endif
