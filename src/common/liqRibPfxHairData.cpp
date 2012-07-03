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
** Liquid Rib pfxHair Curve Data Source
** ______________________________________________________________________
*/

// Renderman Headers
extern "C" {
#include <ri.h>
}

// Maya's Headers
#include <maya/MDoubleArray.h>
#include <maya/MItCurveCV.h>
#include <maya/MPoint.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MPlug.h>

#include <maya/MFnPfxGeometry.h>
#include <maya/MRenderLineArray.h>
#include <maya/MRenderLine.h>
#include <maya/MVectorArray.h>


#include <liquid.h>
#include <liqGlobalHelpers.h>
#include <liqRibData.h>
#include <liqRibPfxHairData.h>

#include <boost/scoped_array.hpp>

extern int debugMode;


/** Create a RIB compatible representation of a Maya pfxHair node as RiCurves.
 */
liqRibPfxHairData::liqRibPfxHairData( MObject pfxHair )
  : nverts(),
    CVs(),
    normals(),
    curveWidth(),
    cvColor(),
    cvOpacity()
{
  LIQDEBUGPRINTF( "-> creating pfxHair curve\n" );
  MStatus status( MS::kSuccess );

  MFnPfxGeometry pfxhair( pfxHair, &status );

  if ( MS::kSuccess == status  ) 
	{
    MRenderLineArray profileArray;
    MRenderLineArray creaseArray;
    MRenderLineArray intersectionArray;
    MRenderLineArray copy;

    bool doLines          = true;
    bool doTwist          = true;
    bool doWidth          = true;
    bool doFlatness       = false;
    bool doParameter      = false;
    bool doColor          = true;
    bool doIncandescence  = false;
    bool doTransparency   = true;
    bool doWorldSpace     = false;

    status = pfxhair.getLineData( profileArray, creaseArray, intersectionArray, doLines, doTwist, doWidth, doFlatness, doParameter, doColor, doIncandescence, doTransparency, doWorldSpace );

    if ( status == MS::kSuccess ) 
		{
      ncurves = profileArray.length();
      {
        MFnDependencyNode pfxNode( pfxHair );
        MString info( "[liquid] pfxHair node " );
        info += pfxNode.name() + " : " + ncurves + " curves.";
        cout <<  info  <<  endl  <<  flush;
      }
      unsigned totalNumberOfVertices( 0 ), totalNumberOfSpans( 0 );
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
            const MVectorArray& vertex( theLine.getLine() );
            totalNumberOfVertices += vertex.length() + 2;
          }
        }
        // Allocate memory
        CVs = shared_array< RtFloat >( new RtFloat[ totalNumberOfVertices ] );
        if ( !CVs ) 
				{
          MString err( "liqRibPfxHairData failed to allocate CV memory!" );
          cout << err << endl << flush;
          throw( err );
          return;
        }
        normals = shared_array< RtFloat >( new RtFloat[ totalNumberOfVertices * 3 ] );
        if ( !normals ) 
				{
          MString err( "liqRibPfxHairData failed to allocate normal memory!" );
          cout << err << endl << flush;
          throw( err );
          return;
        }

        curveWidth = shared_array< RtFloat >( new RtFloat[ totalNumberOfSpans ] );
        if ( !curveWidth ) 
				{
          MString err( "liqRibPfxHairData failed to allocate per vertex width memory!" );
          cout << err << endl << flush;
          throw( err );
          return;
        }

        cvColor = shared_array< RtFloat >( new RtFloat[ totalNumberOfVertices * 3 ] );
        if ( !cvColor ) 
				{
          MString err( "liqRibPfxHairData failed to allocate CV color memory!" );
          cout << err << endl << flush;
          throw( err );
          return;
        }

        cvOpacity = shared_array< RtFloat >( new RtFloat[ totalNumberOfVertices * 3 ] );
        if ( !cvOpacity ) 
				{
          MString err( "liqRibPfxHairData failed to allocate CV opacity memory !" );
          cout << err << endl << flush;
          throw( err );
          return;
        }

        RtFloat* cvPtr;
        RtFloat* normalPtr;
        RtFloat* widthPtr;
        RtFloat* colorPtr;
        RtFloat* opacityPtr;

        totalNumberOfVertices = 0;
        for ( unsigned i( 0 ); i < ncurves; i++ ) 
        {
          MRenderLine theLine( profileArray.renderLine( i, &status ) );

          if ( MS::kSuccess == status ) 
          {
            const MVectorArray& vertex(             theLine.getLine() );
            const MVectorArray& twist(              theLine.getTwist() );
            const MDoubleArray& width(              theLine.getWidth() );
            const MVectorArray& vertexColor(        theLine.getColor() );
            const MVectorArray& vertexTransparency( theLine.getTransparency() );

            nverts[ i ] = vertex.length() + 2;
            totalNumberOfVertices += nverts[ i ];
            totalNumberOfSpans += vertex.length();
            unsigned int vertIndex = 0;


            cvPtr      = CVs.get()        + ( totalNumberOfVertices * 3 - nverts[ i ] * 3 );
            normalPtr  = normals.get()    + ( totalNumberOfVertices * 3 - nverts[ i ] * 3 );
            widthPtr   = curveWidth.get() + ( totalNumberOfSpans - vertex.length() );
            colorPtr   = cvColor.get()    + ( totalNumberOfVertices * 3 - nverts[ i ] * 3 );
            opacityPtr = cvOpacity.get()  + ( totalNumberOfVertices * 3 - nverts[ i ] * 3 );

            *cvPtr++      = ( RtFloat )vertex[ vertIndex ].x;
            *cvPtr++      = ( RtFloat )vertex[ vertIndex ].y;
            *cvPtr++      = ( RtFloat )vertex[ vertIndex ].z;

            *colorPtr++   = ( RtFloat )vertexColor[ vertIndex ].x;
            *colorPtr++   = ( RtFloat )vertexColor[ vertIndex ].y;
            *colorPtr++   = ( RtFloat )vertexColor[ vertIndex ].z;

            *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex ].x );
            *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex ].y );
            *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex ].z );

            for ( ; vertIndex < vertex.length(); vertIndex++ ) 
            {
              *cvPtr++      = ( RtFloat )vertex[ vertIndex ].x;
              *cvPtr++      = ( RtFloat )vertex[ vertIndex ].y;
              *cvPtr++      = ( RtFloat )vertex[ vertIndex ].z;

              *normalPtr++  = ( RtFloat )twist[ vertIndex ].x;
              *normalPtr++  = ( RtFloat )twist[ vertIndex ].y;
              *normalPtr++  = ( RtFloat )twist[ vertIndex ].z;

              *widthPtr++   = ( RtFloat )width[ vertIndex ];

              *colorPtr++   = ( RtFloat )vertexColor[ vertIndex ].x;
              *colorPtr++   = ( RtFloat )vertexColor[ vertIndex ].y;
              *colorPtr++   = ( RtFloat )vertexColor[ vertIndex ].z;

              *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex ].x );
              *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex ].y );
              *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex ].z );
            }

            *cvPtr++ = ( float )vertex[ vertIndex - 1 ].x;
            *cvPtr++ = ( float )vertex[ vertIndex - 1 ].y;
            *cvPtr++ = ( float )vertex[ vertIndex - 1 ].z;

            *colorPtr++ = ( RtFloat )vertexColor[ vertIndex - 1 ].x;
            *colorPtr++ = ( RtFloat )vertexColor[ vertIndex - 1 ].y;
            *colorPtr++ = ( RtFloat )vertexColor[ vertIndex - 1 ].z;

            *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex-1 ].x );
            *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex-1 ].y );
            *opacityPtr++ = ( RtFloat )( 1.0f - vertexTransparency[ vertIndex-1 ].z );
          }
        }

        // Store for CVs
        liqTokenPointer points_pointerPair;
        points_pointerPair.set( "P", rPoint, true, false, totalNumberOfVertices );
        points_pointerPair.setDetailType( rVertex );
        points_pointerPair.setTokenFloats( CVs );
        tokenPointerArray.push_back( points_pointerPair );

         // Store normals
        liqTokenPointer normals_pointerPair;
        normals_pointerPair.set( "N", rNormal, true, false, totalNumberOfVertices );
        points_pointerPair.setDetailType( rVertex );
        points_pointerPair.setTokenFloats( normals );
        tokenPointerArray.push_back( normals_pointerPair );

        // Store width params
        liqTokenPointer width_pointerPair;
        width_pointerPair.set( "width", rFloat, true, false, totalNumberOfSpans );
        width_pointerPair.setDetailType( rVarying );
        width_pointerPair.setTokenFloats( curveWidth );
        tokenPointerArray.push_back( width_pointerPair );

        // Store color params
        liqTokenPointer color_pointerPair;
        color_pointerPair.set( "Cs", rColor, true, false, totalNumberOfVertices );
        color_pointerPair.setDetailType( rVertex );
        color_pointerPair.setTokenFloats( cvColor );
        tokenPointerArray.push_back( color_pointerPair );

        // Store opacity params
        liqTokenPointer opacity_pointerPair;
        opacity_pointerPair.set( "Os", rColor, true, false, totalNumberOfVertices );
        opacity_pointerPair.setDetailType( rVertex );
        opacity_pointerPair.setTokenFloats( cvOpacity );
        tokenPointerArray.push_back( opacity_pointerPair );

        // Additional RMan* params
        addAdditionalSurfaceParameters( pfxHair );
      }
    }
  }
}

/** Write the RIB for this surface
 */
void liqRibPfxHairData::write()
{
  LIQDEBUGPRINTF( "-> writing pfxHair curves\n" );

  if ( ncurves > 0 ) 
  {
    unsigned numTokens( tokenPointerArray.size() );
    scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
    scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
    assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

    RiCurvesV( "cubic", ncurves, nverts.get(), "nonperiodic", numTokens, tokenArray.get(), pointerArray.get() );
  } 
  else 
    RiIdentity(); // In case we're in a motion block!
}

/** Compare this curve to the other for the purpose of determining
 * if it is animated.
 */
bool liqRibPfxHairData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF( "-> comparing pfxHair curves\n");
  //cout << "-> comparing pfxHair curves..." << endl;

  if ( otherObj.type() != MRT_PfxHair ) return false;
  const liqRibPfxHairData & other = (liqRibPfxHairData&)otherObj;

  if ( ncurves != other.ncurves ) return false;

  return true;
}

/** Return the geometry type
 */
ObjectType liqRibPfxHairData::type() const
{
  LIQDEBUGPRINTF( "-> returning pfxHair curve type\n" );
  return MRT_PfxHair;
}


