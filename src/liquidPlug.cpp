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
** Liquid Plug-In
** ______________________________________________________________________
*/

#ifdef _WIN32
#pragma warning(disable:4786)
#endif

// DLL export symbols must be specified under Win32
#ifdef _WIN32
#define LIQUID_EXPORT _declspec(dllexport)
#else
#define LIQUID_EXPORT
#endif

// Renderman Headers
extern "C" {
#include <ri.h>
}

// Maya's Headers
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MSwatchRenderRegister.h>

#include <liquid.h>
#include <liqRibTranslator.h>
#include <liqGetAttr.h>
#include <liqAttachPrefAttribute.h>
#include <liqPreviewShader.h>
#include <liqWriteArchive.h>
#include <liqNodeSwatch.h>
#include <liqSurfaceNode.h>
#include <liqDisplacementNode.h>
#include <liqVolumeNode.h>
#include <liqLightNode.h>
#include <liqLightNodeBehavior.h>
#include <liqRibboxNode.h>
#include <liqRibRequestNode.h>
#include <liqCoordSysNode.h>
#include <liqGlobalHelpers.h>
#include <liqMayaRenderView.h>
#include <liqGlobalsNode.h>
#include <liqJobList.h>
#include <liqRiCommands.h>
#include <liqBoundingBoxLocator.h>
#include <liqCoShaderNode.h>
#include <liqShaderFactory.h>
#include <liqRibShaderTranslator.h>
#include <liqSurfaceSwitcherNode.h>
#include <liqDisplacementSwitcherNode.h>
#include <liqParseString.h>

#define LIQVENDOR "http://liquidmaya.sourceforge.net/"

#if defined(_WIN32) /*&& !defined(DEFINED_LIQUIDVERSION)*/
// unix build gets this from the Makefile
static const char * LIQUIDVERSION =
#include "liquid.version"
;
#define DEFINED_LIQUIDVERSION
#endif

bool liquidBin = false;

