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
#ifndef __LIQ_MAYA_NODE_IDS_H__
#define __LIQ_MAYA_NODE_IDS_H__
#if LIQ_OLD_MAYA_IDS == 1
// THESE ARE THE OLD IDs
#define liqSurfaceNodeId		0x00103511
#define liqDisplacementNodeId	0x00103512
#define liqVolumeNodeId			0x00103513
#define liqLightNodeId			0x00103514
#define liqRibboxNodeId			0x00103515
#define liqGlobalsNodeId		0x00103516
#define liqCoordSysNodeId		0x00103517
#else
// THESE ARE THE NEW IDs
#define liqSurfaceNodeId        0x0010351f
#define liqDisplacementNodeId   0x00103520
#define liqVolumeNodeId         0x00103521
#define liqLightNodeId          0x00103522
#define liqRibboxNodeId         0x00103523
#define liqGlobalsNodeId        0x00103524
#define liqCoordSysNodeId       0x00103525
#define liqRibRequestId         0x0010F6D1
#define liqBoundingBoxLocatorId 0x0010F6D8
#define liqCoShaderNodeId       0x0010F6D9
#define liqSwitcherNodeId       0x0010F6DA
#define liqSurfaceSwitcherNodeId 0x0010F6DB
#define liqDisplacementSwitcherNodeId 0x0010F6DC
#endif

#endif // __LIQ_MAYA_NODE_IDS_H__
