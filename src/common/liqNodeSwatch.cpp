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
** Liquid Node Swatch Source
** ______________________________________________________________________
*/

#include <liquid.h>
#include <liqSurfaceNode.h>

#include <maya/MGlobal.h>
#include <maya/MCommandResult.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFloatVector.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MSwatchRenderBase.h>
#include <maya/MSwatchRenderRegister.h>
#include <maya/MImage.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFileObject.h>

#include <liqIOStream.h>

bool liqNodeSwatch::doIteration () 
{
  MStatus status;
  MImage &img = image();
  MObject thenode = node();
  bool refresh = false;
  bool result = true;

  MFnDependencyNode depNode( thenode );
  MString nodename( depNode.name() );
  MString nodeType( depNode.typeName() );
  MString theShader;

  if ( previewPath == "" ) 
  {
    //cout <<"  >> init previewPath"<<endl;
    MGlobal::executeCommand( "liquidFluidGetPreviewDir();", previewPath, false, false ) ;
  }

  // read the value of the refreshPreview attribute.
  MPlug refreshPlug = depNode.findPlug( "refreshPreview", &status );
  if ( status == MS::kSuccess ) refreshPlug.getValue( refresh );
  
  status.clear();

  // get the short shader name
  MPlug shortShaderPlug = depNode.findPlug( "rmanShader", &status );
  if ( status == MS::kSuccess ) shortShaderPlug.getValue( theShader );
  
  status.clear();

  if ( refresh ) 
  {
    //cout <<"refresh !"<<endl;
    MString filePreviewPath = previewPath + "/" + nodename + "_" + theShader + ".tif";
    MString doneFile = previewPath + "/" + nodename + "_" + theShader + ".tif.done";

    MFileObject doneFObj;
    doneFObj.setFullName( doneFile );

    if ( doneFObj.exists() ) 
    {
	    status = img.readFromFile( filePreviewPath );

      if ( status == MS::kSuccess ) 
      {
        img.verticalFlip();
        refreshPlug.setValue( false );
        result = true;
      } 
      else 
      {
        // raise an error
        MString errStr( status.errorString() );
        errStr = "Liquid Preview Swatch : " + errStr;
        MGlobal:: displayError( errStr );
        refreshPlug.setValue( false );
        return true;
      }
    } 
    else 
      return false;
  } 
  else 
  {
    //cout <<"swatch for node "<<nodeType.asChar()<<endl;

    MString filePreviewPath;
    filePreviewPath = previewPath + "/" + nodename + "_" + theShader + ".tif";

    if ( MS::kSuccess == img.readFromFile( filePreviewPath ) ) img.verticalFlip();
    else 
    {
      MString preview;
      MGlobal::executeCommand( "liquidGetHome();", preview, false, false );

      if ( nodeType == "liquidRibBox" )     preview += "/icons/liquidRibBoxSwatch.iff";
      else if ( nodeType == "liquidLight" ) preview += "/icons/liquidLightSwatch.iff";
      else                                  preview += "/icons/noPreview.jpg";

      status = img.readFromFile( preview );
      if ( status == MS::kSuccess ) img.verticalFlip();
      else 
			{
	  		// Try to get a valid image ...
      	img.create( resolution(), resolution() );
				unsigned char *pixels = img.pixels();
				unsigned char * p = pixels;
				for ( unsigned int i=0 ; i < resolution() * resolution() ; i++ )
			  {
					*p = 1; p++;
					*p = 0; p++;
					*p = 0; p++;
					*p = 1; p++;
				}
	  	}
    }
    result = true;
  }
  return result;
}