////////////////////// EXPORTS /////////////////////////////////////////////////////////
LIQUID_EXPORT MStatus initializePlugin(MObject obj)
//  Description:
//      Register the command when the plug-in is loaded
{
  // liquidBin = false;

  MStatus status;

  MFnPlugin plugin( obj, LIQVENDOR, LIQUIDVERSION, "Any");

  MGlobal::displayInfo(MString("Initializing Liquid v") + LIQUIDVERSION);
  MGlobal::displayInfo("Initial Liquid code by Colin Doncaster");

  status = plugin.registerCommand("liquid", liqRibTranslator::creator, liqRibTranslator::syntax );
  LIQCHECKSTATUS( status, "Can't register liquid translator command" );

  // register the liquidAttachPrefAttribute command
  status = plugin.registerCommand( "liquidAttachPrefAttribute", liqAttachPrefAttribute::creator, liqAttachPrefAttribute::syntax );
  LIQCHECKSTATUS( status, "Can't register liquidAttachPrefAttribute command" );

  // register the liquidPreviewShader command
  status = plugin.registerCommand( "liquidPreviewShader", liqPreviewShader::creator, liqPreviewShader::syntax );
  LIQCHECKSTATUS( status, "Can't register liqPreviewShader command" );

  // register the liqGetSloInfo command
  status = plugin.registerCommand( "liquidGetSloInfo", liqGetSloInfo::creator );
  LIQCHECKSTATUS( status, "Can't register liquidGetSloInfo command" );

  // register the liquidGetAttr command
  status = plugin.registerCommand( "liquidGetAttr", liqGetAttr::creator );
  LIQCHECKSTATUS( status, "Can't register liquidGetAttr command" );

  // register the liquidWriteArchive command
  status = plugin.registerCommand( "liquidWriteArchive", liqWriteArchive::creator, liqWriteArchive::syntax );
  LIQCHECKSTATUS( status, "Can't register liquidWriteArchive command" );

  // register the liquidRenderView command
  status = plugin.registerCommand( "liquidRenderView", liqMayaRenderCmd::creator ,liqMayaRenderCmd::newSyntax);
  LIQCHECKSTATUS( status, "Can't register liquidRenderView command" );

  // register the liquidJobList command
  status = plugin.registerCommand( "liquidJobList", liqJobList::creator ,liqJobList::syntax);
  LIQCHECKSTATUS( status, "Can't register liquidJobList command" );

#ifndef NO_RICMD
  // register the RIArchiveBegin command
  status = plugin.registerCommand( "RIArchiveBegin", RIArchiveBegin::creator, RIArchiveBegin::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIArchiveBegin command" );

  // register the RIArchiveEnd command
  status = plugin.registerCommand( "RIArchiveEnd", RIArchiveEnd::creator, RIArchiveEnd::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIArchiveEnd command" );

  // register the RIArchiveRecord command
  status = plugin.registerCommand( "RIArchiveRecord", RIArchiveRecord::creator, RIArchiveRecord::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIArchiveRecord command" );

  // register the RIAtmosphere command
  status = plugin.registerCommand( "RIAtmosphere", RIAtmosphere::creator, RIAtmosphere::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIAtmosphere command" );

  // register the RIAttribute command
  status = plugin.registerCommand( "RIAttribute", RIAttribute::creator, RIAttribute::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIAttribute command" );

  // register the RIAttributeBegin command
  status = plugin.registerCommand( "RIAttributeBegin", RIAttributeBegin::creator, RIAttributeBegin::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIAttributeBegin command" );

  // register the RIAttributeEnd command
  status = plugin.registerCommand( "RIAttributeEnd", RIAttributeEnd::creator, RIAttributeEnd::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIAttributeEnd command" );

  // register the RIClipping command
  status = plugin.registerCommand( "RIClipping", RIClipping::creator, RIClipping::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIClipping command" );

  // register the RIColor command
  status = plugin.registerCommand( "RIColor", RIColor::creator, RIColor::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIColor command" );

  // register the RIConcatTransform command
  status = plugin.registerCommand( "RIConcatTransform", RIConcatTransform::creator, RIConcatTransform::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIConcatTransform command" );

  // register the RICropWindow command
  status = plugin.registerCommand( "RICropWindow", RICropWindow::creator, RICropWindow::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RICropWindow command" );

  // register the RIDepthOfField command
  status = plugin.registerCommand( "RIDepthOfField", RIDepthOfField::creator, RIDepthOfField::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIDepthOfField command" );

  // register the RIDetail command
  status = plugin.registerCommand( "RIDetail", RIDetail::creator, RIDetail::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIDetail command" );

  // register the RIDetailRange command
  status = plugin.registerCommand( "RIDetailRange", RIDetailRange::creator, RIDetailRange::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIDetailRange command" );

  // register the RIDisplacement command
  status = plugin.registerCommand( "RIDisplacement", RIDisplacement::creator, RIDisplacement::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIDisplacement command" );

  // register the RIDisplay command
  status = plugin.registerCommand( "RIDisplay", RIDisplay::creator, RIDisplay::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIDisplay command" );

  // register the RIDisplayChannel command
  status = plugin.registerCommand( "RIDisplayChannel", RIDisplayChannel::creator, RIDisplayChannel::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIDisplayChannel command" );

  // register the RIExterior command
  status = plugin.registerCommand( "RIExterior", RIExterior::creator, RIExterior::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIExterior command" );

  // register the RIFormat command
  status = plugin.registerCommand( "RIFormat", RIFormat::creator, RIFormat::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIFormat command" );

  // register the RIFrameBegin command
  status = plugin.registerCommand( "RIFrameBegin", RIFrameBegin::creator, RIFrameBegin::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIFrameBegin command" );

  // register the RIFrameEnd command
  status = plugin.registerCommand( "RIFrameEnd", RIFrameEnd::creator, RIFrameEnd::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIFrameEnd command" );

  // register the RIGeometricApproximation command
  status = plugin.registerCommand( "RIGeometricApproximation", RIGeometricApproximation::creator, RIGeometricApproximation::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIGeometricApproximation command" );

  // register the RIGeometry command
  status = plugin.registerCommand( "RIGeometry", RIGeometry::creator, RIGeometry::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIGeometry command" );

  // register the RIHider command
  status = plugin.registerCommand( "RIHider", RIHider::creator, RIHider::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIHider command" );

  // register the RIIdentity command
  status = plugin.registerCommand( "RIIdentity", RIIdentity::creator, RIIdentity::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIIdentity command" );

  // register the RIIlluminate command
  status = plugin.registerCommand( "RIIlluminate", RIIlluminate::creator, RIIlluminate::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIIlluminate command" );

  // register the RIInterior command
  status = plugin.registerCommand( "RIInterior", RIInterior::creator, RIInterior::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIInterior command" );

  // register the RILightSource command
  status = plugin.registerCommand( "RILightSource", RILightSource::creator, RILightSource::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RILightSource command" );

  // register the RIMatte command
  status = plugin.registerCommand( "RIMatte", RIMatte::creator, RIMatte::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIMatte command" );

  // register the RIMotionBegin command
  status = plugin.registerCommand( "RIMotionBegin", RIMotionBegin::creator, RIMotionBegin::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIMotionBegin command" );

  // register the RIMotionEnd command
  status = plugin.registerCommand( "RIMotionEnd", RIMotionEnd::creator, RIMotionEnd::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIMotionEnd command" );

  // register the RIObjectBegin command
  status = plugin.registerCommand( "RIObjectBegin", RIObjectBegin::creator, RIObjectBegin::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIObjectBegin command" );

  // register the RIObjectEnd command
  status = plugin.registerCommand( "RIObjectEnd", RIObjectEnd::creator, RIObjectEnd::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIObjectEnd command" );

  // register the RIOpacity command
  status = plugin.registerCommand( "RIOpacity", RIOpacity::creator, RIOpacity::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIOpacity command" );

  // register the RIOption command
  status = plugin.registerCommand( "RIOption", RIOption::creator, RIOption::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIOption command" );

  // register the RIOrientation command
  status = plugin.registerCommand( "RIOrientation", RIOrientation::creator, RIOrientation::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIOrientation command" );

  // register the RIPixelSamples command
  status = plugin.registerCommand( "RIPixelSamples", RIPixelSamples::creator, RIPixelSamples::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIPixelSamples command" );

  // register the RIProcedural command
  status = plugin.registerCommand( "RIProcedural", RIProcedural::creator, RIProcedural::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIProcedural command" );

  // register the RIProjection command
  status = plugin.registerCommand( "RIProjection", RIProjection::creator, RIProjection::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIProjection command" );

  // register the RIReadArchive command
  status = plugin.registerCommand( "RIReadArchive", RIReadArchive::creator, RIReadArchive::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIReadArchive command" );

  // register the RIRelativeDetail command
  status = plugin.registerCommand( "RIRelativeDetail", RIRelativeDetail::creator, RIRelativeDetail::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIRelativeDetail command" );

  // register the RIResource command
  status = plugin.registerCommand( "RIResource", RIResource::creator, RIResource::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIResource command" );

  // register the RIResourceBegin command
  status = plugin.registerCommand( "RIResourceBegin", RIResourceBegin::creator, RIResourceBegin::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIResourceBegin command" );

  // register the RIResourceEnd command
  status = plugin.registerCommand( "RIResourceEnd", RIResourceEnd::creator, RIResourceEnd::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIResourceEnd command" );

  // register the RIReverseOrientation command
  status = plugin.registerCommand( "RIReverseOrientation", RIReverseOrientation::creator, RIReverseOrientation::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIReverseOrientation command" );

  // register the RIRotate command
  status = plugin.registerCommand( "RIRotate", RIRotate::creator, RIRotate::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIRotate command" );

  // register the RIScale command
  status = plugin.registerCommand( "RIScale", RIScale::creator, RIScale::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIScale command" );

  // register the RIScreenWindow command
  status = plugin.registerCommand( "RIScreenWindow", RIScreenWindow::creator, RIScreenWindow::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIScreenWindow command" );

  // register the RIShadingInterpolation command
  status = plugin.registerCommand( "RIShadingInterpolation", RIShadingInterpolation::creator, RIShadingInterpolation::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIShadingInterpolation command" );

  // register the RIShadingRate command
  status = plugin.registerCommand( "RIShadingRate", RIShadingRate::creator, RIShadingRate::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIShadingRate command" );

  // register the RIShutter command
  status = plugin.registerCommand( "RIShutter", RIShutter::creator, RIShutter::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIShutter command" );

  // register the RISides command
  status = plugin.registerCommand( "RISides", RISides::creator, RISides::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RISides command" );

  // register the RISkew command
  status = plugin.registerCommand( "RISkew", RISkew::creator, RISkew::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RISkew command" );

  // register the RISolidBegin command
  status = plugin.registerCommand( "RISolidBegin", RISolidBegin::creator, RISolidBegin::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RISolidBegin command" );

  // register the RISolidEnd command
  status = plugin.registerCommand( "RISolidEnd", RISolidEnd::creator, RISolidEnd::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RISolidEnd command" );

  // register the RISphere command
  status = plugin.registerCommand( "RISphere", RISphere::creator, RISphere::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RISphere command" );

  // register the RISurface command
  status = plugin.registerCommand( "RISurface", RISurface::creator, RISurface::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RISurface command" );

  // register the RITransformBegin command
  status = plugin.registerCommand( "RITransformBegin", RITransformBegin::creator, RITransformBegin::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RITransformBegin command" );

  // register the RITransformEnd command
  status = plugin.registerCommand( "RITransformEnd", RITransformEnd::creator, RITransformEnd::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RITransformEnd command" );

  // register the RITranslate command
  status = plugin.registerCommand( "RITranslate", RITranslate::creator, RITranslate::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RITranslate command" );

  // register the RIWorldBegin command
  status = plugin.registerCommand( "RIWorldBegin", RIWorldBegin::creator, RIWorldBegin::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIWorldBegin command" );

  // register the RIWorldEnd command
  status = plugin.registerCommand( "RIWorldEnd", RIWorldEnd::creator, RIWorldEnd::newSyntax );
  LIQCHECKSTATUS( status, "Can't register RIWorldEnd command" );

#endif

  // register the liquidShader node
  const MString UserClassify( "shader/surface:swatch/liqSurfSwatch" );
  status = plugin.registerNode( "liquidSurface", liqSurfaceNode::id, liqSurfaceNode::creator, liqSurfaceNode::initialize, MPxNode::kDependNode, &UserClassify );
  LIQCHECKSTATUS( status, "Can't register liquidSurface node" );
  status.clear();
  status = MSwatchRenderRegister::registerSwatchRender( "liqSurfSwatch", liqNodeSwatch::creator );
  LIQCHECKSTATUS( status, "Can't register liquidSurface swatch" );

  // register the liquidDisplacement node
  const MString UserClassify1( "shader/displacement:swatch/liqDispSwatch" );
  status = plugin.registerNode( "liquidDisplacement", liqDisplacementNode::id, liqDisplacementNode::creator, liqDisplacementNode::initialize, MPxNode::kDependNode, &UserClassify1 );
  LIQCHECKSTATUS( status, "Can't register liquidDisp node" );
  status.clear();
  status = MSwatchRenderRegister::registerSwatchRender( "liqDispSwatch", liqNodeSwatch::creator );
  LIQCHECKSTATUS( status, "Can't register liquidDisplacement swatch" );

  // register the liquidVolume node
  const MString UserClassify2( "shader/volume:swatch/liqVolSwatch" );
  status = plugin.registerNode( "liquidVolume", liqVolumeNode::id, liqVolumeNode::creator, liqVolumeNode::initialize, MPxNode::kDependNode, &UserClassify2 );
  LIQCHECKSTATUS( status, "Can't register liquidVolume node" );
  status.clear();
  status = MSwatchRenderRegister::registerSwatchRender( "liqVolSwatch", liqNodeSwatch::creator );
  LIQCHECKSTATUS( status, "Can't register liquidVolume swatch" );

  // register the liquidLight node
  const MString UserClassify3( "shader/surface:swatch/liqLightSwatch" );
  status = plugin.registerNode( "liquidLight", liqLightNode::id, liqLightNode::creator, liqLightNode::initialize, MPxNode::kDependNode, &UserClassify3 );
  LIQCHECKSTATUS( status, "Can't register liquidLight node" );
  status.clear();
  status = MSwatchRenderRegister::registerSwatchRender( "liqLightSwatch", liqNodeSwatch::creator );
  LIQCHECKSTATUS( status, "Can't register liqLightSwatch swatch" );
  status = plugin.registerDragAndDropBehavior( "liquidLightBehavior", liqLightNodeBehavior::creator);
  LIQCHECKSTATUS( status, "Can't register liquidLight behavior" );

  // register the liquidRibbox node
  const MString UserClassify4( "utility/general:swatch/liqRibSwatch" );
  status = plugin.registerNode( "liquidRibBox", liqRibboxNode::id, liqRibboxNode::creator, liqRibboxNode::initialize, MPxNode::kDependNode, &UserClassify4 );
  LIQCHECKSTATUS( status, "Can't register liquidRibbox node" );
  status.clear();
  status = MSwatchRenderRegister::registerSwatchRender( "liqRibSwatch", liqNodeSwatch::creator );
  LIQCHECKSTATUS( status, "Can't register liquidRibbox swatch" );

  // register the liquidRequest node
  status = plugin.registerNode( "liquidRibRequest", liqRibRequestNode::id, liqRibRequestNode::creator, liqRibRequestNode::initialize, MPxNode::kDependNode );
  LIQCHECKSTATUS( status, "Can't register liquidRibRequest node" );
  status.clear();

  // register the liquidCoordSys node
  status = plugin.registerNode( "liquidCoordSys", liqCoordSysNode::id, liqCoordSysNode::creator, liqCoordSysNode::initialize, MPxNode::kLocatorNode );
  LIQCHECKSTATUS( status, "Can't register liquidCoordSys node" );
  status.clear();

  // register the liqBoundingBoxLocator node
  status = plugin.registerNode( "liqBoundingBoxLocator", liqBoundingBoxLocator::id, liqBoundingBoxLocator::creator, liqBoundingBoxLocator::initialize, MPxNode::kLocatorNode );
  LIQCHECKSTATUS( status, "Can't register liqBoundingBoxLocator node" );
  status.clear();

  // register the liquidGlobals node
  status = plugin.registerNode( "liquidGlobals", liqGlobalsNode::id, liqGlobalsNode::creator, liqGlobalsNode::initialize, MPxNode::kDependNode );
  LIQCHECKSTATUS( status, "Can't register liquidGlobals node" );
  status.clear();

  // register the liquidCoShader node
  const MString UserClassifyCoShader( "utility/general:swatch/liqCoShaderSwatch" );
  status = plugin.registerNode( "liquidCoShader", liqCoShaderNode::id, liqCoShaderNode::creator, liqCoShaderNode::initialize, MPxNode::kDependNode, &UserClassifyCoShader );
  LIQCHECKSTATUS( status, "Can't register liquidCoShader node" );
  status.clear();
  status = MSwatchRenderRegister::registerSwatchRender( "liqCoShaderSwatch", liqNodeSwatch::creator );
  LIQCHECKSTATUS( status, "Can't register liquidCoShader swatch" );

  status = plugin.registerCommand("liqRibShaderTranslator", liqRibShaderTranslator::creator );
  LIQCHECKSTATUS( status, "Can't register liquid translator command" );

  // register the liquidShaderSwitcher node
  const MString UserClassifySurfaceSwitcher( "shader/surface" );
  status = plugin.registerNode( "liquidSurfaceSwitcher", liqSurfaceSwitcherNode::id, liqSurfaceSwitcherNode::creator, liqSurfaceSwitcherNode::initialize, MPxNode::kDependNode, &UserClassifySurfaceSwitcher );
  LIQCHECKSTATUS( status, "Can't register liquidSurfaceSwitcher node" );
  status.clear();
  //status = MSwatchRenderRegister::registerSwatchRender( "liqSurfSwatch", liqNodeSwatch::creator );
  //LIQCHECKSTATUS( status, "Can't register liquidSurfaceSwitcher swatch" );

  // register the liquidDisplacementSwitcher node
  const MString UserClassifyDisplaceSwitcher( "shader/displacement" );
  status = plugin.registerNode( "liquidDisplacementSwitcher", liqDisplacementSwitcherNode::id, liqDisplacementSwitcherNode::creator, liqDisplacementSwitcherNode::initialize, MPxNode::kDependNode, &UserClassifyDisplaceSwitcher );
  LIQCHECKSTATUS( status, "Can't register liquidDisplacementSwitcher node" );
  status.clear();

  status = plugin.registerCommand("liqParseString", liqParseString::creator );
  LIQCHECKSTATUS( status, "Can't register liquid parseString command" );
  // setup all of the base liquid interface
  MString sourceLine("source ");
  char *tmphomeChar;
  if( ( tmphomeChar = getenv( "LIQUIDHOME" ) ) ) {

#ifndef WIN32

    MString tmphome( tmphomeChar );
    sourceLine += "\"" + liquidSanitizePath( tmphome ) + "/mel/" + "liquidStartup.mel\"";
#else
	for (unsigned k( 0 ); k < strlen(tmphomeChar) ; k++ ) 
		if ( tmphomeChar[ k ] == '\\' ) tmphomeChar[ k ] = '/';
	

	MString tmphome( tmphomeChar );
	sourceLine += "\"" + tmphome + "/mel/" + "liquidStartup.mel\"";
#endif
  } 
  else 
  {
    sourceLine += "\"liquidStartup.mel\"";
  }

  status = MGlobal::executeCommand(sourceLine);
  if ( status == MS::kSuccess )
  {
    MString msg( "Liquid " );
    msg += LIQUIDVERSION;
    msg += " registered";
    liquidMessage( msg, messageInfo );
    status = plugin.registerUI("liquidStartup", "liquidShutdown");
  }
  LIQCHECKSTATUS( status, "Can't register liquidStartup and liquidShutdown interface scripts" );
  
  return MS::kSuccess;
}

LIQUID_EXPORT MStatus uninitializePlugin(MObject obj)
//  Description:
//      Deregister the command when the plug-in is unloaded
{
  MStatus status;
  MFnPlugin plugin(obj);

  status = plugin.deregisterCommand("liquid");
  LIQCHECKSTATUS( status, "Can't deregister liquid command" );

  status = plugin.deregisterCommand("liquidAttachPrefAttribute");
  LIQCHECKSTATUS( status, "Can't deregister liquidAttachPrefAttribute command" );

  status = plugin.deregisterCommand("liquidPreviewShader");
  LIQCHECKSTATUS( status, "Can't deregister liquidPreviewShader command" );

  status = plugin.deregisterCommand("liquidGetSloInfo");
  LIQCHECKSTATUS( status, "Can't deregister liquidGetSloInfo command" );

  status = plugin.deregisterCommand("liquidGetAttr");
  LIQCHECKSTATUS( status, "Can't deregister liquidGetAttr command" );

  status = plugin.deregisterCommand("liquidWriteArchive");
  LIQCHECKSTATUS( status, "Can't deregister liquidWriteArchive command" );

  status = plugin.deregisterCommand("liquidRenderView");
  LIQCHECKSTATUS( status, "Can't deregister liquidRenderView command" );

  status = plugin.deregisterCommand("liquidJobList");
  LIQCHECKSTATUS( status, "Can't deregister liquidJobList command" );

#ifndef NO_RICMD
  status = plugin.deregisterCommand("RIArchiveBegin");
  LIQCHECKSTATUS( status, "Can't deregister RIArchiveBegin command" );

  status = plugin.deregisterCommand("RIArchiveEnd");
  LIQCHECKSTATUS( status, "Can't deregister RIArchiveEnd command" );

  status = plugin.deregisterCommand("RIArchiveRecord");
  LIQCHECKSTATUS( status, "Can't deregister RIArchiveRecord command" );

  status = plugin.deregisterCommand("RIAtmosphere");
  LIQCHECKSTATUS( status, "Can't deregister RIAtmosphere command" );

  status = plugin.deregisterCommand("RIAttribute");
  LIQCHECKSTATUS( status, "Can't deregister RIAttribute command" );

  status = plugin.deregisterCommand("RIAttributeBegin");
  LIQCHECKSTATUS( status, "Can't deregister RIAttributeBegin command" );

  status = plugin.deregisterCommand("RIAttributeEnd");
  LIQCHECKSTATUS( status, "Can't deregister RIAttributeEnd command" );

  status = plugin.deregisterCommand("RIClipping");
  LIQCHECKSTATUS( status, "Can't deregister RIClipping command" );

  status = plugin.deregisterCommand("RIColor");
  LIQCHECKSTATUS( status, "Can't deregister RIColor command" );

  status = plugin.deregisterCommand("RIConcatTransform");
  LIQCHECKSTATUS( status, "Can't deregister RIConcatTransform command" );

  status = plugin.deregisterCommand("RICropWindow");
  LIQCHECKSTATUS( status, "Can't deregister RICropWindow command" );

  status = plugin.deregisterCommand("RIDepthOfField");
  LIQCHECKSTATUS( status, "Can't deregister RIDepthOfField command" );

  status = plugin.deregisterCommand("RIDetail");
  LIQCHECKSTATUS( status, "Can't deregister RIDetail command" );

  status = plugin.deregisterCommand("RIDetailRange");
  LIQCHECKSTATUS( status, "Can't deregister RIDetailRange command" );
  
  status = plugin.deregisterCommand("RIDisplacement");
  LIQCHECKSTATUS( status, "Can't deregister RIDisplacement command" );

  status = plugin.deregisterCommand("RIDisplay");
  LIQCHECKSTATUS( status, "Can't deregister RIDisplay command" );

  status = plugin.deregisterCommand("RIDisplayChannel");
  LIQCHECKSTATUS( status, "Can't deregister RIDisplayChannel command" );

  status = plugin.deregisterCommand("RIExterior");
  LIQCHECKSTATUS( status, "Can't deregister RIExterior command" );

  status = plugin.deregisterCommand("RIFormat");
  LIQCHECKSTATUS( status, "Can't deregister RIFormat command" );

  status = plugin.deregisterCommand("RIFrameBegin");
  LIQCHECKSTATUS( status, "Can't deregister RIFrameBegin command" );

  status = plugin.deregisterCommand("RIFrameEnd");
  LIQCHECKSTATUS( status, "Can't deregister RIFrameEnd command" );

  status = plugin.deregisterCommand("RIGeometricApproximation");
  LIQCHECKSTATUS( status, "Can't deregister RIGeometricApproximation command" );

  status = plugin.deregisterCommand("RIGeometry");
  LIQCHECKSTATUS( status, "Can't deregister RIGeometry command" );

  status = plugin.deregisterCommand("RIHider");
  LIQCHECKSTATUS( status, "Can't deregister RIHider command" );

  status = plugin.deregisterCommand("RIIdentity");
  LIQCHECKSTATUS( status, "Can't deregister RIIdentity command" );

  status = plugin.deregisterCommand("RIIlluminate");
  LIQCHECKSTATUS( status, "Can't deregister RIIlluminate command" );

  status = plugin.deregisterCommand("RIInterior");
  LIQCHECKSTATUS( status, "Can't deregister RIInterior command" );

  status = plugin.deregisterCommand("RILightSource");
  LIQCHECKSTATUS( status, "Can't deregister RILightSource command" );

  status = plugin.deregisterCommand("RIMatte");
  LIQCHECKSTATUS( status, "Can't deregister RIMatte command" );

  status = plugin.deregisterCommand("RIMotionBegin");
  LIQCHECKSTATUS( status, "Can't deregister RIMotionBegin command" );

  status = plugin.deregisterCommand("RIMotionEnd");
  LIQCHECKSTATUS( status, "Can't deregister RIMotionEnd command" );

  status = plugin.deregisterCommand("RIObjectBegin");
  LIQCHECKSTATUS( status, "Can't deregister RIObjectBegin command" );

  status = plugin.deregisterCommand("RIObjectEnd");
  LIQCHECKSTATUS( status, "Can't deregister RIObjectEnd command" );

  status = plugin.deregisterCommand("RIOpacity");
  LIQCHECKSTATUS( status, "Can't deregister RIOpacity command" );

  status = plugin.deregisterCommand("RIOption");
  LIQCHECKSTATUS( status, "Can't deregister RIOption command" );

  status = plugin.deregisterCommand("RIOrientation");
  LIQCHECKSTATUS( status, "Can't deregister RIOrientation command" );

  status = plugin.deregisterCommand("RIPixelSamples");
  LIQCHECKSTATUS( status, "Can't deregister RIPixelSamples command" );

  status = plugin.deregisterCommand("RIProcedural");
  LIQCHECKSTATUS( status, "Can't deregister RIProcedural command" );

  status = plugin.deregisterCommand("RIProjection");
  LIQCHECKSTATUS( status, "Can't deregister RIProjection command" );

  status = plugin.deregisterCommand("RIReadArchive");
  LIQCHECKSTATUS( status, "Can't deregister RIReadArchive command" );

  status = plugin.deregisterCommand("RIRelativeDetail");
  LIQCHECKSTATUS( status, "Can't deregister RIRelativeDetail command" );

  status = plugin.deregisterCommand("RIResource");
  LIQCHECKSTATUS( status, "Can't deregister RIResource command" );

  status = plugin.deregisterCommand("RIResourceBegin");
  LIQCHECKSTATUS( status, "Can't deregister RIResourceBegin command" );

  status = plugin.deregisterCommand("RIResourceEnd");
  LIQCHECKSTATUS( status, "Can't deregister RIResourceEnd command" );

  status = plugin.deregisterCommand("RIReverseOrientation");
  LIQCHECKSTATUS( status, "Can't deregister RIReverseOrientation command" );

  status = plugin.deregisterCommand("RIRotate");
  LIQCHECKSTATUS( status, "Can't deregister RIRotate command" );

  status = plugin.deregisterCommand("RIScale");
  LIQCHECKSTATUS( status, "Can't deregister RIScale command" );

  status = plugin.deregisterCommand("RIScreenWindow");
  LIQCHECKSTATUS( status, "Can't deregister RIScreenWindow command" );

  status = plugin.deregisterCommand("RIShadingInterpolation");
  LIQCHECKSTATUS( status, "Can't deregister RIShadingInterpolation command" );

  status = plugin.deregisterCommand("RIShadingRate");
  LIQCHECKSTATUS( status, "Can't deregister RIShadingRate command" );

  status = plugin.deregisterCommand("RIShutter");
  LIQCHECKSTATUS( status, "Can't deregister RIShutter command" );

  status = plugin.deregisterCommand("RISides");
  LIQCHECKSTATUS( status, "Can't deregister RISides command" );

  status = plugin.deregisterCommand("RISkew");
  LIQCHECKSTATUS( status, "Can't deregister RISkew command" );

  status = plugin.deregisterCommand("RISolidBegin");
  LIQCHECKSTATUS( status, "Can't deregister RISolidBegin command" );

  status = plugin.deregisterCommand("RISolidEnd");
  LIQCHECKSTATUS( status, "Can't deregister RISolidEnd command" );

  status = plugin.deregisterCommand("RISphere");
  LIQCHECKSTATUS( status, "Can't deregister RISphere command" );

  status = plugin.deregisterCommand("RISurface");
  LIQCHECKSTATUS( status, "Can't deregister RISurface command" );

  status = plugin.deregisterCommand("RITransformBegin");
  LIQCHECKSTATUS( status, "Can't deregister RITransformBegin command" );

  status = plugin.deregisterCommand("RITransformEnd");
  LIQCHECKSTATUS( status, "Can't deregister RITransformEnd command" );

  status = plugin.deregisterCommand("RITranslate");
  LIQCHECKSTATUS( status, "Can't deregister RITranslate command" );

  status = plugin.deregisterCommand("RIWorldBegin");
  LIQCHECKSTATUS( status, "Can't deregister RIWorldBegin command" );

  status = plugin.deregisterCommand("RIWorldEnd");
  LIQCHECKSTATUS( status, "Can't deregister RIWorldEnd command" );
  
#endif    

  status = plugin.deregisterNode( liqSurfaceNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidSurface node" );
  status = MSwatchRenderRegister::unregisterSwatchRender( "liqSurfSwatch" );
  LIQCHECKSTATUS( status, "Can't deregister liquidSurface swatch generator" );

  status = plugin.deregisterNode( liqDisplacementNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidDisp node" );
  status = MSwatchRenderRegister::unregisterSwatchRender( "liqDispSwatch" );
  LIQCHECKSTATUS( status, "Can't deregister liquidDisp swatch generator" );

  status = plugin.deregisterNode( liqVolumeNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidVolume node" );
  status = MSwatchRenderRegister::unregisterSwatchRender( "liqVolSwatch" );
  LIQCHECKSTATUS( status, "Can't deregister liquidVolume swatch generator" );

  status = plugin.deregisterNode( liqLightNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidLight node" );
  status = MSwatchRenderRegister::unregisterSwatchRender( "liqLightSwatch" );
  LIQCHECKSTATUS( status, "Can't deregister liquidLight swatch generator" );
  status = plugin.deregisterDragAndDropBehavior( "liquidLightBehavior" );
  LIQCHECKSTATUS( status, "Can't deregister liquidLight behavior" );

  status = plugin.deregisterNode( liqRibboxNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidRibbox node" );
  status = MSwatchRenderRegister::unregisterSwatchRender( "liqRibSwatch" );
  LIQCHECKSTATUS( status, "Can't deregister liquidRibbox swatch generator" );

  status = plugin.deregisterNode( liqRibRequestNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidRibRequest node" );

  status = plugin.deregisterNode( liqCoordSysNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidCoordSys node" );

  status = plugin.deregisterNode( liqBoundingBoxLocator::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidBoundingBox node" );

  status = plugin.deregisterNode( liqGlobalsNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidGlobals node" );

  status = plugin.deregisterNode( liqCoShaderNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidCoShader node" );
  status = MSwatchRenderRegister::unregisterSwatchRender( "liqCoShaderSwatch" );
  LIQCHECKSTATUS( status, "Can't deregister liquidCoShader swatch generator" );

  status = plugin.deregisterCommand("liqRibShaderTranslator");
  LIQCHECKSTATUS( status, "Can't deregister liqRibShaderTranslator command" );

  status = plugin.deregisterNode( liqSurfaceSwitcherNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidSurfaceSwitcher node" );

  status = plugin.deregisterNode( liqDisplacementSwitcherNode::id );
  LIQCHECKSTATUS( status, "Can't deregister liquidDisplacementSwitcher node" );

	status = plugin.deregisterCommand("liqParseString");
  LIQCHECKSTATUS( status, "Can't deregister liqParseString command" );

  MString msg( "Liquid " );
  msg += LIQUIDVERSION;
  msg += " unregistered";
  liquidMessage( msg, messageInfo );

	liqShaderFactory::deleteInstance();

  return MS::kSuccess;
}
