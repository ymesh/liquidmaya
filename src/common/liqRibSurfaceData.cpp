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
** The RenderMan (R) Interface Procedures and Protocol are: Copyright 1988,
** 1989, Pixar All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
**
*/

/* ______________________________________________________________________
**
** Liquid Rib Surface Data Source
** ______________________________________________________________________
*/


// RenderMan headers
extern "C" {
#include <ri.h>
}

// Maya headers
#include<maya/MPlug.h>
#include<maya/MFnNurbsCurve.h>
#include<maya/MDoubleArray.h>
#include<maya/MItSurfaceCV.h>
#include<maya/MFnNurbsSurface.h>

// Liquid headers
#include <liqGlobalHelpers.h>
#include <liqRibSurfaceData.h>
#include <liqRenderer.h>

// Boost headers
#include <boost/scoped_array.hpp>
#include <boost/shared_array.hpp>

using namespace boost;

extern int debugMode;
extern liqRenderer liquidRenderer;


/** Ccreate a RIB compatible representation of a Maya nurbs surface.
 */
liqRibSurfaceData::liqRibSurfaceData( MObject surface )
:   hasTrims( false ),
    grain( 0 ),
    uknot(),
    vknot(),
    CVs(),
    ncurves(),
    order(),
    numCVs(),
    knot(),
    minKnot(),
    maxKnot(),
    u(),
    v(),
    w()

