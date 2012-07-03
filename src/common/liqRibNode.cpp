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
** Liquid Rib Node Source
** ______________________________________________________________________
*/


// RenderMan headers
extern "C" {
#include <ri.h>
}

// Maya headers
#include <maya/MPlug.h>
#include <maya/MFnLambertShader.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnPhongShader.h>
#include <maya/MPlugArray.h>
#include <maya/MObjectArray.h>
#include <maya/MFnSet.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDagNode.h>
#include <maya/MBoundingBox.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MVectorArray.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnPointArrayData.h>
#include <maya/MPointArray.h>

// Liquid headers
#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRibNode.h>

// Standard/Boost headers
#include <list>
#include <boost/scoped_array.hpp>


#ifdef _WIN32
#undef min
#undef max
#endif

extern int debugMode;

extern MStringArray liqglo_preReadArchive;
extern MStringArray liqglo_preRibBox;
extern MStringArray liqglo_preReadArchiveShadow;
extern MStringArray liqglo_preRibBoxShadow;
extern MString      liqglo_currentNodeName;
extern MString      liqglo_currentNodeShortName;


/**
 * Class constructor.
 */
liqRibNode::liqRibNode( liqRibNodePtr instanceOfNode,
                        const MString instanceOfNodeStr )
  :   matXForm( MRX_Const ),
      bodyXForm( MRX_Const ),
      instance( instanceOfNode ),
      instanceStr( instanceOfNodeStr ),
      overrideColor( false )
{
  LIQDEBUGPRINTF( "-> creating rib node\n");
  objects.resize( LIQMAXMOTIONSAMPLES );
  //for( unsigned i = 0; i < LIQMAXMOTIONSAMPLES; i++ )
  //  objects[ i ] = NULL;

  name.clear();
  mayaMatteMode             = false;

  shading.shadingRate       = -1.0f;
  shading.diceRasterOrient  = true;
  shading.color.r           = -1.0;
  shading.opacity.r         = -1.0;
  shading.matte             = false;
  shading.doubleShaded      = false;

  trace.displacements       = false;
  trace.sampleMotion        = false;
  trace.bias                = 0.01f;
  trace.maxDiffuseDepth     = 1;
  trace.maxSpecularDepth    = 2;

  visibility.camera         = true;
  // philippe: pre-prman 12.5 style
  visibility.trace          = false;
  visibility.transmission   = visibility::TRANSMISSION_TRANSPARENT;
  // philippe: prman 12.5 style
  visibility.diffuse        = false;
  visibility.specular       = false;
  visibility.newtransmission = false;
  visibility.midpoint       = true;
  visibility.photon         = false;
  
  shade.strategy = shade::SHADE_STRATEGY_GRIDS;
  shade.volumeIntersectionStrategy = shade::SHADE_VOLUMEINTERSECTIONSTRATEGY_EXCLUSIVE;
  shade.volumeIntersectionPriority = 0.0;

  hitmode.diffuse           = hitmode::DIFFUSE_HITMODE_PRIMITIVE;
  hitmode.specular          = hitmode::SPECULAR_HITMODE_SHADER;
  hitmode.transmission      = hitmode::TRANSMISSION_HITMODE_SHADER;
  hitmode.camera            = hitmode::CAMERA_HITMODE_SHADER;

  irradiance.shadingRate    = 1.0f;
  irradiance.nSamples       = 64;
  irradiance.maxError       = 0.5f;
  irradiance.maxPixelDist   = 30.0f;
  irradiance.handle         = "";
  irradiance.fileMode       = irradiance::FILEMODE_NONE;

  photon.globalMap          = "";
  photon.causticMap         = "";
  photon.shadingModel       = photon::SHADINGMODEL_MATTE;
  photon.estimator          = 100;

  motion.transformationBlur = true;
  motion.deformationBlur    = true;
  motion.samples            = 2;
  motion.factor             = 1.0;

  rib.box                   = "";
  rib.generator             = "";
  rib.readArchive           = "";
  rib.delayedReadArchive    = "";

  delightSSS.doScatter = false;
  delightSSS.shadingRate = 1.0;
  delightSSS.groupName = "";
  delightSSS.scattering.r = 0.0;
  delightSSS.scattering.g = 0.0;
  delightSSS.scattering.b = 0.0;
  delightSSS.absorption.r = 0.0;
  delightSSS.absorption.g = 0.0;
  delightSSS.absorption.b = 0.0;
  delightSSS.refraction = 0.0;
  delightSSS.scale = 1.0;

  invisible                 = false;
  ignoreShapes              = false;
}

/**
 * Class destructor.
 */
liqRibNode::~liqRibNode()
{
  LIQDEBUGPRINTF( "-> killing rib node %s\n", name.asChar() );

  for ( unsigned i( 0 ); i < LIQMAXMOTIONSAMPLES; i++ ) 
	{
    if ( objects[ i ] ) 
		{
      LIQDEBUGPRINTF( "-> killing %d. ref\n", i );
      objects[ i ]->unref();
      //objects[ i ] = NULL;
    }
  }
  LIQDEBUGPRINTF( "-> killing no obj\n" );
  //name.clear();
  //irradiance.handle.clear();
  //photon.globalMap.clear();
  //photon.causticMap.clear();
  //rib.box.clear();
  //rib.generator.clear();
  //rib.readArchive.clear();
  //rib.delayedReadArchive.clear();

  LIQDEBUGPRINTF( "-> finished killing rib node.\n" );
}

/**
 * Get the object referred to by this node.
 * This returns the surface, mesh, light, etc. this node points to.
 */
liqRibObjPtr liqRibNode::object( unsigned interval )
{
  return objects[ interval ];
}

/**
 * Set this node with the given path.
 * If this node already refers to the given object, then it is assumed that the
 * path represents the object at the next frame.
 * This method also scans the dag upwards and thereby sets any attributes
 * Liquid knows that have non-default values and sets them for to this node.
 */
