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
** Liquid Rib pfxToon Curve Data Source
** ______________________________________________________________________
*/

// RenderMan jeaders
extern "C" {
#include <ri.h>
}

// Maya headers
#include <maya/MDoubleArray.h>
#include <maya/MItCurveCV.h>
#include <maya/MPoint.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MPlug.h>

#include <maya/MFnPfxGeometry.h>
#include <maya/MRenderLineArray.h>
#include <maya/MRenderLine.h>
#include <maya/MVectorArray.h>
#include <maya/MIntArray.h>
#include <maya/MFloatArray.h>
#include <maya/MColorArray.h>
#include <maya/MDagModifier.h>
#include <maya/MSelectionList.h>
#include <maya/MMatrix.h>

// Liquid headers
#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRibData.h>
#include <liqRibPfxToonData.h>

// Boost headers
#include <boost/scoped_array.hpp>


extern int debugMode;
extern MString liqglo_renderCamera;


/** Create a RIB compatible representation of a Maya pfxToon node as RiCurves.
 */
liqRibPfxToonData::liqRibPfxToonData( MObject pfxToon )
  : nverts(),
    CVs(),
    curveWidth(),
    cvColor(),
    cvOpacity()
{
  LIQDEBUGPRINTF( "-> creating pfxToon curves\n" );
  MStatus status( MS::kSuccess );

  // Update the pfxToon node with the renderCamera's position
  // otherwise the resulting outline might be incorrect
  MDagPath cameraPath;
  MSelectionList camList;
  camList.add( liqglo_renderCamera );
  camList.getDagPath( 0, cameraPath );
  MMatrix cam_mat( cameraPath.inclusiveMatrix() );
  MFnDependencyNode pfxToonNode( pfxToon );
  pfxToonNode.findPlug( "cameraPointX" ).setValue( cam_mat( 3, 0 ) );
  pfxToonNode.findPlug( "cameraPointY" ).setValue( cam_mat( 3, 1 ) );
  pfxToonNode.findPlug( "cameraPointZ" ).setValue( cam_mat( 3, 2 ) );


  MFnPfxGeometry pfxtoon( pfxToon, &status );

  if ( status == MS::kSuccess ) 
	{
    MRenderLineArray profileArray;
    MRenderLineArray creaseArray;
    MRenderLineArray intersectionArray;

    bool doLines          = true;
    bool doTwist          = false;
    bool doWidth          = true;
    bool doFlatness       = false;
    bool doParameter      = false;
    bool doColor          = true;
    bool doIncandescence  = false;
    bool doTransparency   = true;
    bool doWorldSpace     = false;

    status = pfxtoon.getLineData( profileArray, creaseArray, intersectionArray, doLines, doTwist, doWidth, doFlatness, doParameter, doColor, doIncandescence, doTransparency, doWorldSpace );

    if ( status == MS::kSuccess ) 
    {
      // Het the lines and fill the arrays.
      ncurves = profileArray.length();
      {
        MFnDependencyNode pfxNode( pfxToon );
        MString info( "[liquid] pfxToon node " );
        info += pfxNode.name() + " : " + ncurves + " curves.";
        cout << info << endl << flush;
      }

      unsigned totalNumberOfVertices( 0 );

      if ( ncurves > 0 ) 
      {
        nverts = shared_array< RtInt >( new RtInt[ ncurves ] );

        // Calculate storage requirments.
        // This is a lot more efficient than all those reallocs()
        // (or resize()s if we used a vector) that were done before
        // in the main loop below.
        for ( unsigned i( 0 ); i < ncurves; i++ ) 
				{
          MRenderLine theLine( profileArray.renderLine( i, &status ) );
          if ( MS::kSuccess == status ) 
					{
            MVectorArray vertices( theLine.getLine() );
            totalNumberOfVertices += vertices.length();
          }
        }

        // Allocate memory
        CVs = shared_array< RtFloat >( new RtFloat[ totalNumberOfVertices * 3 ] );
        if ( !CVs ) 
				{
          MString err( "liqRibPfxToonData failed to allocate CV memory!" );
          cout << err << endl << flush;
          throw( err );
          return;
        }

        curveWidth = shared_array< RtFloat >( new RtFloat[ totalNumberOfVertices ] );
        if ( !curveWidth ) 
				{
          MString err( "liqRibPfxToonData failed to allocate per vertex width memory!" );
          cout << err << endl << flush;
          throw( err );
          return;
        }

        cvColor = shared_array< RtFloat >( new RtFloat[ totalNumberOfVertices * 3 ] );
        if ( !cvColor ) 
				{
          MString err( "liqRibPfxToonData failed to allocate CV color memory!" );
          cout << err << endl << flush;
          throw(err);
          return;
        }

        cvOpacity = shared_array< RtFloat >( new RtFloat[ totalNumberOfVertices * 3 ] );
        if ( !cvOpacity ) 
				{
          MString err("liqRibPfxToonData failed to allocate CV opacity memory !");
          cout << err << endl << flush;
          throw( err );
          return;
        }

        RtFloat* cvPtr;
        RtFloat* widthPtr;
        RtFloat* colorPtr;
        RtFloat* opacityPtr;

        totalNumberOfVertices = 0;

        for ( unsigned i( 0 ); i < ncurves; i++ ) 
        {
          MRenderLine theLine( profileArray.renderLine( i, &status ) );

          if ( MS::kSuccess == status ) {

            const MVectorArray& vertices(           theLine.getLine() );
            const MDoubleArray& width(              theLine.getWidth() );
            const MVectorArray& vertexColor(        theLine.getColor() );
            const MVectorArray& vertexTransparency( theLine.getTransparency() );

            //cout <<"line "<<i<<" contains "<<vertices.length()<<" vertices."<<endl;
            //cout <<vertexColor<<endl;

            nverts[i] = vertices.length();
            totalNumberOfVertices += vertices.length();

            cvPtr      = CVs.get()        + ( totalNumberOfVertices * 3 - nverts[ i ] * 3 ) ;
            widthPtr   = curveWidth.get() + ( totalNumberOfVertices     - nverts[ i ] ) ;
            colorPtr   = cvColor.get()    + ( totalNumberOfVertices * 3 - nverts[ i ] * 3 ) ;
            opacityPtr = cvOpacity.get()  + ( totalNumberOfVertices * 3 - nverts[ i ] * 3 ) ;

            for ( unsigned vertIndex( 0 ); vertIndex < vertices.length(); vertIndex++ ) 
            {
              *cvPtr++      = ( RtFloat ) vertices[ vertIndex ].x;
              *cvPtr++      = ( RtFloat ) vertices[ vertIndex ].y;
              *cvPtr++      = ( RtFloat ) vertices[ vertIndex ].z;

              *widthPtr++   = ( RtFloat )width[ vertIndex ];

              *colorPtr++   = ( RtFloat )vertexColor[ vertIndex ].x ;
              *colorPtr++   = ( RtFloat )vertexColor[ vertIndex ].y ;
              *colorPtr++   = ( RtFloat )vertexColor[ vertIndex ].z ;

              *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex ].x ) ;
              *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex ].y ) ;
              *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex ].z ) ;
            }
          }
        }

        // Store for output
        liqTokenPointer points_pointerPair;
        if ( !points_pointerPair.set( "P", rPoint, true, false, totalNumberOfVertices ) ) 
				{
          MString err( "liqRibPfxToonData: liqTokenPointer failed to allocate CV memory !" );
          cout << err << endl;
          throw(err);
          return;
        }
        points_pointerPair.setDetailType( rVertex );
        points_pointerPair.setTokenFloats( CVs );
        tokenPointerArray.push_back( points_pointerPair );

        // Store width params
        liqTokenPointer width_pointerPair;
        if ( !width_pointerPair.set( "width", rFloat, true, false, totalNumberOfVertices ) ) 
				{
          MString err("liqRibPfxToonData: liqTokenPointer failed to allocate width memory !");
          cout <<err<<endl;
          throw(err);
          return;
        }
        width_pointerPair.setDetailType( rVarying );
        width_pointerPair.setTokenFloats( curveWidth );
        tokenPointerArray.push_back( width_pointerPair );

        // Store color params
        liqTokenPointer color_pointerPair;
        if ( !color_pointerPair.set( "pfxToon_vtxColor", rColor, true, false, totalNumberOfVertices ) ) 
				{
          MString err("liqRibPfxToonData: liqTokenPointer failed to allocate color memory !");
          cout <<err<<endl;
          throw(err);
          return;
        }
        color_pointerPair.setDetailType( rVertex );
        color_pointerPair.setTokenFloats( cvColor );
        tokenPointerArray.push_back( color_pointerPair );

        // Store opacity params
        liqTokenPointer opacity_pointerPair;
        if ( !opacity_pointerPair.set( "pfxToon_vtxOpacity", rColor, true, false, totalNumberOfVertices ) ) 
				{
          MString err("liqRibPfxToonData: liqTokenPointer failed to allocate opacity memory !");
          cout <<err<<endl<<flush;
          throw(err);
          return;
        }
        opacity_pointerPair.setDetailType( rVertex );
        opacity_pointerPair.setTokenFloats( cvOpacity );
        tokenPointerArray.push_back( opacity_pointerPair );

        addAdditionalSurfaceParameters( pfxToon );

      }
    }
  }
}