{
  LIQDEBUGPRINTF( "-> creating nurbs surface\n" );
  if ( debugMode ) {
    MFnDependencyNode myDep( surface );
    MString name = myDep.name();
    LIQDEBUGPRINTF( "-> surface path %s\n", name.asChar() );
  }

  // Hmmmmm Was global but never changed ...
  bool normalizeNurbsUV( true );

  MStatus status( MS::kSuccess );
  MFnNurbsSurface nurbs( surface, &status );
  if ( MS::kSuccess == status ) 
  {
    // Extract the order and number of CVs in the surface

    MDoubleArray uKnots, vKnots;

    if ( liquidRenderer.requires_SWAPPED_UVS ) 
    {
      LIQDEBUGPRINTF( "-> swapping uvs\n" );

      uorder = nurbs.degreeV() + 1; // uv order is switched
      vorder = nurbs.degreeU() + 1; // uv order is switched
      nu = nurbs.numCVsInV();       // uv order is switched
      nv = nurbs.numCVsInU();       // uv order is switched

      // Read the knot information

      nurbs.getKnotsInU( vKnots ); // uv order is switched
      nurbs.getKnotsInV( uKnots ); // uv order is switched
      
      double uMin_d, uMax_d, vMin_d, vMax_d;
      nurbs.getKnotDomain(uMin_d, uMax_d, vMin_d, vMax_d);
      umin = ( RtFloat )vMin_d; // uv order is switched
      umax = ( RtFloat )vMax_d; // uv order is switched
      vmin = ( RtFloat )uMin_d; // uv order is switched
      vmax = ( RtFloat )uMax_d; // uv order is switched
    } 
    else 
    {
      LIQDEBUGPRINTF( "-> not swapping uvs\n" );

      uorder = nurbs.degreeU() + 1;
      vorder = nurbs.degreeV() + 1;
      nu = nurbs.numCVsInU();
      nv = nurbs.numCVsInV();

      // Read the knot information

      nurbs.getKnotsInU( uKnots );
      nurbs.getKnotsInV( vKnots );

      double uMin_d, uMax_d, vMin_d, vMax_d;
      nurbs.getKnotDomain( uMin_d, uMax_d, vMin_d, vMax_d );
      umin = ( RtFloat )uMin_d;
      umax = ( RtFloat )uMax_d;
      vmin = ( RtFloat )vMin_d;
      vmax = ( RtFloat )vMax_d;
      
          	
    	// Reverse v knots values for matching Maya texture coordinates
      for ( unsigned k = 0; k < vKnots.length(); k++ )
    	{	
    		 vKnots[ k ] = vmax - vKnots[ k ] + vmin;
    		 //printf( "vKnots[%d]  = %f\n", k, vKnots[ k ] );
    	}
    	
    }

    float uKnotMult = 1;
    float vKnotMult = 1;

    // this was added to simulate MTOR's parameterization handling
    // it, by default, normalizes the U and V coordinates.

    bool noNormalizeNurbs = false;
    
    liquidGetPlugValue( nurbs, "noNormalizeNurbs", noNormalizeNurbs, status );
    
    normalizeNurbsUV = !noNormalizeNurbs;
    
    if ( normalizeNurbsUV ) 
    {
      uKnotMult = 1 / ( umax - umin );
      vKnotMult = 1 / ( vmax - vmin );
    }

    // Allocate CV and knot storage
    CVs   = shared_array< RtFloat >( new RtFloat[ nu * nv * 4 ] );
    uknot = shared_array< RtFloat >( new RtFloat[ uKnots.length() + 2 ] );
    vknot = shared_array< RtFloat >( new RtFloat[ vKnots.length() + 2 ] );

    unsigned k;
    if ( normalizeNurbsUV ) 
    {
      for ( k = 0; k < uKnots.length(); k++ ) 
        uknot[ k + 1 ] = ( ( RtFloat )uKnots[ k ] - umin ) * uKnotMult;
    } 
    else 
      for ( k = 0; k < uKnots.length(); k++ ) 
        uknot[ k + 1 ] = ( RtFloat )uKnots[ k ];

    // Maya doesn't store the first and last knots, so we double them up
    // manually
    //
    uknot[ 0 ]   = uknot[ 1 ];
    uknot[ k+1 ] = uknot[ k ];

    
    if ( liquidRenderer.requires_SWAPPED_UVS )
    {
			if ( normalizeNurbsUV ) 
				for ( k = 0; k < vKnots.length(); k++ ) vknot[ k + 1 ] = ( ( RtFloat )vKnots[ k ] - vmin ) * vKnotMult;
			else 
				for ( k = 0; k < vKnots.length(); k++ ) vknot[ k + 1 ] = ( RtFloat )vKnots[ k ];
		}
		else
		{
			if ( normalizeNurbsUV ) 
				for ( k = 0; k < vKnots.length(); k++ ) vknot[ k + 1 ] = ( ( RtFloat )vKnots[ vKnots.length() - 1 - k ] - vmin ) * vKnotMult;
			else 
				for ( k = 0; k < vKnots.length(); k++ ) vknot[ k + 1 ] = ( RtFloat )vKnots[ vKnots.length() - 1 - k ];	
		}
    // Maya doesn't store the first and last knots, so we double them up
    // manually
    //
    vknot[ 0 ] = vknot[ 1 ];
    vknot[ k + 1 ] = vknot[ k ];

    // Read CV information
    //
    MItSurfaceCV cvs( surface, liquidRenderer.requires_SWAPPED_UVS == false );
    
    if ( liquidRenderer.requires_SWAPPED_UVS )
    {
			RtFloat* cvPtr( CVs.get() );
    	while ( !cvs.isDone() ) 
			{
				while ( !cvs.isRowDone() ) 
				{
					MPoint pt( cvs.position( MSpace::kObject ) );
					*cvPtr++ = ( RtFloat )(pt.x * pt.w);
					*cvPtr++ = ( RtFloat )(pt.y * pt.w); 
					*cvPtr++ = ( RtFloat )(pt.z * pt.w);
					*cvPtr++ = ( RtFloat )pt.w;
					cvs.next();
				}
				cvs.nextRow();
			}
		}
		else
    {
      // Reverse v knots order for matching Maya texture coordinates	
    	// store rows in reversed order
    	RtFloat* cvPtr( CVs.get() );
    	cvPtr += nu * ( nv - 1 ) * 4; // set pointer to last row in array
    	
    	while( !cvs.isDone() ) 
			{
				while ( !cvs.isRowDone() ) 
				{
					MPoint pt( cvs.position( MSpace::kObject ) );
					*cvPtr++ = ( RtFloat )(pt.x * pt.w);
					*cvPtr++ = ( RtFloat )(pt.y * pt.w); 
					*cvPtr++ = ( RtFloat )(pt.z * pt.w);
					*cvPtr++ = ( RtFloat )pt.w;
					cvs.next();
					// LIQDEBUGPRINTF( "-> %f %f %f %f\n", pt.x, pt.y, pt.z, pt.w  );
				}
				cvs.nextRow();
				cvPtr -= (nu * 4 * 2); // skip two rows
			}
    }
    // Store trim information
    //
    if ( nurbs.isTrimmedSurface() ) 
    {
      hasTrims = true;
      LIQDEBUGPRINTF( "-> storing trim information\n" );
      unsigned numRegions, numBoundaries, numEdges, numCurves;
      // Get the number of loops
      //
      numRegions = nurbs.numRegions();
      nloops = 0;
      for ( unsigned r( 0 ); r < numRegions; r++ ) 
      {
        numBoundaries = nurbs.numBoundaries( r );
        nloops += numBoundaries;
      }
      // Get the number of trim curves in each loop and gather curve
      // information
      //
      for ( unsigned r( 0 ); r < (unsigned) nloops; r++ ) 
      {
        numBoundaries = nurbs.numBoundaries( r );
        for ( unsigned b( 0 ); b < numBoundaries; b++ ) 
        {
          numCurves = 0;
          numEdges = nurbs.numEdges( r, b );
          for ( unsigned e( 0 ); e < numEdges; e++ ) 
          {
            MObjectArray curves( nurbs.edge( r, b, e, true ) );
            numCurves += curves.length();
            // Gather extra stats for each curve
            //
            for ( unsigned c( 0 ); c < curves.length(); c++ ) 
            {
              // Read the # of CVs in and the order of each curve
              //
              MFnNurbsCurve curveFn(curves[ c ]);
              order.push_back( curveFn.degree() + 1 );
              numCVs.push_back( curveFn.numCVs() );

              // Read the CVs for each curve
              //
              MPoint pnt;
              for ( unsigned i( 0 ); i < curveFn.numCVs(); ++i ) 
              {
                curveFn.getCV( i, pnt );
                //cvArray.push_back( pnt );
                if ( liquidRenderer.requires_SWAPPED_UVS ) 
                {
                	if ( normalizeNurbsUV )
                 {	
										u.push_back( ( pnt.y - umin ) * uKnotMult );
										v.push_back( ( pnt.x - vmin ) * vKnotMult );
									} 
									else
									{
										u.push_back( pnt.y );
										v.push_back( pnt.x );	
									}	
                } 
                else 
                {
									if ( normalizeNurbsUV )
                 {		
                 	 u.push_back( ( pnt.x - umin ) * uKnotMult );
                 	 v.push_back( ( vmax - pnt.y - vmin + vmin) * vKnotMult );
                 }
                 else
                 {
                 		u.push_back( pnt.x );
                 		v.push_back( vmax - pnt.y + vmin ); 
                 }
                }
                w.push_back( pnt.w );
              }
              // Read the knot array for each curve
              //
              MDoubleArray knotsTmpArray;
              curveFn.getKnots( knotsTmpArray );
              knot.push_back( knotsTmpArray[ 0 ] );
              for ( unsigned i( 0 ); i < knotsTmpArray.length(); ++i ) knot.push_back( (float)knotsTmpArray[ i ] );
              
              knot.push_back( knotsTmpArray[ knotsTmpArray.length() - 1 ] );

              // Read the knot domain for each curve
              //
              double start, end;
              curveFn.getKnotDomain( start, end );
              minKnot.push_back( start );
              maxKnot.push_back( end );
            }
          }
          ncurves.push_back( numCurves );
        }
      }
    }
    
    if ( normalizeNurbsUV )
    {
			 umin = 0; 
			 umax = 1;
			 vmin = 0; 
			 vmax = 1;
    }

    // now place our tokens and parameters into our tokenlist

    liqTokenPointer tokenPointerPair;
    tokenPointerPair.set( "Pw", rHpoint, nu * nv );
    tokenPointerPair.setDetailType( rVertex );
    tokenPointerPair.setTokenFloats( CVs );
    tokenPointerArray.push_back( tokenPointerPair );

    addAdditionalSurfaceParameters( surface );
  }
}