void liqRibNode::set( const MDagPath &path, int sample, ObjectType objType, int particleId )
{
  LIQDEBUGPRINTF( "-> setting rib node\n");
  DagPath = path;
#if 0
  int instanceNum = path.instanceNumber();
#endif
  MStatus status;
  MFnDagNode fnNode( path );
  MPlug nPlug;
  status.clear();

  MSelectionList hierarchy; // needed to find objectSets later below
  MDagPath dagSearcher( path );

  liqglo_currentNodeName      = path.fullPathName();
  liqglo_currentNodeShortName = path.partialPathName();

  do 
	{ // while( dagSearcher.length() > 0 )
    dagSearcher.pop(); // Go upwards (should be a transform node)

    hierarchy.add( dagSearcher, MObject::kNullObj, true );

    if ( dagSearcher.apiType( &status ) == MFn::kTransform ) 
    {
      MFnDagNode nodePeeker( dagSearcher );

      // Shading. group ----------------------------------------------------------
      if ( !invisible ) 
			{
        if ( liquidGetPlugValue( nodePeeker, "template", invisible, status ) == MS::kSuccess ) 
        {
          if ( invisible ) break; 
					// Exit do..while loop -- IF OBJECT ATTRIBUTES NEED TO BE PARSED FOR INVISIBLE OBJECTS TOO IN THE FUTURE -- REMOVE THIS LINE!
        } 
        else 
        {
          if ( liquidGetPlugValue( nodePeeker, "liqInvisible", invisible, status ) == MS::kSuccess ) 
					{
            if ( invisible ) break; 
						// Exit do..while loop -- IF OBJECT ATTRIBUTES NEED TO BE PARSED FOR INVISIBLE OBJECTS TOO IN THE FUTURE -- REMOVE THIS LINE!
          }
        }
      }

      if ( shading.shadingRate == -1.0f ) 
        liquidGetPlugValue( nodePeeker, "liqShadingRate", shading.shadingRate, status );

      if ( shading.diceRasterOrient == true ) 
        liquidGetPlugValue( nodePeeker, "liqDiceRasterOrient", shading.diceRasterOrient, status );
 
      if ( shading.color.r == -1.0f ) 
      {
        status.clear();
        nPlug = nodePeeker.findPlug( MString( "liqColor"), &status );
        if ( status == MS::kSuccess ) 
        {
          MPlug tmpPlug;
          tmpPlug = nPlug.child(0,&status);
          if ( status == MS::kSuccess ) tmpPlug.getValue( shading.color.r );
          tmpPlug = nPlug.child(1,&status);
          if ( status == MS::kSuccess ) tmpPlug.getValue( shading.color.g );
          tmpPlug = nPlug.child(2,&status);
          if ( status == MS::kSuccess ) tmpPlug.getValue( shading.color.b );
        }
      }

      if ( shading.opacity.r == -1.0f ) 
      {
        status.clear();
        nPlug = nodePeeker.findPlug( MString( "liqOpacity"), &status );
        if ( status == MS::kSuccess) 
        {
        	MPlug tmpPlug;
          tmpPlug = nPlug.child(0,&status);
          if ( status == MS::kSuccess ) tmpPlug.getValue( shading.opacity.r );
          tmpPlug = nPlug.child(1,&status);
          if ( status == MS::kSuccess ) tmpPlug.getValue( shading.opacity.g );
          tmpPlug = nPlug.child(2,&status);
          if ( status == MS::kSuccess ) tmpPlug.getValue( shading.opacity.b );
        }
      }

      liquidGetPlugValue( nodePeeker, "liqMatte", shading.matte, status );
      liquidGetPlugValue( nodePeeker, "liqDoubleShaded", shading.doubleShaded, status );

      // trace group ----------------------------------------------------------
      
      if ( trace.sampleMotion == false ) 
        liquidGetPlugValue( nodePeeker, "liqTraceSampleMotion", trace.sampleMotion, status );
      
      if ( trace.displacements == false ) 
        liquidGetPlugValue( nodePeeker, "liqTraceDisplacements", trace.displacements, status );

      if ( trace.bias == 0.01f ) 
        liquidGetPlugValue( nodePeeker, "liqTraceBias", trace.bias, status );

      if ( trace.maxDiffuseDepth == 1 ) 
        liquidGetPlugValue( nodePeeker, "liqMaxDiffuseDepth", trace.maxDiffuseDepth, status );

      if ( trace.maxSpecularDepth == 2 ) 
        liquidGetPlugValue( nodePeeker, "liqMaxSpecularDepth", trace.maxSpecularDepth, status );

      // visibility group -----------------------------------------------------

      if ( visibility.camera == true ) 
        liquidGetPlugValue( nodePeeker, "liqVisibilityCamera", visibility.camera, status );

      // philippe : deprecated in prman 12.5
      if ( visibility.trace == false ) 
        liquidGetPlugValue( nodePeeker, "liqVisibilityTrace", visibility.trace, status );

      if ( visibility.transmission == visibility::TRANSMISSION_TRANSPARENT ) 
        liquidGetPlugValue( nodePeeker, "liqVisibilityTransmission", ( int& )visibility.transmission, status );

      // philippe : new visibility attributes in prman 12.5

      if ( visibility.diffuse == false ) 
        liquidGetPlugValue( nodePeeker, "liqVisibilityDiffuse", visibility.diffuse, status );

      if ( visibility.specular == false ) 
        liquidGetPlugValue( nodePeeker, "liqVisibilitySpecular", visibility.specular, status );

      if ( visibility.newtransmission == false ) 
        liquidGetPlugValue( nodePeeker, "liqVisibilityNewTransmission", visibility.newtransmission, status );

      if ( visibility.photon == false ) 
        liquidGetPlugValue( nodePeeker, "liqVisibilityPhoton", visibility.photon, status );
      
      // ymesh: new shade attrributes in prman 16.x
      if ( shade.strategy == shade::SHADE_STRATEGY_GRIDS ) 
        liquidGetPlugValue( nodePeeker, "liqShadeStrategy", (int&)shade.strategy, status ); 
      if ( shade.volumeIntersectionStrategy == shade::SHADE_VOLUMEINTERSECTIONSTRATEGY_EXCLUSIVE ) 
        liquidGetPlugValue( nodePeeker, "liqVolumeIntersectionStrategy", (int&)shade.volumeIntersectionStrategy, status ); 
      if ( shade.volumeIntersectionPriority == 0.0 ) 
        liquidGetPlugValue( nodePeeker, "liqVolumeIntersectionPriority", shade.volumeIntersectionPriority, status ); 

      // philippe : new shading hit-mode attributes in prman 12.5

      if ( hitmode.camera == hitmode::CAMERA_HITMODE_SHADER ) 
        liquidGetPlugValue( nodePeeker, "liqHitModeCamera", (int&)hitmode.camera, status );

      if ( hitmode.diffuse == hitmode::DIFFUSE_HITMODE_PRIMITIVE ) 
        liquidGetPlugValue( nodePeeker, "liqHitModeDiffuse", (int&)hitmode.diffuse, status );

      if ( hitmode.specular == hitmode::SPECULAR_HITMODE_SHADER ) 
        liquidGetPlugValue( nodePeeker, "liqHitModeSpecular", (int&)hitmode.specular, status );

      if ( hitmode.transmission == hitmode::TRANSMISSION_HITMODE_SHADER ) 
        liquidGetPlugValue( nodePeeker, "liqHitModeTransmission", (int&)hitmode.transmission, status );

      // irradiance group -----------------------------------------------------

      if ( irradiance.shadingRate == 1.0f ) 
        liquidGetPlugValue( nodePeeker, "liqIrradianceShadingRate", irradiance.shadingRate, status );

      if ( irradiance.nSamples == 64 ) 
        liquidGetPlugValue( nodePeeker, "liqIrradianceNSamples", irradiance.nSamples, status );

      if ( irradiance.maxError == 0.5f ) 
        liquidGetPlugValue( nodePeeker, "liqIrradianceMaxError", irradiance.maxError, status );

      if ( irradiance.maxPixelDist == 30.0f ) 
        liquidGetPlugValue( nodePeeker, "liqIrradianceMaxPixelDist", irradiance.maxPixelDist, status );

      if ( irradiance.handle == "" ) 
        liquidGetPlugValue( nodePeeker, "liqIrradianceHandle", irradiance.handle, status );

      if ( irradiance.fileMode == irradiance::FILEMODE_NONE ) 
        liquidGetPlugValue( nodePeeker, "liqIrradianceFileMode", (int&)irradiance.fileMode, status );

      // photon group ---------------------------------------------------------

      if ( photon.globalMap == "" ) 
        liquidGetPlugValue( nodePeeker, "liqPhotonGlobalMap", photon.globalMap, status );

      if ( photon.causticMap == "" ) 
        liquidGetPlugValue( nodePeeker, "liqPhotonCausticMap", photon.causticMap, status );
      
      if ( photon.shadingModel == photon::SHADINGMODEL_MATTE ) 
        liquidGetPlugValue( nodePeeker, "liqPhotonShadingModel", (int&)photon.shadingModel, status );

      if ( photon.estimator == 100 ) 
        liquidGetPlugValue( nodePeeker, "liqPhotonEstimator", photon.estimator, status );

      // Motion blur group ---------------------------------------------------------
      // DOES NOT SEEM TO OVERRIDE GLOBALS
      if ( motion.transformationBlur == true ) 
        liquidGetPlugValue( nodePeeker, "liqTransformationBlur", motion.transformationBlur, status );
      
      if ( motion.deformationBlur == true ) 
        liquidGetPlugValue( nodePeeker, "liqDeformationBlur", motion.deformationBlur, status );
      
      if ( motion.samples == 2 ) 
        liquidGetPlugValue( nodePeeker, "liqMotionSamples", motion.samples, status );

      if ( motion.factor == 1.0f ) 
        liquidGetPlugValue( nodePeeker, "liqMotionFactor", motion.factor, status );

		  // 3Delight sss group ---------------------------------------------------------
      if ( liquidGetPlugValue( nodePeeker, "liqDelightSSShadingRate", delightSSS.shadingRate, status ) == MS::kSuccess )
			  delightSSS.doScatter = true;

      liquidGetPlugValue( nodePeeker, "liqDelightSSGroupName", delightSSS.groupName, status );
      liquidGetPlugValue( nodePeeker, "SSScattering1", delightSSS.scattering.r, status );
      liquidGetPlugValue( nodePeeker, "SSScattering2", delightSSS.scattering.g, status );
      liquidGetPlugValue( nodePeeker, "SSScattering3", delightSSS.scattering.b, status );
  		
		  liquidGetPlugValue( nodePeeker, "SSAbsorption1", delightSSS.absorption.r, status );
      liquidGetPlugValue( nodePeeker, "SSAbsorption2", delightSSS.absorption.g, status );
		  liquidGetPlugValue( nodePeeker, "SSAbsorption3", delightSSS.absorption.b, status );

      liquidGetPlugValue( nodePeeker, "SSMeanfreepath1", delightSSS.meanfreepath.r, status );
      liquidGetPlugValue( nodePeeker, "SSMeanfreepath2", delightSSS.meanfreepath.g, status );
		  liquidGetPlugValue( nodePeeker, "SSMeanfreepath3", delightSSS.meanfreepath.b, status );

      liquidGetPlugValue( nodePeeker, "SSReflectance1", delightSSS.reflectance.r, status );
      liquidGetPlugValue( nodePeeker, "SSReflectance2", delightSSS.reflectance.g, status );
		  liquidGetPlugValue( nodePeeker, "SSReflectance3", delightSSS.reflectance.b, status );

      liquidGetPlugValue( nodePeeker, "liqDelightSSReferenceCamera", delightSSS.referencecamera, status );

      liquidGetPlugValue( nodePeeker, "liqDelightSSRefraction", delightSSS.refraction, status );
      liquidGetPlugValue( nodePeeker, "liqDelightSSScale", delightSSS.scale, status );

      // 3Delight light group ---------------------------------------------------------

      // RIB group ---------------------------------------------------------

      if ( rib.box == "" ) 
      {
        MString ribBoxValue;
				bool disableRibBoxParsing = 0;

				liquidGetPlugValue( nodePeeker, "liqDisableRibBoxParsing", disableRibBoxParsing, status );
        
        if ( liquidGetPlugValue( nodePeeker, "liqRIBBox", ribBoxValue, status ) == MS::kSuccess ) 
        {
          if ( ribBoxValue.substring(0,2) == "*H*" ) 
          {
            MString parseThis = ribBoxValue.substring(3, ribBoxValue.length() - 1 );
            liqglo_preRibBox.append( parseString( parseThis ) );
          } 
          else if ( ribBoxValue.substring(0,3) == "*SH*" ) 
          {
            MString parseThis = ribBoxValue.substring(3, ribBoxValue.length() - 1 );
            liqglo_preRibBoxShadow.append( parseString( parseThis ) );
          }
        }
        rib.box = ( ribBoxValue == "" )? "-" : (( disableRibBoxParsing )? ribBoxValue : parseString( ribBoxValue ));
      }

      if ( rib.readArchive == "" ) 
      {
        MString archiveValue;
        if ( liquidGetPlugValue( nodePeeker, "liqRIBReadArchive", archiveValue, status ) == MS::kSuccess ) 
        {
          if ( archiveValue.substring(0,2) == "*H*" ) 
          {
            MString parseThis = archiveValue.substring(3, archiveValue.length() - 1 );
            liqglo_preReadArchive.append( parseString( parseThis ) );
          } 
					else if ( archiveValue.substring(0,3) == "*SH*" ) 
          {
            MString parseThis = archiveValue.substring(3, archiveValue.length() - 1 );
            liqglo_preReadArchiveShadow.append( parseString( parseThis ) );
          }
        }
        rib.readArchive = ( archiveValue == "" )? "-" : parseString(archiveValue);
      }

      if ( rib.delayedReadArchive == "" ) 
      {
        MString delayedArchiveString, delayedArchiveValue;
        if ( liquidGetPlugValue( nodePeeker, "liqRIBDelayedReadArchive", delayedArchiveValue, status ) == MS::kSuccess ) 
        {
          delayedArchiveString = parseString( delayedArchiveValue );

          MStatus Dstatus;
          MPlug delayedPlug = fnNode.findPlug( MString( "ribDelayedArchiveBBox" ), &Dstatus );
          if ( ( Dstatus == MS::kSuccess ) && ( delayedPlug.isConnected() ) ) 
          {
            MPlugArray delayedNodeArray;
            delayedPlug.connectedTo( delayedNodeArray, true, true );
            MObject delayedNodeObj;
            delayedNodeObj = delayedNodeArray[0].node();
            MFnDagNode delayedfnNode( delayedNodeObj );

            MBoundingBox bounding = delayedfnNode.boundingBox();
            MPoint bMin = bounding.min();
            MPoint bMax = bounding.max();
            bound[0] = bMin.x;
            bound[1] = bMin.y;
            bound[2] = bMin.z;
            bound[3] = bMax.x;
            bound[4] = bMax.y;
            bound[5] = bMax.z;
          } 
          else 
          {
            // here, we are going to calculate the bounding box of the gprim
            //

            // get the dagPath
            MDagPath fullPath( nodePeeker.dagPath() );
            fullPath.extendToShape();
            /* cout <<"  + "<<fullPath.fullPathName()<<endl; */

            // get the full transform
            MMatrix currentMatrix( fullPath.inclusiveMatrixInverse() );
            /* cout <<"  + got matrix "<<currentMatrix<<endl; */

            // get the bounding box
            MFnDagNode shapeNode( fullPath );
            MBoundingBox bounding = shapeNode.boundingBox();
            /* cout <<"  + got bbox "<<endl; */

            // retrieve the bounding box expansion attribute
            double expansion;
            if ( liquidGetPlugValue( nodePeeker, "liqRIBDelayedReadArchiveBBoxScale", expansion, Dstatus ) == MS::kSuccess ) 
            {
              /* cout <<"  + found scale attr"<<endl; */
              if ( expansion != 1.0 ) 
              {
                /* cout <<"  + expansion = "<<expansion<<endl; */
                MTransformationMatrix bboxScale;
                double exp[3] = {expansion, expansion, expansion};
                bboxScale.setScale( exp, MSpace::kTransform );
                bounding.transformUsing( bboxScale.asMatrix() );
              }
            }
            // transform it to account for flattened transforms
            //bounding.transformUsing( currentMatrix );
            MPoint bMin = bounding.min() ;
            MPoint bMax = bounding.max() ;
            bound[0] = bMin.x;
            bound[1] = bMin.y;
            bound[2] = bMin.z;
            bound[3] = bMax.x;
            bound[4] = bMax.y;
            bound[5] = bMax.z;
          }
        }
        rib.delayedReadArchive = ( delayedArchiveString == "" )? "-" : delayedArchiveString;
      }

      if ( ignoreShapes == false )
        liquidGetPlugValue( nodePeeker, "liqIgnoreShapes", ignoreShapes, status );
 
      // MFnDependencyNode nodeFn( nodePeeker );

      // find the attributes
      MStringArray floatAttributesFound  = findAttributesByPrefix( "rmanF", nodePeeker );
      MStringArray pointAttributesFound  = findAttributesByPrefix( "rmanP", nodePeeker );
      MStringArray vectorAttributesFound = findAttributesByPrefix( "rmanV", nodePeeker );
      MStringArray normalAttributesFound = findAttributesByPrefix( "rmanN", nodePeeker );
      MStringArray colorAttributesFound  = findAttributesByPrefix( "rmanC", nodePeeker );
      MStringArray stringAttributesFound = findAttributesByPrefix( "rmanS", nodePeeker );

      if ( floatAttributesFound.length() > 0 ) 
      {
        for ( unsigned i( 0 ); i < floatAttributesFound.length(); i++ ) 
        {
          liqTokenPointer tokenPointerPair;
          MString cutString( floatAttributesFound[i].substring( 5, floatAttributesFound[i].length() ) );
          MPlug fPlug( nodePeeker.findPlug( floatAttributesFound[i] ) );
          MObject plugObj;
          status = fPlug.getValue( plugObj );
          
          if ( plugObj.apiType() == MFn::kDoubleArrayData ) 
          {
            MFnDoubleArrayData fnDoubleArrayData( plugObj );
            const MDoubleArray& doubleArrayData( fnDoubleArrayData.array( &status ) );
            tokenPointerPair.set( cutString.asChar(), rFloat, doubleArrayData.length() );
            
            for ( unsigned kk( 0 ); kk < doubleArrayData.length(); kk++ ) 
              tokenPointerPair.setTokenFloat( kk, doubleArrayData[kk] );
          } 
          else 
          {
            if ( fPlug.isArray() ) 
            {
              unsigned nbElts( fPlug.evaluateNumElements() );
              float floatValue;
              tokenPointerPair.set( cutString.asChar(),
                                    rFloat,
                                    false,
                                    true, // philippe :passed as uArray, otherwise it will think it is a single float
                                    nbElts );
              MPlug elementPlug;
              for ( unsigned kk( 0 ); kk < nbElts; kk++ ) 
              {
                elementPlug = fPlug.elementByPhysicalIndex(kk);
                elementPlug.getValue( floatValue );
                tokenPointerPair.setTokenFloat( kk, floatValue );
              }
            } 
            else 
            {
              float floatValue;
              tokenPointerPair.set( cutString.asChar(), rFloat );
              fPlug.getValue( floatValue );
              tokenPointerPair.setTokenFloat( 0, floatValue );
            }
          }
          if ( tokenPointerMap.end() == tokenPointerMap.find( tokenPointerPair.getDetailedTokenName() ) ) 
            tokenPointerMap[ tokenPointerPair.getDetailedTokenName() ] = tokenPointerPair;
        }
      }

      if ( pointAttributesFound.length() > 0 ) 
      {
        for ( unsigned i( 0 ); i < pointAttributesFound.length(); i++ ) 
        {
          liqTokenPointer tokenPointerPair;
          MString cutString( pointAttributesFound[i].substring( 5, pointAttributesFound[i].length() ) );
          MPlug pPlug( nodePeeker.findPlug( pointAttributesFound[i] ) );
          MObject plugObj;
          status = pPlug.getValue( plugObj );
          if ( plugObj.apiType() == MFn::kPointArrayData ) 
          {
            MFnPointArrayData  fnPointArrayData( plugObj );
            MPointArray pointArrayData = fnPointArrayData.array( &status );
            tokenPointerPair.set( cutString.asChar(), rPoint, pointArrayData.length() );
            for ( unsigned kk( 0 ); kk < pointArrayData.length(); kk++ ) 
              tokenPointerPair.setTokenFloat( kk, pointArrayData[kk].x, pointArrayData[kk].y, pointArrayData[kk].z );
          } 
          else 
          {
            // Hmmmm float ? double ?
            float x, y, z;
            tokenPointerPair.set( cutString.asChar(), rPoint );
            // Hmmm should check as for arrays if we are in nurbs mode : 4 values
            pPlug.child(0).getValue( x );
            pPlug.child(1).getValue( y );
            pPlug.child(2).getValue( z );
            tokenPointerPair.setTokenFloat( 0, x, y, z );
          }
          if ( tokenPointerMap.end() == tokenPointerMap.find( tokenPointerPair.getDetailedTokenName() ) ) 
            tokenPointerMap[ tokenPointerPair.getDetailedTokenName() ] = tokenPointerPair;
        }
      }
      parseVectorAttributes( nodePeeker, vectorAttributesFound, rVector );
      parseVectorAttributes( nodePeeker, normalAttributesFound, rNormal );
      parseVectorAttributes( nodePeeker, colorAttributesFound,  rColor  );

      if ( stringAttributesFound.length() > 0 ) 
      {
        for ( unsigned i( 0 ); i < stringAttributesFound.length(); i++ ) 
        {
          liqTokenPointer tokenPointerPair;
          MString cutString( stringAttributesFound[i].substring( 5, stringAttributesFound[i].length() ) );
          MPlug sPlug( nodePeeker.findPlug( stringAttributesFound[i] ) );
          MObject plugObj;
          status = sPlug.getValue( plugObj );
          tokenPointerPair.set( cutString.asChar(), rString );
          MString stringVal;
          sPlug.getValue( stringVal );
					stringVal = parseString( stringVal );
          tokenPointerPair.setTokenString( 0, stringVal.asChar() );
          
          if ( tokenPointerMap.end() == tokenPointerMap.find( tokenPointerPair.getDetailedTokenName() ) ) 
            tokenPointerMap[ tokenPointerPair.getDetailedTokenName() ] = tokenPointerPair;
        }
      }
    } // if( dagSearcher.apiType( &status ) == MFn::kTransform )
  } while ( dagSearcher.length() > 0 );

	// Raytracing Sets membership handling
	if ( grouping.membership.empty() )
	{
		MObjectArray setArray;
		MGlobal::getAssociatedSets( hierarchy, setArray );
		for ( unsigned i( 0 ); i < setArray.length(); i++ )
		{
			MFnDependencyNode depNodeFn( setArray[ i ] );
			bool value = false;

      if ( liquidGetPlugValue( depNodeFn, "liqTraceSet", value, status ) == MS::kSuccess )
			{
				if ( value )
				{
					status.clear();
					grouping.membership += "," + string( depNodeFn.name( &status ).asChar() );
				}
			}
		}
		if ( !grouping.membership.empty() )
			grouping.membership = grouping.membership.substr( 1 );
    status.clear();
	}

  // Get the object's color
  if ( objType != MRT_Shader ) 
  {
    MObject shadingGroup = findShadingGroup( path, objType );
    if ( shadingGroup != MObject::kNullObj ) 
    {
      assignedShadingGroup.setObject( shadingGroup );
      MObject surfaceShader = findShader();
      assignedShader.setObject( surfaceShader );
      assignedDisp.setObject( findDisp() );
      assignedVolume.setObject( findVolume() );
      if ( ( surfaceShader == MObject::kNullObj ) || !getColor( surfaceShader, color ) ) 
        color.r = -1.0; // This is how we specify that the color was not found.
      if ( ( surfaceShader == MObject::kNullObj ) || !getOpacity( surfaceShader, opacity ) ) 
        opacity.r = -1.0; // This is how we specify that the opacity was not found.
      mayaMatteMode = getMatteMode( surfaceShader );
    } 
    else 
    {
      color.r = -1.0;
      opacity.r = -1.0;
    }
    doubleSided = isObjectTwoSided( path );
    reversedNormals = isObjectReversed( path );
  }

  // Check to see if the object should have its color overridden
  // (if possible).
  //
  bool override;
  if ( liquidGetPlugValue( fnNode, "useParticleColorWhenInstanced", override, status ) == MS::kSuccess ) 
  {
    if ( override && particleId != -1 ) 
    {
      // Traverse upwards, looking for some connection between this
      // geometry hierarchy and a particle instancer node.
      //
      MFnDagNode dagNode( path.node() );
      bool foundInstancerNode = false;
      while ( true ) 
      {
        // The instancer is always connected to the "matrix" attribute.
        //
        MPlug matrixPlug = dagNode.findPlug( MString( "matrix" ), &status );
        if ( status != MS::kSuccess ) break;
        // If the matrix plug is connected, iterate over the connections
        // to see if one of them is an instancer.
        //
        if ( matrixPlug.isConnected() ) 
				{
          MPlugArray connections;
          matrixPlug.connectedTo( connections, false, true );
          for ( unsigned i( 0 ); i < connections.length() ; i++  )
          {
            MObject obj = connections[i].node();
            if ( obj.hasFn( MFn::kInstancer ) )
            {
              dagNode.setObject( obj );
              foundInstancerNode = true;
              break;
            }
          }
        }
        // If we've found an instancer or we're at the top of the
        // hierarchy, break.
        //
        if ( foundInstancerNode || dagNode.parentCount() == 0 ) break;
        dagNode.setObject( dagNode.parent( 0 ) );
      }
      // If we've got an instancer, find the associated particle system.
      //
      if ( foundInstancerNode )
      {
        // Find out what particles we're replacing.
        //
        MPlug inputPointsPlug = dagNode.findPlug( "inputPoints", &status );
        if ( status == MS::kSuccess )
        {
          // Find the array of connected plugs.
          //
          MPlugArray sourcePlugArray;
          inputPointsPlug.connectedTo( sourcePlugArray, true, false, &status );

          // There SHOULD always be a connected plug,
          // but this is a safety check.
          //
          if ( sourcePlugArray.length() > 0 )
          {
            MObject sourceObject = sourcePlugArray[0].node();

            // Another sanity check: make sure the source is
            // actually a particle system.
            //
            if ( sourceObject.hasFn( MFn::kParticle ) || sourceObject.hasFn( MFn::kNParticle ) )
            {
              MFnParticleSystem particles( sourceObject );

              // Proceed with color overrides if the rgbPP exists.
              //
              if ( particles.hasRgb() )
              {
                MVectorArray rgbPP;
                particles.rgb( rgbPP );

                // Find the ID's
                //
                MPlug idPlug = particles.findPlug( "id", &status );
                if ( status == MS::kSuccess )
                {
                  MObject idObject;
                  idPlug.getValue( idObject );
                  MFnDoubleArrayData idArray( idObject, &status );

                  // Look for an id that matches.
                  //
                  for ( int i = 0; i < idArray.length(); i++ )
                  {
                    // If a match is found, grab the color.
                    //
                    if ( static_cast<int>( idArray[i] ) == particleId )
                    {
                      color.r = rgbPP[i].x;
                      color.g = rgbPP[i].y;
                      color.b = rgbPP[i].z;
                      overrideColor = true;
                      break;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  // Create a new RIB object for the given path
  //
  LIQDEBUGPRINTF( "-> creating rib object for given path\n");
  MObject obj( path.node() );
  liqRibObjPtr no( new liqRibObj( path, objType ) );
  LIQDEBUGPRINTF( "-> creating rib object for reference\n");
  no->ref();
  name = path.fullPathName();
  LIQDEBUGPRINTF( "-> getting objects name %s\n", name.asChar() );
  if ( name == "" ) 
  {
    LIQDEBUGPRINTF( "-> name unknown -- searching for name\n");
    MDagPath parentPath = path;
    parentPath.pop();

    name = parentPath.fullPathName( &status );
    LIQDEBUGPRINTF( "-> found name: %s\n", name.asChar() );
  }
  if ( objType == MRT_RibGen ) name += "RIBGEN";
  
  LIQDEBUGPRINTF( "-> inserting object into ribnode's obj sample table\n" );
  if ( !objects[ sample ] ) 
    objects[ sample ] = no;
  else 
  {
	  objects[ sample ]->unref();
    objects[ sample ] = no;
  }
  LIQDEBUGPRINTF( "-> done creating rib object for given path\n");
}
/**
 * Return the path in the DAG to the instance that this node represents.
 */
MDagPath & liqRibNode::path()
{
  return DagPath;
}
/**
 * Find the shading group assigned to the given object.
 */
MObject liqRibNode::findShadingGroup( const MDagPath& path, ObjectType type )
{
	LIQDEBUGPRINTF( "-> finding rib node shading group\n");

	// Alf: the case of a custom shading group assigned to the shape directly
	MStatus status;
	MFnDagNode fnDagNode( path.transform() );
	MPlug rmanSGPlug( fnDagNode.findPlug( MString( "liquidCustomShadingGroup" ), &status ) );
	if ( status == MS::kSuccess && rmanSGPlug.isConnected() )
	{
		MPlugArray rmSGArray;
		rmanSGPlug.connectedTo( rmSGArray, true, true );
		MObject rmSGObj( rmSGArray[0].node() );
		MFnDependencyNode SGDepNode( rmSGObj );
		if ( SGDepNode.typeName() == "shadingEngine" ) return rmSGObj;
	}

	// Alf: Paint effects shading group
	fnDagNode.setObject( path.node() );
	if ( path.hasFn( MFn::kPfxGeometry ) )
	{
		if ( type == MRT_PfxTube ) rmanSGPlug = fnDagNode.findPlug( MString( "liquidTubeShadingGroup" ), &status );
		if ( type == MRT_PfxLeaf ) rmanSGPlug = fnDagNode.findPlug( MString( "liquidLeafShadingGroup" ), &status );
		if ( type == MRT_PfxPetal ) rmanSGPlug = fnDagNode.findPlug( MString( "liquidPetalShadingGroup" ), &status );
		if ( status == MS::kSuccess && rmanSGPlug.isConnected() )
		{
			MPlugArray rmSGArray;
			rmanSGPlug.connectedTo( rmSGArray, true, true );
			MObject rmSGObj( rmSGArray[0].node() );
			MFnDependencyNode SGDepNode( rmSGObj );
			if ( SGDepNode.typeName() == "shadingEngine" ) return rmSGObj;
		}
	}
	MSelectionList objects;
	objects.add( path );
	MObjectArray setArray;

	// Get all of the sets that this object belongs to
	//
	MGlobal::getAssociatedSets( objects, setArray );
	MObject mobj;

	// Look for a set that is a "shading group"
	//
	for ( unsigned i( 0 ); i<setArray.length(); i++ )
	{
		mobj = setArray[i];
		MFnSet fnSet( mobj );
		MStatus stat;
		if ( MFnSet::kRenderableOnly == fnSet.restriction(&stat) ) return mobj;
	}
	return MObject::kNullObj;
}
/**
 * Find the shading node for the given shading group.
 */
MObject liqRibNode::findShader()
{
  LIQDEBUGPRINTF( "-> finding shader for rib node shading group\n");
  // MFnDependencyNode fnNode( group );
  // MPlug shaderPlug = fnNode.findPlug( "surfaceShader" );

  MStatus status;
  MFnDagNode fnDagNode( path() );
  MPlug shaderPlug = fnDagNode.findPlug( MString( "liquidSurfaceShaderNode" ), &status );
  if ( ( status != MS::kSuccess ) || ( !shaderPlug.isConnected() ) ) { status.clear(); shaderPlug = assignedShadingGroup.findPlug( MString( "liquidSurfaceShaderNode" ), &status ); }
  if ( ( status != MS::kSuccess ) || ( !shaderPlug.isConnected() ) ) { status.clear(); shaderPlug = assignedShadingGroup.findPlug( MString( "surfaceShader" ), &status ); }
  if ( !shaderPlug.isNull() ) 
	{
    MPlugArray connectedPlugs;
    bool asSrc = false;
    bool asDst = true;
    shaderPlug.connectedTo( connectedPlugs, asDst, asSrc );
    if ( connectedPlugs.length() != 1 ) 
    {
      //cerr << "Error getting shader\n";
    }
    else
      return connectedPlugs[0].node();
  }
  return MObject::kNullObj;
}
/**
 * Find the displacement node for the given shading group
 */
MObject liqRibNode::findDisp()
{
  LIQDEBUGPRINTF( "-> finding shader for rib node shading group\n");
  // MFnDependencyNode fnNode( group );
  // MPlug shaderPlug = fnNode.findPlug( "displacementShader" );

  MStatus status;
  MFnDagNode fnDagNode( path() );
  MPlug shaderPlug = fnDagNode.findPlug( MString( "liquidDispShaderNode" ), &status );
  if ( ( status != MS::kSuccess ) || ( !shaderPlug.isConnected() ) ) { status.clear(); shaderPlug = assignedShadingGroup.findPlug( MString( "liquidDispShaderNode" ), &status ); }
  if ( ( status != MS::kSuccess ) || ( !shaderPlug.isConnected() ) ) { status.clear(); shaderPlug = assignedShadingGroup.findPlug( MString( "displacementShader" ), &status ); }
  if ( !shaderPlug.isNull() ) 
	{
    MPlugArray connectedPlugs;
    bool asSrc = false;
    bool asDst = true;
    shaderPlug.connectedTo( connectedPlugs, asDst, asSrc );
    if ( connectedPlugs.length() != 1 ) 
    {
      //cerr << "Error getting shader\n";
    }
    else
      return connectedPlugs[0].node();
  }
  return MObject::kNullObj;
}
/**
 * Find the volume shading node for the given shading group.
 */
MObject liqRibNode::findVolume()
{
  LIQDEBUGPRINTF( "-> finding shader for rib node shading group\n");
  // MFnDependencyNode fnNode( group );
  // MPlug shaderPlug = fnNode.findPlug( "volumeShader" );

  MStatus status;
  MFnDagNode fnDagNode( path() );
  MPlug shaderPlug = fnDagNode.findPlug( MString( "liquidVolumeShaderNode" ), &status );
  if ( ( status != MS::kSuccess ) || ( !shaderPlug.isConnected() ) ) { status.clear(); shaderPlug = assignedShadingGroup.findPlug( MString( "liquidVolumeShaderNode" ), &status ); }
  if ( ( status != MS::kSuccess ) || ( !shaderPlug.isConnected() ) ) { status.clear(); shaderPlug = assignedShadingGroup.findPlug( MString( "volumeShader" ), &status ); }
  if ( !shaderPlug.isNull() ) {
    MPlugArray connectedPlugs;
    bool asSrc = false;
    bool asDst = true;
    shaderPlug.connectedTo( connectedPlugs, asDst, asSrc );
    if ( connectedPlugs.length() != 1 ) 
    {
      //cerr << "Error getting shader\n";
    }
    else
      return connectedPlugs[0].node();
  }
  return MObject::kNullObj;
}

// Get the list of all included/excluded lights for this node
// using liquid light sets
void liqRibNode::getSetLights( MObjectArray& linkLights )
{
	MStatus status;

	// get associated sets
	MSelectionList hierarchy;
	MDagPath dagSearcher( DagPath );
	do
	{ 
		dagSearcher.pop();
		hierarchy.add( dagSearcher, MObject::kNullObj, true );
	}
	while ( dagSearcher.length() > 0 );
	MObjectArray oaSets;
	MGlobal::getAssociatedSets( hierarchy, oaSets );

	// get linked lights
	MObjectArray oaLights;
	for ( unsigned i( 0 ); i < oaSets.length(); i++ )
	{
		MFnDependencyNode depNodeFn( oaSets[ i ] );
		MPlug lightLinkPlug = depNodeFn.findPlug( "liqLinkedLights", &status );
		if ( status == MS::kSuccess )
		{
			int numElements = lightLinkPlug.evaluateNumElements();
			for ( int k( 0 ); k < numElements; k++ )
			{
				MPlug elementPlug  = lightLinkPlug.elementByPhysicalIndex( k );
				MPlugArray connectedPlugs;
				if ( !elementPlug.isConnected() ) continue;
				elementPlug.connectedTo( connectedPlugs, true, false, &status );
				oaLights.append( connectedPlugs[0].node() );
			}
		}
	}
	// remove duplicate lights
	MSelectionList selList;
	for ( unsigned i( 0 ); i < oaLights.length(); i++ ) selList.add( oaLights[i], 1 );

	for ( unsigned i( 0 ); i < selList.length(); i++ )
	{
		MObject node;
		selList.getDependNode( i, node );
		linkLights.append( node );
	}
}

// Get the list of all included/excluded lights for this node
void liqRibNode::getLinkLights( MObjectArray& linkLights, bool exclusive )
{
	MStatus status;
	LIQDEBUGPRINTF( "-> getting linked lights\n");
	MFnDependencyNode fnNode( DagPath.node() );
	MPlug msgPlug = fnNode.findPlug( "message", &status );
	if ( status != MS::kSuccess ) return;
	
	MPlugArray llPlugs;
	msgPlug.connectedTo(llPlugs, true, true);

	for ( unsigned i = 0; i < llPlugs.length() ; i++ )
	{
		MPlug llPlug = llPlugs[i];
		MObject llPlugAttr = llPlug.attribute();
		MFnAttribute llPlugAttrFn(llPlugAttr);

		if ( llPlugAttrFn.name() == MString( "objectIgnored" ) && exclusive )
		{
			MPlug llParentPlug = llPlug.parent(&status);
			int numChildren  = llParentPlug.numChildren();

			for ( int k = 0; k < numChildren ; k++ )
			{
				MPlug   childPlug  = llParentPlug.child(k);
				MObject llChildAttr = childPlug.attribute();
				MFnAttribute llChildAttrFn(llChildAttr);

				if ( llChildAttrFn.name() == MString( "lightIgnored" ) )
				{
					MPlugArray connectedPlugs;
					childPlug.connectedTo(connectedPlugs,true,true);
					MFnDependencyNode conP( connectedPlugs[0].node() );
					if ( connectedPlugs[0].node().hasFn( MFn::kSet ) )
					{
						MStatus setStatus;
						MFnDependencyNode listSetNode( connectedPlugs[0].node() );
						MPlug setPlug = fnNode.findPlug( "dagSetMembers", &setStatus );
						if ( setStatus == MS::kSuccess )
						{
							MPlugArray setConnectedPlugs;
							setPlug.connectedTo(setConnectedPlugs,true,true);
							linkLights.append( setConnectedPlugs[0].node() );
						}
					}
					else
						linkLights.append( connectedPlugs[0].node() );
				}
			}
		}
		if ( llPlugAttrFn.name() == MString( "object" ) && !exclusive )
		{
			MPlug llParentPlug = llPlug.parent(&status);
			int numChildren  = llParentPlug.numChildren();

			for ( int k = 0; k < numChildren; k++ )
			{
				MPlug   childPlug  = llParentPlug.child(k);
				MObject llChildAttr = childPlug.attribute();
				MFnAttribute llChildAttrFn(llChildAttr);

				if ( llChildAttrFn.name() == MString( "light" ) )
				{
					MPlugArray connectedPlugs;
					childPlug.connectedTo(connectedPlugs,true,true);
					MFnDependencyNode conP( connectedPlugs[0].node() );
	//				MGlobal::displayInfo( MString( "connectedPlugs: " ) + conP.name() );
					if ( connectedPlugs[0].node().hasFn( MFn::kSet ) )
					{
						MStatus setStatus;
						MFnDependencyNode listSetNode( connectedPlugs[0].node() );
						MPlug setPlug = fnNode.findPlug( "dagSetMembers", &setStatus );
						if ( setStatus == MS::kSuccess )
						{
							MPlugArray setConnectedPlugs;
							setPlug.connectedTo(setConnectedPlugs,true,true);
							linkLights.append( setConnectedPlugs[0].node() );
						}
					}
					else
						linkLights.append( connectedPlugs[0].node() );
				}
			}
		}
	}
	// so that liquid works with illuminate by default
}
/**
 * Get the color & opacity of the given shading node.
 */
bool liqRibNode::getColor( MObject& shader, MColor& color )
{
  LIQDEBUGPRINTF( "-> getting a shader color\n");
  MStatus stat = MS::kSuccess;
  switch ( shader.apiType() )
  {
    case MFn::kLambert :
    {
      MFnLambertShader fnShader( shader );
      color = fnShader.color();
      break;
    }
    case MFn::kBlinn :
    {
      MFnBlinnShader fnShader( shader );
      color = fnShader.color();
      break;
    }
    case MFn::kPhong :
    {
      MFnPhongShader fnShader( shader );
      color = fnShader.color();
      break;
    }
    default:
    {
      MFnDependencyNode fnNode( shader );
      MPlug colorPlug = fnNode.findPlug( "outColor" );
      MPlug tmpPlug;
      tmpPlug = colorPlug.child(0,&stat);
      if ( stat == MS::kSuccess ) tmpPlug.getValue( color.r );
      tmpPlug = colorPlug.child(1,&stat);
      if ( stat == MS::kSuccess ) tmpPlug.getValue( color.g );
      tmpPlug = colorPlug.child(2,&stat);
      if ( stat == MS::kSuccess ) tmpPlug.getValue( color.b );
        return false;
    }
  }
  return true;
}

bool liqRibNode::getOpacity( MObject& shader, MColor& opacity )
{
  LIQDEBUGPRINTF( "-> getting a shader opacity\n");
  MStatus stat = MS::kSuccess;
  switch ( shader.apiType() )
  {
    case MFn::kLambert :
    {
      MFnLambertShader fnShader( shader );
      opacity = fnShader.transparency();
      opacity.r = 1. - opacity.r;
      opacity.g = 1. - opacity.g;
      opacity.b = 1. - opacity.b;
      break;
    }
    case MFn::kBlinn :
    {
      MFnBlinnShader fnShader( shader );
      opacity = fnShader.transparency();
      opacity.r = 1. - opacity.r;
      opacity.g = 1. - opacity.g;
      opacity.b = 1. - opacity.b;
      break;
    }
    case MFn::kPhong :
    {
      MFnPhongShader fnShader( shader );
      opacity = fnShader.transparency();
      opacity.r = 1. - opacity.r;
      opacity.g = 1. - opacity.g;
      opacity.b = 1. - opacity.b;
      break;
    }
    default:
    {
      MFnDependencyNode fnNode( shader );
      MPlug colorPlug = fnNode.findPlug( "outTransparency" );
      MPlug tmpPlug;
      tmpPlug = colorPlug.child(0,&stat);
      if ( stat == MS::kSuccess ) tmpPlug.getValue( opacity.r );
      tmpPlug = colorPlug.child(1,&stat);
      if ( stat == MS::kSuccess ) tmpPlug.getValue( opacity.g );
      tmpPlug = colorPlug.child(2,&stat);
      if ( stat == MS::kSuccess ) tmpPlug.getValue( opacity.b );
      opacity.r = 1. - opacity.r;
      opacity.g = 1. - opacity.g;
      opacity.b = 1. - opacity.b;
      return false;
    }
  }
  return true;
}
/**
 * Check to see if this is a matte object.
 * if a regular maya shader with a matteOpacityMode attribute is attached,
 * and the value of the attribute is 0 ( Black Hole ) then we return true.
 */
bool liqRibNode::getMatteMode( MObject& shader )
{
  MObject matteModeObj;
  short matteModeInt;
  MStatus myStatus;
  LIQDEBUGPRINTF( "-> getting matte mode\n");
  if ( !shader.isNull() ) 
  {
    MFnDependencyNode fnNode( shader );
    MPlug mattePlug = fnNode.findPlug( "matteOpacityMode", &myStatus );
    if ( myStatus == MS::kSuccess ) 
		{
      mattePlug.getValue( matteModeInt );
      LIQDEBUGPRINTF(  "-> matte mode: %d \n", matteModeInt );
      if ( matteModeInt == 0 ) return true;
    }
  }
  return false;
}
/**
 * Checks if this node has at least n objects.
 */
bool liqRibNode::hasNObjects( unsigned n )
{
  for ( int i = 0; i < n; i++ ) 
    if ( !objects[ i ] ) return false;
  
  return true;
}

void liqRibNode::parseVectorAttributes( const MFnDependencyNode& nodeFn, 
																				const MStringArray& strArray, const ParameterType& pType ) 
{
  MStatus status;
  if ( strArray.length() > 0 ) 
  {
    for ( unsigned i( 0 ); i < strArray.length(); i++ ) 
    {
      liqTokenPointer tokenPointerPair;
      MString cutString( strArray[i].substring( 5, strArray[i].length() ) );
      MPlug vPlug( nodeFn.findPlug( strArray[i] ) );
      MObject plugObj;
      status = vPlug.getValue( plugObj );
      if ( plugObj.apiType() == MFn::kVectorArrayData ) 
      {
        MFnVectorArrayData  fnVectorArrayData( plugObj );
        MVectorArray vectorArrayData = fnVectorArrayData.array( &status );
        tokenPointerPair.set( cutString.asChar(), pType, vectorArrayData.length() );
        for ( unsigned kk( 0 ); kk < vectorArrayData.length(); kk++ ) 
          tokenPointerPair.setTokenFloat( kk, vectorArrayData[kk].x, vectorArrayData[kk].y, vectorArrayData[kk].z );

        // store it all
        if ( tokenPointerMap.end() == tokenPointerMap.find( tokenPointerPair.getDetailedTokenName() ) ) 
          tokenPointerMap[ tokenPointerPair.getDetailedTokenName() ] = tokenPointerPair;
      } 
      else 
      {
        // Hmmmm float ? double ?
        float x, y, z;
        tokenPointerPair.set( cutString.asChar(), pType );

        vPlug.child(0).getValue( x );
        vPlug.child(1).getValue( y );
        vPlug.child(2).getValue( z );

/*		// temporay solution to convert the rim tint to vector value
		if ( vPlug.partialName() == "rmanCglobalRimTint" )
		{
			vPlug.getValue( x );
			y = x;
			z = x;
			MGlobal::displayInfo( MString ( "rmanCglobalRimTint converted to vector value: " ) + vPlug.partialName() );
		}
*/
		//if ( plugObj.apiType() == MFn::kDoubleArrayData )

        tokenPointerPair.setTokenFloat( 0, x, y, z );
        if ( tokenPointerMap.end() == tokenPointerMap.find( tokenPointerPair.getDetailedTokenName() ) ) 
          tokenPointerMap[ tokenPointerPair.getDetailedTokenName() ] = tokenPointerPair;
      }
    }
  }
}


void liqRibNode::writeUserAttributes() 
{
  unsigned numTokens( tokenPointerMap.size() );
  if ( numTokens ) 
  {
    scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
    scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
    // Can't use assignTokenArraysV() since we're dealing with std::map
    unsigned i( 0 );
    for ( map< string, liqTokenPointer >::const_iterator iter( tokenPointerMap.begin() ); 
					iter != tokenPointerMap.end(); 
					iter++, i++ ) 
    {
      tokenArray[ i ] = const_cast< RtString >( const_cast< liqTokenPointer* >( &( iter->second ) )->getDetailedTokenName().c_str() );
      pointerArray[ i ] = const_cast< liqTokenPointer* >( &( iter->second ) )->getRtPointer();
    }
    RiAttributeV( "user", numTokens, tokenArray.get(), pointerArray.get() );
  }
}