/** Write the RIB for this paint effects toon line.
 */
void liqRibPfxToonData::write()
{
  LIQDEBUGPRINTF( "-> writing pfxToon curve\n" );

  if ( 0 < ncurves  ) 
  {
    unsigned numTokens( tokenPointerArray.size() );
    scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
    scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
    assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

    RiCurvesV( "linear", ncurves, nverts.get(), "nonperiodic", numTokens, tokenArray.get(), pointerArray.get() );
  } else 
    RiIdentity(); // Make sure we don't create empty motion blocks
  
}

/** Compare this curve to the other for the purpose of determining
 *  if it is animated.
 */
bool liqRibPfxToonData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF( "-> comparing pfxToon curves\n");
  if ( otherObj.type() != MRT_PfxToon ) return false;
  //const liqRibPfxToonData & other = (liqRibPfxToonData&)otherObj;

  // // Check CVs
  // last = nverts[0] * 3;
  // for ( i = 0; i < last; ++i ) {
  //     if ( !equiv( CVs[i], other.CVs[i] ) ) return false;
  // }

  return true;
}

/** Return the geometry type
 */
ObjectType liqRibPfxToonData::type() const
{
  LIQDEBUGPRINTF( "-> returning pfxToon curve type\n" );
  return MRT_PfxToon;
}