/** Write the RIB for this surface.
 */
void liqRibSurfaceData::write()
{
  LIQDEBUGPRINTF( "-> writing nurbs surface\n" );

  LIQDEBUGPRINTF( "-> writing nurbs surface trims\n" );
  if ( hasTrims ) 
  {
    RiTrimCurve( nloops,
                 const_cast< RtInt* >( &ncurves[ 0 ] ),
                 const_cast< RtInt* >( &order[ 0 ] ),
                 const_cast< RtFloat* >( &knot[ 0 ] ),
                 const_cast< RtFloat* >( &minKnot[ 0 ] ),
                 const_cast< RtFloat* >( &maxKnot[ 0 ] ),
                 const_cast< RtInt* >( &numCVs[ 0 ] ),
                 const_cast< RtFloat* >( &u[ 0 ] ),
                 const_cast< RtFloat* >( &v[ 0 ] ),
                 const_cast< RtFloat* >( &w[ 0 ] ) );
  }

  if ( !tokenPointerArray.empty() ) 
  {
    unsigned numTokens( tokenPointerArray.size() );
    scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
    scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
    assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

    RiNuPatchV( nu,
                uorder,
                uknot.get(),
                umin,
                umax,
                nv,
                vorder,
                vknot.get(),
                vmin,
                vmax,
                numTokens,
                tokenArray.get(),
                pointerArray.get() );
  } 
  else 
  {
    LIQDEBUGPRINTF( "-> ignoring nurbs surface\n" );
  }
  LIQDEBUGPRINTF( "-> done writing nurbs surface\n" );
}

