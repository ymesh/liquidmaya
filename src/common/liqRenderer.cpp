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
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>

#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRenderer.h>

extern MObject rGlobalObj;

liqRenderer::liqRenderer()
{
    pixelFilterNames.clear();
    pixelFilterNames.append("box");
    pixelFilterNames.append("triangle");
    pixelFilterNames.append("catmull-rom");
    pixelFilterNames.append("gaussian");
    pixelFilterNames.append("sinc");
    pixelFilterNames.append("blackman-harris");
    pixelFilterNames.append("mitchell");
    pixelFilterNames.append("separable-catmull-rom");
    pixelFilterNames.append("lanczos");
    pixelFilterNames.append("bessel");
    pixelFilterNames.append("disk");
}

liqRenderer::~liqRenderer()
{
}

void liqRenderer::setRenderer()
{
  MStatus status;
  MFnDependencyNode globalsNode( initGlobals() );
  MPlug gPlug;

  liquidGetPlugValue( globalsNode, "renderCommand", renderCommand, status );
  liquidGetPlugValue( globalsNode, "previewer", renderPreview, status );
  liquidGetPlugValue( globalsNode, "renderCmdFlags", renderCmdFlags, status );
  renderCmdFlags = parseString( renderCmdFlags );
  liquidGetPlugValue( globalsNode, "shaderExt", shaderExtension, status );
  liquidGetPlugValue( globalsNode, "shaderInfo", shaderInfo, status );
  liquidGetPlugValue( globalsNode, "shaderComp", shaderCompiler, status );
  liquidGetPlugValue( globalsNode, "makeTexture", textureMaker, status );
  liquidGetPlugValue( globalsNode, "viewTexture", textureViewer, status );
  liquidGetPlugValue( globalsNode, "textureExt", textureExtension, status );

  {
    // get enabled features from the globals.
    // the bits_features attribute is a compound attribute with boolean children attributes.
    // retrieving the list of children gives you the name of the feature.
    // retrieving the children's value tells you if the feature is supported.

    status.clear();
    gPlug = globalsNode.findPlug( "bits_features", &status );
    if ( status == MS::kSuccess ) 
		{
      unsigned int nFeatures = gPlug.numChildren( &status );
      for ( unsigned int i(0); i<nFeatures; i++ ) 
			{
        MPlug featurePlug;
        featurePlug = gPlug.child( i, &status );
        if ( status == MS::kSuccess ) 
				{
          MString feature;
          bool enabled =  false;
          feature = featurePlug.partialName( false, false, false, false, false, &status );
          featurePlug.getValue( enabled );
          feature = feature.toLowerCase();

		      if ( feature == "blobbies" )            supports_BLOBBIES             = enabled;
		      if ( feature == "points" )              supports_POINTS               = enabled;
		      if ( feature == "eyesplits" )           supports_EYESPLITS            = enabled;
		      if ( feature == "raytracing" )          supports_RAYTRACE             = enabled;
		      if ( feature == "depthoffield" )        supports_DOF                  = enabled;
		      if ( feature == "advancedvisibility" )  supports_ADVANCED_VISIBILITY  = enabled;
		      if ( feature == "displaychannels" )     supports_DISPLAY_CHANNELS     = enabled;
        }
      }
    }
  }

  {
    status.clear();
    gPlug = globalsNode.findPlug( "bits_filters", &status );
    if ( status == MS::kSuccess ) 
		{
      unsigned int nFeatures = gPlug.numChildren( &status );
      for ( unsigned int i=0; i<nFeatures; i++ ) 
			{
        MPlug filterPlug;
        filterPlug = gPlug.child( i, &status );
        if ( status == MS::kSuccess ) 
				{
          MString filter;
          bool enabled =  false;
          filter = filterPlug.partialName( false, false, false, false, false, &status );
          filterPlug.getValue( enabled );
          filter = filter.toLowerCase();

          if ( filter == "box" )                   pixelfilter_BOX             = enabled;
          if ( filter == "triangle" )              pixelfilter_TRIANGLE        = enabled;
          if ( filter == "catmull_rom" )           pixelfilter_CATMULLROM      = enabled;
          if ( filter == "gaussian" )              pixelfilter_GAUSSIAN        = enabled;
          if ( filter == "sinc" )                  pixelfilter_SINC            = enabled;
          if ( filter == "blackman_harris" )       pixelfilter_BLACKMANHARRIS  = enabled;
          if ( filter == "mitchell" )              pixelfilter_MITCHELL        = enabled;
          if ( filter == "separablecatmull_rom" )  pixelfilter_SEPCATMULLROM   = enabled;
          if ( filter == "lanczos" )               pixelfilter_LANCZOS         = enabled;
          if ( filter == "bessel" )                pixelfilter_BESSEL          = enabled;
          if ( filter == "disk" )                  pixelfilter_DISK            = enabled;
        }
      }
    }
  }

  {
    status.clear();
    gPlug = globalsNode.findPlug( "bits_hiders", &status );
    if ( status == MS::kSuccess ) 
		{
      unsigned int nFeatures = gPlug.numChildren( &status );
      for ( unsigned int i=0; i<nFeatures; i++ ) 
			{
        MPlug hiderPlug;
        hiderPlug = gPlug.child( i, &status );
        if ( status == MS::kSuccess ) 
				{
          MString hider;
          bool enabled =  false;
          hider = hiderPlug.partialName( false, false, false, false, false, &status );
          hiderPlug.getValue( enabled );
          hider = hider.toLowerCase();

          if ( hider == "hidden" )      hider_HIDDEN    = enabled;
          if ( hider == "photon" )      hider_PHOTON    = enabled;
          if ( hider == "zbuffer" )     hider_ZBUFFER   = enabled;
          if ( hider == "raytrace" )    hider_RAYTRACE  = enabled;
          if ( hider == "opengl" )      hider_OPENGL    = enabled;
          if ( hider == "depthmask" )   hider_DEPTHMASK = enabled;
        }
      }
    }
  }

  {
    status.clear();
    gPlug = globalsNode.findPlug( "bits_required", &status );
    if ( status == MS::kSuccess ) 
		{
      unsigned int nFeatures = gPlug.numChildren( &status );
      for ( unsigned int i=0 ; i<nFeatures ; i++ ) 
			{
        MPlug requiredPlug;
        requiredPlug = gPlug.child( i, &status );
        if ( status == MS::kSuccess ) 
				{
          MString required;
          bool enabled =  false;
          required = requiredPlug.partialName( false, false, false, false, false, &status );
          requiredPlug.getValue( enabled );
          required = required.toLowerCase();

          if ( required == "swap_uv" )     requires_SWAPPED_UVS  = enabled;
          if ( required == "__pref" )      requires__PREF        = enabled;
          if ( required == "makeshadow" )  requires_MAKESHADOW   = enabled;
        }
      }
    }
  }

  liquidGetPlugValue( globalsNode, "dshDisplayName", dshDisplayName, status );
  liquidGetPlugValue( globalsNode, "dshImageMode", dshImageMode, status );

  if ( renderCommand == "" )
	  liquidMessage( "The render command is not defined !!", messageError );
  else 
  {
    int lastSlash = renderCommand.rindex( '/' );
    MString tmp, envvar, homeDir;
    if ( lastSlash < 0 ) tmp = renderCommand;
    else 
    {
      int len = renderCommand.length();
      tmp = renderCommand.substring( lastSlash+1, len-1 );
    }
    if ( tmp == "prman" ) 
    {
      renderName = "PRMan";
      envvar = "RMANTREE";
    } 
    else if ( tmp == "rndr" ) 
    {
      renderName = "Pixie";
      envvar = "PIXIEHOME";
    } 
    else if ( tmp == "renderdl" ) 
    {
      renderName = "3Delight";
      envvar = "DELIGHT";
    } 
    else if ( tmp == "aqsis" ) 
    {
      renderName = "Aqsis";
      envvar = "AQSISHOME";
    } 
    else if ( tmp == "air" ) 
    {
      renderName = "Air";
      envvar = "AIRHOME";
    }
    renderHome = getenv( envvar.asChar() );
    if ( renderHome == "" ) 
      liquidMessage( "The " + envvar + " environment variable (renderHome) is not defined !!", messageError );
  }
  /* cout <<"\nrenderName : "<<renderName<<endl;
  cout <<"  renderCommand   : "<<renderCommand<<endl;
  cout <<"  renderPreview   : "<<renderPreview<<endl;
  cout <<"  renderCmdFlags  : "<<renderCmdFlags<<endl;
  cout <<"  shaderExtension : "<<shaderExtension<<endl;
  cout <<"  shaderInfo      : "<<shaderInfo<<endl;
  cout <<"  shaderCompiler  : "<<shaderCompiler<<endl;
  cout <<"  supports_BLOBBIES            : "<<supports_BLOBBIES<<endl;
  cout <<"  supports_POINTS              : "<<supports_POINTS<<endl;
  cout <<"  supports_EYESPLITS           : "<<supports_EYESPLITS<<endl;
  cout <<"  supports_RAYTRACE            : "<<supports_RAYTRACE<<endl;
  cout <<"  supports_DOF                 : "<<supports_DOF<<endl;
  cout <<"  supports_ADVANCED_VISIBILITY : "<<supports_ADVANCED_VISIBILITY<<endl;
  cout <<"  supports_DISPLAY_CHANNELS    : "<<supports_DISPLAY_CHANNELS<<endl;
  cout <<"  pixelfilter_BOX            : "<<pixelfilter_BOX<<endl;
  cout <<"  pixelfilter_TRIANGLE       : "<<pixelfilter_TRIANGLE<<endl;
  cout <<"  pixelfilter_CATMULLROM     : "<<pixelfilter_CATMULLROM<<endl;
  cout <<"  pixelfilter_GAUSSIAN       : "<<pixelfilter_GAUSSIAN<<endl;
  cout <<"  pixelfilter_SINC           : "<<pixelfilter_SINC<<endl;
  cout <<"  pixelfilter_BLACKMANHARRIS : "<<pixelfilter_BLACKMANHARRIS<<endl;
  cout <<"  pixelfilter_MITCHELL       : "<<pixelfilter_MITCHELL<<endl;
  cout <<"  pixelfilter_SEPCATMULLROM  : "<<pixelfilter_SEPCATMULLROM<<endl;
  cout <<"  pixelfilter_LANCZOS        : "<<pixelfilter_LANCZOS<<endl;
  cout <<"  pixelfilter_BESSEL         : "<<pixelfilter_BESSEL<<endl;
  cout <<"  pixelfilter_DISK           : "<<pixelfilter_DISK<<endl;
  cout <<"  hider_HIDDEN    : "<<hider_HIDDEN<<endl;
  cout <<"  hider_PHOTON    : "<<hider_PHOTON<<endl;
  cout <<"  hider_ZBUFFER   : "<<hider_ZBUFFER<<endl;
  cout <<"  hider_RAYTRACE  : "<<hider_RAYTRACE<<endl;
  cout <<"  hider_OPENGL    : "<<hider_OPENGL<<endl;
  cout <<"  hider_DEPTHMASK : "<<hider_DEPTHMASK<<endl;
  cout <<"  requires_SWAPPED_UVS : "<<requires_SWAPPED_UVS<<endl;
  cout <<"  requires__PREF       : "<<requires__PREF<<endl;
  cout <<"  requires_MAKESHADOW  : "<<requires_MAKESHADOW<<endl;
  cout <<"  dshDisplayName : "<<dshDisplayName<<endl;
  cout <<"  dshImageMode   : "<<dshImageMode<<endl; */
}

MObject liqRenderer::initGlobals()
{
  MStatus status;
  MObject renderGlobalsObj;
  MSelectionList rGlobalList;

  status = rGlobalList.add( "liquidGlobals" );

  if ( rGlobalList.isEmpty() ) 
  {
    MGlobal::executeCommand( "liquidCreateGlobals()", false, false );
    status.clear();
    rGlobalList.clear();
    status = rGlobalList.add( "liquidGlobals" );
  }

  if ( rGlobalList.length() > 0 ) 
  {
    status.clear();
    status = rGlobalList.getDependNode( 0, renderGlobalsObj );
    if ( status == MS::kSuccess ) return renderGlobalsObj;
  }

  return MObject::kNullObj;
}