unsigned liqRibSurfaceData::granularity() const 
{
  if ( hasTrims && !tokenPointerArray.empty() ) return 2;
  else if ( !tokenPointerArray.empty() ) return 1;
  else return 0;
}

bool liqRibSurfaceData::writeNextGrain()
{
  if ( hasTrims && ( 0 == grain ) ) 
  {
    RiTrimCurve( nloops,
                 const_cast< RtInt* >( &ncurves[ 0 ] ),
                 const_cast< RtInt* >( &order[ 0 ] ),
                 const_cast< RtFloat* >( &knot[ 0 ] ),
                 const_cast< RtFloat* >( &minKnot[ 0 ] ),
                 const_cast< RtFloat* >( &maxKnot[ 0 ] ),
                 const_cast< RtInt* >( &numCVs[ 0 ] ),
                 const_cast< RtFloat* >( &u[ 0 ] ),
                 const_cast< RtFloat* >( &v[ 0 ] ),
                 const_cast< RtFloat* >( &w[ 0 ] ) );
    ++grain;
    return true;
  } 
  else if ( !tokenPointerArray.empty() ) 
  {
    unsigned numTokens( tokenPointerArray.size() );
    scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
    scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
    assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

    RiNuPatchV( nu,
                uorder,
                uknot.get(),
                umin,
                umax,
                nv,
                vorder,
                vknot.get(),
                vmin,
                vmax,
                numTokens,
                tokenArray.get(),
                pointerArray.get() );

    grain = 0;
  }
  return false;
}

/** Compare this surface to the other for the purpose of determining
 *  if it is animated.
 */
bool liqRibSurfaceData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF( "-> comparing nurbs surface\n" );
  if ( otherObj.type() != MRT_Nurbs ) return false;

  const liqRibSurfaceData & other = (liqRibSurfaceData&)otherObj;

  if ( ( nu != other.nu ) ||
       ( nv != other.nv ) ||
       ( uorder != other.uorder ) ||
       ( vorder != other.vorder ) ||
       !equiv( umin, other.umin ) ||
       !equiv( umax, other.umax ) ||
       !equiv( vmin, other.vmin ) ||
       !equiv( vmax, other.vmax ) )
  {
    return false;
  }
  // Check Knots
  unsigned i;
  unsigned last( nu + uorder );
  for ( i = 0; i < last; ++i ) 
    if ( !equiv( uknot[ i ], other.uknot[ i ] ) )
      return false;
  last = nv + vorder;
  for ( i = 0; i < last; ++i ) 
    if ( !equiv( vknot[ i ], other.vknot[ i ] ) )
      return false;
  // Check CVs
  last = nu * nv * 4;
  for ( i = 0; i < last; ++i ) 
    if ( !equiv( CVs[ i ], other.CVs[ i ] ) )
      return false;
  // TODO: Check trims as well
  return true;
}

/** Return the geometry type.
 */
ObjectType liqRibSurfaceData::type() const
{
  LIQDEBUGPRINTF( "-> returning nurbs surface type\n" );
  return MRT_Nurbs;
}

