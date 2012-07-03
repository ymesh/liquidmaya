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

/* ______________________________________________________________________
**
** Liquid Rib Particle Data Source
** ______________________________________________________________________
*/

// Renderman Headers
extern "C" {
#include <ri.h>
}

#ifdef _WIN32
#ifdef MSVC6
#  include <map>
#  include <algorithm>
#else
#  include <hash_map>
#  include <algorithm>
#endif
#else
#  include <ext/hash_map>
using namespace __gnu_cxx;
#endif

// Maya's Headers
#include <maya/MFnVectorArrayData.h>
#include <maya/MFnDoubleArrayData.h>
#include <maya/MFnStringArrayData.h>
#include <maya/MFnIntArrayData.h>
#include <maya/MPlug.h>
#include <maya/MVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MAnimControl.h>
#include <maya/MGlobal.h>
#include <maya/MFnParticleSystem.h>
#include <maya/MQuaternion.h>

#include <liquid.h>
#include <liqRibParticleData.h>
#include <liqGlobalHelpers.h>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>

using namespace boost;

extern int debugMode;

extern RtFloat liqglo_sampleTimes[5];
extern liquidlong liqglo_motionSamples;
extern bool liqglo_doDef;
extern bool liqglo_doMotion;
extern structJob liqglo_currentJob;


// these classes are needed to produce a list of particles sorted by their ids
class liq_particleInfo
{
public:
  liq_particleInfo( int num, int id )
    : m_particleNum( num ), m_particleId( id )
    {
      // nothing else needed
    }

  int m_particleNum; // index into the per particle attribute arrays
  int m_particleId; // global particle id
};

typedef boost::shared_ptr< liq_particleInfo > liq_particleInfoPtr;
typedef vector< liq_particleInfoPtr > liq_particleInfoVector ;

class liq_particleIdSort
{
public:
  bool operator()(liq_particleInfoPtr const & a, liq_particleInfoPtr const & b)
    {
      return a.get()->m_particleId < b.get()->m_particleId;
    }
};




/** Create a RIB compatible representation of a Maya particles/
 */
liqRibParticleData::liqRibParticleData( MObject partobj )
  : grain( 0 )
{
  LIQDEBUGPRINTF( "-> creating particles\n");
  MStatus status( MS::kSuccess );
  MFnParticleSystem fnNode;
  fnNode.setObject( partobj );

  MPlug particleRenderTypePlug = fnNode.findPlug( "particleRenderType", &status );
  short particleTypeVal;
  particleRenderTypePlug.getValue( particleTypeVal );
  particleType = ( pType )particleTypeVal;

  LIQDEBUGPRINTF( "-> Reading Particle Count\n");

  // first get the particle position data
  MPlug posPlug( fnNode.findPlug( "position", &status ) );
  MObject  posObject;

  posPlug.getValue( posObject );

  MFnVectorArrayData posArray( posObject, &status );
  status.clear();

  m_numParticles = posArray.length();

  // we need to sort the particles by id. The position array doesn't keep
  // things in particle id order so things can get pretty screwed up when
  // we're motion blurring
  MPlug idPlug;
  MObject idObject;

  liq_particleInfoVector particlesForSorting;;

  // If we're motion blurring, we need to deal with only the particles that existed
  // both at the shutterOpen timepoint and at the shutterClose timepoint. This is
  // actually a bit more complicated than one would think. You cannot ask a particle
  // when it will die by polling its lifespan. The particle cache contains the
  // expected lifespan, but does not take into account that particle lifespans can be
  // overriden (for example, particles can die in collisions). As such, we actually
  // need to go to the shutterOpen and shutterClose timepoints, find the particles
  // common to both, and then use the position for those particles.
  //
  if ( liqglo_doMotion || liqglo_doDef )
  {
    MTime shutterOpen ( (double)liqglo_sampleTimes[0], MTime::uiUnit() );
    MTime shutterClose ( (double)liqglo_sampleTimes[liqglo_motionSamples - 1], MTime::uiUnit() );
    MTime exportTime = MAnimControl::currentTime();


#ifdef _WIN32
	#ifdef MSVC6
	  std::map<int, int> soParticles;
	  std::map<int, int> scParticles;
	#else
	  stdext::hash_map<int, int> soParticles;
	  stdext::hash_map<int, int> scParticles;
	#endif
#else
    hash_map<int, int> soParticles;
    hash_map<int, int> scParticles;
#endif

    bool isCaching;
    MPlug cachePlug( fnNode.findPlug( "cacheData", &status ) );
    cachePlug.getValue( isCaching );
    status.clear();
    if ( !isCaching && ( exportTime == shutterOpen ) ) 
      liquidMessage( string( fnNode.particleName().asChar() ) + " has Cache Data switched off! Exported motion blur information will likely be wrong.", messageWarning );
    
    // Shutter open
    //
    {
      if ( exportTime != shutterOpen ) MGlobal::viewFrame( shutterOpen );
      
      fnNode.setObject(partobj);
      idPlug = fnNode.findPlug( "id", &status );
      idPlug.getValue( idObject );
      const MFnDoubleArrayData idArray( idObject, &status );
      for ( unsigned i( 0 ); i < idArray.length(); i++ ) soParticles[ ( int )idArray[ i ] ] = 1;
    }
    // Shutter close
    //
    {
      MGlobal::viewFrame( shutterClose );
      fnNode.setObject(partobj);
      idPlug = fnNode.findPlug( "id", &status );
      idPlug.getValue( idObject );
      const MFnDoubleArrayData idArray( idObject, &status );
      for ( unsigned i( 0 ); i < idArray.length(); i++ ) scParticles[ ( int )idArray[ i ] ] = 1;
    }
    // Export time
    //
    {
      if ( exportTime != shutterClose ) MGlobal::viewFrame( exportTime );
      
      fnNode.setObject(partobj);
      idPlug = fnNode.findPlug( "id", &status );
      idPlug.getValue( idObject );
      const MFnDoubleArrayData idArray( idObject, &status );
      for ( unsigned i( 0 ); i < idArray.length(); i++ ) 
      {
        if ( soParticles[ ( int )idArray[ i ] ] == 1 &&
             scParticles[ ( int )idArray[ i ] ] == 1 ) 
        {
          particlesForSorting.push_back( liq_particleInfoPtr( new liq_particleInfo( i, ( int )idArray[ i ] ) ) );
        }
      }
    }
  }
  else
  {
    fnNode.setObject(partobj);
    idPlug = fnNode.findPlug( "id", &status );
    idPlug.getValue( idObject );
    const MFnDoubleArrayData idArray( idObject, &status );
    for ( unsigned i( 0 ); i < idArray.length(); i++ ) 
      particlesForSorting.push_back( liq_particleInfoPtr( new liq_particleInfo( i, ( int )idArray[ i ] ) ) );
  }

  status.clear();

  // sort in increasing id order
  sort( particlesForSorting.begin(),
        particlesForSorting.end(),
        liq_particleIdSort());

  // lastly.. we get all of the particle numbers into our m_validParticles
  // array for use later (we don't need the ids any more)
  for ( unsigned i( 0 ); i < particlesForSorting.size(); ++i ) 
    m_validParticles.append( particlesForSorting[ i ]->m_particleNum );

  m_numValidParticles = m_validParticles.length();

  // Check for a multi-count parameter (set if using multi-point
  // or multi-streak particles). Default to 1, otherwise.
  //
  MPlug multiCountPlug( fnNode.findPlug( "multiCount", &status ) );
  if ( MS::kSuccess == status  &&
     (particleType == MPTMultiPoint || particleType == MPTMultiStreak ) )
  {
    multiCountPlug.getValue( m_multiCount );
  }
  else
  {
    m_multiCount = 1;
  }

  // Check for a multi-count radius parameter (again, only for
  // multi-point or multi-streak particles).
  //
  float multiRadius = 0;
  MPlug multiRadiusPlug( fnNode.findPlug( "multiRadius", &status ) );
  if ( MS::kSuccess == status  &&
     (particleType == MPTMultiPoint || particleType == MPTMultiStreak ) )
  {
    multiRadiusPlug.getValue( multiRadius );
  }

  // Get the velocity information (used for streak, multi-streak).
  //
  MPlug velPlug( fnNode.findPlug( "velocity", &status ) );
  MObject velObject;
  velPlug.getValue( velObject );
  MFnVectorArrayData velArray( velObject, &status );

  // Check for the tail size parameter (only for streak, multi-streak).
  //
  float tailSize = 0;
  MPlug tailSizePlug( fnNode.findPlug( "tailSize", &status ) );
  if ( MS::kSuccess == status  &&
     (particleType == MPTStreak || particleType == MPTMultiStreak ) )
  {
    tailSizePlug.getValue( tailSize );
  }

  // Check for the tail fade parameter (only for streak, multi-streak).
  //
  float tailFade = 1;
  MPlug tailFadePlug( fnNode.findPlug( "tailFade", &status ) );
  if ( MS::kSuccess == status  &&
     (particleType == MPTStreak || particleType == MPTMultiStreak ) )
  {
    tailFadePlug.getValue( tailFade );
  }

  // then we get the particle radius data
  LIQDEBUGPRINTF( "-> Reading Particle Radius\n");

  MDoubleArray radiusArray;
  MPlug radiusPPPlug( fnNode.findPlug( "radiusPP", &status ) );
  bool haveRadiusArray = false;
  float radius = 1.0;

  // check if there's a per-particle radius attribute
  if ( MS::kSuccess == status ) 
  {
    MObject radiusPPObject;

    radiusPPPlug.getValue( radiusPPObject );
    MFnDoubleArrayData radiusArrayData( radiusPPObject, &status );
    radiusArray = radiusArrayData.array();
    haveRadiusArray = true;
  } 
  else 
  {
    // no per-particle radius. Try for a global radius
    MPlug radiusPlug( fnNode.findPlug( "radius", &status ) );

    if ( MS::kSuccess == status ) radiusPlug.getValue( radius );
    else 
    {
      // no radius attribute.. try pointSize
      MPlug pointSizePlug = fnNode.findPlug( "pointSize", &status );

      if ( MS::kSuccess == status ) pointSizePlug.getValue( radius );
      else
      {
        // Try lineWidth (used by streak and multi-streak).
        //
        MPlug lineWidthPlug = fnNode.findPlug( "lineWidth", &status );
        if ( MS::kSuccess == status ) lineWidthPlug.getValue( radius );
        
      }
    }
  }
  status.clear();

  MVectorArray rotationArray;
  MPlug rotationPPPlug( fnNode.findPlug( "rotationPP", &status ) );
  bool haveRotationArray = false;
  // check if there's a per-particle radius attribute
  if ( MS::kSuccess == status ) 
	{
    MObject rotationPPObject;
    rotationPPPlug.getValue( rotationPPObject );
    MFnVectorArrayData rotationArrayData( rotationPPObject, &status );
    rotationArray = rotationArrayData.array();
    haveRotationArray = true;
  }
  status.clear();

  // then we get the particle color info
  LIQDEBUGPRINTF( "-> Reading Particle Color\n");

  MVectorArray rgbArray;
  MPlug rgbPPPlug( fnNode.findPlug( "rgbPP", &status ) );
  bool haveRgbArray( false );

  if ( MS::kSuccess == status ) 
  {
    MObject rgbPPObject;

    rgbPPPlug.getValue( rgbPPObject );
    MFnVectorArrayData rgbArrayData( rgbPPObject, &status );
    rgbArray = rgbArrayData.array();
    haveRgbArray = true;
  }
  status.clear();
  // Then we get the per-particle opacity info
  //
  LIQDEBUGPRINTF( "-> Reading Particle Opacity\n");

  MDoubleArray opacityArray;
  MPlug opacityPPPlug( fnNode.findPlug( "opacityPP", &status ) );
  bool haveOpacityArray( false );

  if ( MS::kSuccess == status ) 
  {
    MObject opacityPPObject;
    opacityPPPlug.getValue( opacityPPObject );
    MFnDoubleArrayData opacityArrayData( opacityPPObject, &status );

    opacityArray = opacityArrayData.array();
    haveOpacityArray = true;
  }

  status.clear();
  // and then we do any particle type specific work
  switch ( particleType ) 
  {
    case MPTBlobbies: 
    {
      LIQDEBUGPRINTF( "-> Reading Blobby Particles\n");

      // setup the arrays to store the data to pass the correct codes to the
      // implicit surface command
      m_codeArray.clear();
      m_floatArray.clear();
      m_stringArray.clear();

      LIQDEBUGPRINTF( "-> Reading Particle Data\n");

      RtInt floatOn( 0 );

      for ( unsigned part_num( 0 ); part_num < m_numValidParticles; part_num++ ) 
      {

        // add the particle to the list
        m_codeArray.push_back( 1001 );
        m_codeArray.push_back( floatOn );

        if ( haveRadiusArray )
          radius = ( float )radiusArray[ m_validParticles[ part_num ] ];
        // else radius was set to either a scalar attribute or 1.0
        // above

        m_floatArray.push_back( radius * 2.0f );
        m_floatArray.push_back( 0.0f );
        m_floatArray.push_back( 0.0f );
        m_floatArray.push_back( 0.0f );

        m_floatArray.push_back( 0.0f );
        m_floatArray.push_back( radius * 2.0f );
        m_floatArray.push_back( 0.0 );
        m_floatArray.push_back( 0.0 );

        m_floatArray.push_back( 0.0f );
        m_floatArray.push_back( 0.0f );
        m_floatArray.push_back( radius * 2.0f );
        m_floatArray.push_back( 0.0f );

        m_floatArray.push_back( posArray[ m_validParticles[ part_num ] ].x );
        m_floatArray.push_back( posArray[ m_validParticles[ part_num ] ].y );
        m_floatArray.push_back( posArray[ m_validParticles[ part_num ] ].z );
        m_floatArray.push_back( 1.0f );

        floatOn += 16;
      }

      if ( m_numValidParticles > 0 ) 
      {
        m_codeArray.push_back( 0 );
        m_codeArray.push_back( m_numValidParticles );

        for ( unsigned k( 0 ); k < m_numValidParticles; k++ ) m_codeArray.push_back( k );
      }
      LIQDEBUGPRINTF( "-> Setting up implicit data\n");
      m_stringArray.push_back( "" );
    }
    break;

    // 3Delight supports rendering RiPoints as spheres

    case MPTSpheres:
    {

#ifdef DELIGHT

      liqTokenPointer typeParameter;
      typeParameter.set( "type", rString );
      typeParameter.setTokenString( 0, "sphere" );
      tokenPointerArray.push_back( typeParameter );

#else // Write real spheres
      liqTokenPointer Pparameter;
      liqTokenPointer radiusParameter;

      Pparameter.set( "P", rPoint, true, false, m_numValidParticles );
      Pparameter.setDetailType( rVertex );

      radiusParameter.set( "radius", rFloat, true, false, m_numValidParticles );
      radiusParameter.setDetailType( rVertex );

      for ( unsigned part_num( 0 ); part_num < m_numValidParticles; part_num++ ) 
      {
        Pparameter.setTokenFloat( part_num,
                                  posArray[m_validParticles[part_num]].x,
                                  posArray[m_validParticles[part_num]].y,
                                  posArray[m_validParticles[part_num]].z );
        if ( haveRadiusArray ) 
          radiusParameter.setTokenFloat( part_num, radiusArray[ m_validParticles[ part_num ] ] );
        else
          radiusParameter.setTokenFloat(part_num, radius);
      }
      tokenPointerArray.push_back( Pparameter );
      tokenPointerArray.push_back( radiusParameter );
      break;
  #endif // #ifdef DELIGHT

    }

    case MPTMultiPoint:
    case MPTPoints:
    {
      liqTokenPointer Pparameter;
      Pparameter.set( "P", rPoint, true, false, m_numValidParticles*m_multiCount );
      Pparameter.setDetailType( rVertex );

      for ( unsigned part_num( 0 ); part_num < m_numValidParticles; part_num++ ) 
      {
        // Seed the random number generator using the particle ID
        // (this ensures that the multi-points won't jump during animations,
        //  and won't jump when other particles die)
        //
        srand( particlesForSorting[ part_num ]->m_particleId );
        for ( unsigned multiNum( 0 ); multiNum < m_multiCount; multiNum++ )
        {
          float xDir( 0 ), yDir( 0 ), zDir( 0 ), vLen( 0 ), rad( 0 );

          // Only do the random placement if we're dealing with
          // multi-point or multi-streak particles.
          //
          if ( m_multiCount > 1 ) 
          {
            do 
            {
              xDir = rand() / ( float )RAND_MAX - 0.5f;
              yDir = rand() / ( float )RAND_MAX - 0.5f;
              zDir = rand() / ( float )RAND_MAX - 0.5f;
              vLen = sqrt( pow( xDir, 2 ) + pow( yDir, 2 ) + pow( zDir, 2 ) );
            } while ( vLen == 0.0 );

            xDir /= vLen;
            yDir /= vLen;
            zDir /= vLen;
            rad = rand() / ( float )RAND_MAX * multiRadius / 2.0f;
          }

          Pparameter.setTokenFloat( part_num * m_multiCount + multiNum,
                                    RtFloat(posArray[ m_validParticles[ part_num ] ].x + rad * xDir),
                                    RtFloat(posArray[ m_validParticles[ part_num ] ].y + rad * yDir),
                                    RtFloat(posArray[ m_validParticles[ part_num ] ].z + rad * zDir) );
        }
      }

      tokenPointerArray.push_back( Pparameter );


      // TODO: have we got to do some unit conversion here? what units
      // are the radii in?  What unit is Maya in?
      if ( haveRadiusArray ) 
      {
        liqTokenPointer widthParameter;

        widthParameter.set( "width",
                            rFloat,
                            true,
                            false,
                            m_numValidParticles*m_multiCount );

        widthParameter.setDetailType( rVertex );

        for ( unsigned part_num( 0 ); part_num < m_numValidParticles*m_multiCount; part_num++ ) 
          widthParameter.setTokenFloat( part_num, 
																				radiusArray[ m_validParticles[ part_num / m_multiCount ] ] * 2 );

        tokenPointerArray.push_back( widthParameter );
      } 
      else 
      {
        liqTokenPointer constantwidthParameter;
        constantwidthParameter.set( "constantwidth",
                                    rFloat,
                                    false,
                                    false,
                                    0);
        constantwidthParameter.setDetailType( rConstant );
        constantwidthParameter.setTokenFloat( 0, radius * 2 );

        tokenPointerArray.push_back( constantwidthParameter );
      }
    }
    break;

    case MPTMultiStreak:
    case MPTStreak:
    {
      // Streak particles have a head and a tail, so double the vertex count.
      //
      m_multiCount *= 2;

      liqTokenPointer Pparameter;
      Pparameter.set( "P", rPoint, true, false, m_numValidParticles*m_multiCount );

      Pparameter.setDetailType( rVertex );

      for ( unsigned part_num( 0 ); part_num < m_numValidParticles; part_num++ ) 
      {
        // Seed the random number generator using the particle ID
        // (this ensures that the multi-points won't jump during animations,
        //  and won't jump when other particles die)
        //
        srand( particlesForSorting[ part_num ]->m_particleId );
        for ( unsigned multiNum( 0 ); multiNum < m_multiCount; multiNum += 2 ) 
        {
          float xDir( 0 ), yDir( 0 ), zDir( 0 ), vLen=( 0 ), rad( 0 );

          // Only do the random placement if we're dealing with
          // multi-point or multi-streak particles.
          //
          if ( m_multiCount > 1 ) 
          {
            do 
            {
              xDir = rand() / (float)RAND_MAX - 0.5;
              yDir = rand() / (float)RAND_MAX - 0.5;
              zDir = rand() / (float)RAND_MAX - 0.5;
              vLen = sqrt( pow( xDir, 2 ) + pow( yDir, 2 ) + pow( zDir, 2 ) );
            } while ( vLen == 0.0 );

            xDir /= vLen;
            yDir /= vLen;
            zDir /= vLen;
            rad = rand() / (float) RAND_MAX * multiRadius / 2.0;
          }
          extern double liqglo_FPS;
          // Tail (the formula below is a bit of a guess as to how Maya places the tail).
          //
          Pparameter.setTokenFloat( part_num*m_multiCount + multiNum,
                        posArray[ m_validParticles[ part_num ] ].x + rad * xDir -
                        velArray[ m_validParticles[ part_num ] ].x * tailSize / liqglo_FPS,
                        posArray[ m_validParticles[ part_num ] ].y + rad * yDir -
                        velArray[ m_validParticles[ part_num ] ].y * tailSize / liqglo_FPS,
                        posArray[ m_validParticles[ part_num ] ].z + rad * zDir -
                        velArray[ m_validParticles[ part_num ] ].z * tailSize / liqglo_FPS );

          // Head
          //
          Pparameter.setTokenFloat( part_num * m_multiCount + multiNum + 1,
                        posArray[ m_validParticles[ part_num ] ].x + rad * xDir,
                        posArray[ m_validParticles[ part_num ] ].y + rad * yDir,
                        posArray[ m_validParticles[ part_num ] ].z + rad * zDir );
        }
      }
      tokenPointerArray.push_back( Pparameter );

      // TODO: have we got to do some unit conversion here? what units
      // are the radii in?  What unit is Maya in?
      if (haveRadiusArray) 
      {
        liqTokenPointer widthParameter;

        widthParameter.set( "width",
                            rFloat,
                            true,
                            false,
                            m_numValidParticles * m_multiCount );

        // Since we're specifying the width at both ends of the streak, we must
        // use "varying" instead of "vertex" to describe streak particle width.
        //
        widthParameter.setDetailType( rVarying );

        for ( unsigned part_num( 0 ); part_num < m_numValidParticles * m_multiCount; part_num++ ) 
          widthParameter.setTokenFloat( part_num, 
																				radiusArray[m_validParticles[part_num/m_multiCount]]*2);

        tokenPointerArray.push_back( widthParameter );
      } 
      else 
      {
        liqTokenPointer constantwidthParameter;

        constantwidthParameter.set( "constantwidth",
                                    rFloat,
                                    false,
                                    false,
                                    0);
        constantwidthParameter.setDetailType( rConstant );
        constantwidthParameter.setTokenFloat( 0, radius * 2 );

        tokenPointerArray.push_back( constantwidthParameter );
      }
    }
    break;

    case MPTSprites: {

#ifdef DELIGHT

      liqTokenPointer typeParameter;
      typeParameter.set( "type", rString );
      typeParameter.setTokenString( 0, "patch" );
      tokenPointerArray.push_back( typeParameter );

      liqTokenPointer Pparameter;
      liqTokenPointer spriteNumParameter;
      liqTokenPointer spriteTwistParameter;
      liqTokenPointer spriteWidthParameter;
      liqTokenPointer spriteAspectParameter;

      Pparameter.set( "P", rPoint, m_numValidParticles );
      Pparameter.setDetailType( rVertex );

      spriteNumParameter.set( "spriteNum", rFloat, m_numValidParticles );

      spriteTwistParameter.set( "patchrotation", rFloat,  m_numValidParticles );

      spriteWidthParameter.set( "width", rFloat, m_numValidParticles );

      spriteAspectParameter.set( "patchaspectratio", rFloat, m_numValidParticles );

      bool haveSpriteNums( false );
      bool haveSpriteNumsArray( false );
      MObject spriteNumObject;
      MFnDoubleArrayData spriteNumArray;
      float spriteNum;
      MPlug spriteNumPlug( fnNode.findPlug( "spriteNumPP", &status ) );
      if ( MS::kSuccess == status ) {
        haveSpriteNumsArray = true;
        spriteNumPlug.getValue( spriteNumObject );
        spriteNumArray.setObject( spriteNumObject );
        spriteNumParameter.setDetailType( rVarying );
      } else {
        spriteNumPlug = fnNode.findPlug( "spriteNum", &status );
        if ( MS::kSuccess == status ) {
          haveSpriteNums = true;
          spriteNumPlug.getValue( spriteNum );
          spriteNumParameter.setDetailType( rUniform );
        }
      }

      bool haveSpriteTwist( false );
      bool haveSpriteTwistArray( false );
      MObject spriteTwistObject;
      MFnDoubleArrayData spriteTwistArray;
      float spriteTwist;
      MPlug spriteTwistPlug( fnNode.findPlug( "spriteTwistPP", &status ) );
      if ( MS::kSuccess == status ) {
        haveSpriteTwistArray = true;
        spriteTwistPlug.getValue( spriteTwistObject );
        spriteTwistArray.setObject( spriteTwistObject );
        spriteTwistParameter.setDetailType( rVarying );
      } else {
        spriteTwistPlug = fnNode.findPlug( "spriteTwist", &status );
        if ( MS::kSuccess == status ) {
          haveSpriteTwist = true;
          spriteTwistPlug.getValue( spriteTwist );
          spriteTwistParameter.setDetailType( rUniform );
        }
      }

      bool haveSpriteScaleX( false );
      bool haveSpriteScaleXArray( false );
      MObject spriteScaleXObject;
      MFnDoubleArrayData spriteScaleXArray;
      float spriteScaleX;
      MPlug spriteScaleXPlug( fnNode.findPlug( "spriteScaleXPP", &status ) );
      if ( MS::kSuccess == status ) {
        haveSpriteScaleXArray = true;
        spriteScaleXPlug.getValue( spriteScaleXObject );
        spriteScaleXArray.setObject( spriteScaleXObject );
        spriteWidthParameter.setDetailType( rVarying );
      } else {
        spriteScaleXPlug = fnNode.findPlug( "spriteScaleX", &status );
        if ( MS::kSuccess == status ) {
          haveSpriteScaleX = true;
          spriteScaleXPlug.getValue( spriteScaleX );
          spriteWidthParameter.setDetailType( rUniform );
        }
      }

      bool haveSpriteScaleY( false );
      bool haveSpriteScaleYArray( false );
      MObject spriteScaleYObject;
      MFnDoubleArrayData spriteScaleYArray;
      float spriteScaleY;
      MPlug spriteScaleYPlug( fnNode.findPlug( "spriteScaleYPP", &status ) );
      if ( MS::kSuccess == status ) {
        haveSpriteScaleYArray = true;
        spriteScaleYPlug.getValue( spriteScaleYObject );
        spriteScaleYArray.setObject( spriteScaleYObject );
        spriteAspectParameter.setDetailType( rVarying );
      } else {
        spriteScaleYPlug = fnNode.findPlug( "spriteScaleY", &status );
        if ( MS::kSuccess == status ) {
          haveSpriteScaleY = true;
          spriteScaleYPlug.getValue( spriteScaleY );
          spriteAspectParameter.setDetailType( rUniform );
        }
      }

      for ( unsigned part_num( 0 ); part_num < m_numValidParticles; part_num++ ) {

        Pparameter.setTokenFloat( part_num,
                                  posArray[m_validParticles[part_num]].x,
                                  posArray[m_validParticles[part_num]].y,
                                  posArray[m_validParticles[part_num]].z );
        if ( haveSpriteNumsArray ) {
          spriteNumParameter.setTokenFloat( part_num, spriteNumArray[m_validParticles[part_num]] );
        } else if ( haveSpriteNums ) {
          spriteNumParameter.setTokenFloat( part_num, spriteNum );
        }

        if ( haveSpriteTwistArray ) {
          spriteTwistParameter.setTokenFloat( part_num, -spriteTwistArray[m_validParticles[part_num]] );
        }
        else if ( haveSpriteTwist ) {
          spriteTwistParameter.setTokenFloat( part_num, -spriteTwist);
        }

        if ( haveSpriteScaleXArray ) {
          spriteWidthParameter.setTokenFloat( part_num, spriteScaleXArray[m_validParticles[part_num]] );
        }
        else if ( haveSpriteScaleX ) {
          spriteWidthParameter.setTokenFloat( part_num, spriteScaleX);
        }

        if ( haveSpriteScaleYArray ) {
          if ( haveSpriteScaleXArray ) {
            spriteAspectParameter.setTokenFloat( part_num, spriteScaleXArray[m_validParticles[part_num]] / spriteScaleYArray[m_validParticles[part_num]] );
          } else if ( haveSpriteScaleX ) {
            spriteAspectParameter.setTokenFloat( part_num, spriteScaleX / spriteScaleYArray[m_validParticles[part_num]] );
          } else {
            spriteAspectParameter.setTokenFloat( part_num, 1. / spriteScaleYArray[m_validParticles[part_num]] );
          }
        } else if ( haveSpriteScaleY ) {
          if ( haveSpriteScaleXArray ) {
            spriteAspectParameter.setTokenFloat( part_num, spriteScaleXArray[m_validParticles[part_num]] / spriteScaleY );
          } else if ( haveSpriteScaleX ) {
            spriteAspectParameter.setTokenFloat( part_num, spriteScaleX / spriteScaleY );
          } else {
            spriteAspectParameter.setTokenFloat( part_num, 1. / spriteScaleY );
          }
        }
      }

      tokenPointerArray.push_back( Pparameter );
      if ( haveSpriteNumsArray || haveSpriteNums ) {
        tokenPointerArray.push_back( spriteNumParameter );
      }
      if ( haveSpriteTwistArray || haveSpriteTwist ) {
        tokenPointerArray.push_back( spriteTwistParameter );
      }
      if ( haveSpriteScaleXArray || haveSpriteScaleX ) {
        tokenPointerArray.push_back( spriteWidthParameter );
      }
      if ( haveSpriteScaleYArray || haveSpriteScaleY ) {
        tokenPointerArray.push_back( spriteAspectParameter );
      }

#else // Write real bilinear patches

      liqTokenPointer Pparameter;
      liqTokenPointer spriteNumParameter;
      liqTokenPointer spriteTwistParameter;
      liqTokenPointer spriteScaleXParameter;
      liqTokenPointer spriteScaleYParameter;

      Pparameter.set( "P", rPoint, true, false, m_numValidParticles );
      Pparameter.setDetailType( rVertex );

      spriteNumParameter.set( "spriteNum", rFloat, true, false, m_numValidParticles );
      spriteNumParameter.setDetailType( rUniform );

      spriteTwistParameter.set( "spriteTwist", rFloat, true, false, m_numValidParticles );
      spriteTwistParameter.setDetailType( rUniform );

      spriteScaleXParameter.set( "spriteScaleX", rFloat, true, false, m_numValidParticles );
      spriteScaleXParameter.setDetailType( rUniform );

      spriteScaleYParameter.set( "spriteScaleY", rFloat, true, false, m_numValidParticles );
      spriteScaleYParameter.setDetailType( rUniform );

      bool haveSpriteNums( false );
      bool haveSpriteNumsArray( false );
      MObject spriteNumObject;
      MFnDoubleArrayData spriteNumArray;
      float spriteNum;
      MPlug spriteNumPlug( fnNode.findPlug( "spriteNumPP", &status ) );
      if ( MS::kSuccess == status ) 
      {
        haveSpriteNumsArray = true;
        spriteNumPlug.getValue( spriteNumObject );
        spriteNumArray.setObject( spriteNumObject );
        cout << "DBG> spriteNumPP found length = " << spriteNumArray.length() << endl;
      } 
      else 
      {
        spriteNumPlug = fnNode.findPlug( "spriteNum", &status );
        if ( MS::kSuccess == status ) 
        {
          haveSpriteNums = true;
          spriteNumPlug.getValue( spriteNum );
          cout << "DBG> spriteNum found spriteNum = " << spriteNum << endl;
        }
      }

      bool haveSpriteTwist( false );
      bool haveSpriteTwistArray( false );
      MObject spriteTwistObject;
      MFnDoubleArrayData spriteTwistArray;
      float spriteTwist;
      MPlug spriteTwistPlug( fnNode.findPlug( "spriteTwistPP", &status ) );
      if ( MS::kSuccess == status ) 
      {
        haveSpriteTwistArray = true;
        spriteTwistPlug.getValue( spriteTwistObject );
        spriteTwistArray.setObject( spriteTwistObject );
      } 
      else 
      {
        spriteTwistPlug = fnNode.findPlug( "spriteTwist", &status );
        if ( MS::kSuccess == status ) 
        {
          haveSpriteTwist = true;
          spriteTwistPlug.getValue( spriteTwist );
        }
      }

      bool haveSpriteScaleX( false );
      bool haveSpriteScaleXArray( false );
      MObject spriteScaleXObject;
      MFnDoubleArrayData spriteScaleXArray;
      float spriteScaleX;
      MPlug spriteScaleXPlug( fnNode.findPlug( "spriteScaleXPP", &status ) );
      if ( MS::kSuccess == status ) 
      {
        haveSpriteScaleXArray = true;
        spriteScaleXPlug.getValue( spriteScaleXObject );
        spriteScaleXArray.setObject( spriteScaleXObject );
      } 
      else 
      {
        spriteScaleXPlug = fnNode.findPlug( "spriteScaleX", &status );
        if ( MS::kSuccess == status ) 
        {
          haveSpriteScaleX = true;
          spriteScaleXPlug.getValue( spriteScaleX );
        }
      }

      bool haveSpriteScaleY( false );
      bool haveSpriteScaleYArray( false );
      MObject spriteScaleYObject;
      MFnDoubleArrayData spriteScaleYArray;
      float spriteScaleY;
      MPlug spriteScaleYPlug( fnNode.findPlug( "spriteScaleYPP", &status ) );
      if ( MS::kSuccess == status ) 
      {
        haveSpriteScaleYArray = true;
        spriteScaleYPlug.getValue( spriteScaleYObject );
        spriteScaleYArray.setObject( spriteScaleYObject );
      } 
      else 
      {
        spriteScaleYPlug = fnNode.findPlug( "spriteScaleY", &status );
        if ( MS::kSuccess == status ) 
        {
          haveSpriteScaleY = true;
          spriteScaleYPlug.getValue( spriteScaleY );
        }
      }

      for ( unsigned part_num( 0 ); part_num < m_numValidParticles; part_num++ ) 
      {
        Pparameter.setTokenFloat( part_num,
                                  posArray[m_validParticles[part_num]].x,
                                  posArray[m_validParticles[part_num]].y,
                                  posArray[m_validParticles[part_num]].z );
        if ( haveSpriteNumsArray )
        {   
          float spriteNumPP = spriteNumArray[ m_validParticles[ part_num ] ];
          spriteNumParameter.setTokenFloat( part_num, spriteNumPP );
          cout << "DBG> part_num = " << part_num << " spriteNumArray[ m_validParticles[ part_num ] ] = " << spriteNumArray[ m_validParticles[ part_num ] ] << endl;
          
        }
        else if ( haveSpriteNums ) 
        {  
          spriteNumParameter.setTokenFloat( part_num, spriteNum );
        }
        if ( haveSpriteTwistArray ) 
        {  
          spriteTwistParameter.setTokenFloat( part_num, spriteTwistArray[m_validParticles[part_num]] );
        }
        else if ( haveSpriteTwist ) 
        {  
          spriteTwistParameter.setTokenFloat( part_num, spriteTwist);
        }
        if ( haveSpriteScaleXArray ) 
        {  
          spriteScaleXParameter.setTokenFloat(part_num, spriteScaleXArray[m_validParticles[part_num]] );
        }
        else if ( haveSpriteScaleX )
        {   
          spriteScaleXParameter.setTokenFloat( part_num, spriteScaleX);
        }
        if ( haveSpriteScaleYArray ) 
        {  
          spriteScaleYParameter.setTokenFloat( part_num, spriteScaleYArray[m_validParticles[part_num]] );
        }  
        else if ( haveSpriteScaleY ) 
        {  
          spriteScaleYParameter.setTokenFloat( part_num, spriteScaleY);
        }  
      }
      tokenPointerArray.push_back( Pparameter );
      if ( haveSpriteNumsArray || haveSpriteNums )
      {   
        tokenPointerArray.push_back( spriteNumParameter );
      }
      if ( haveSpriteTwistArray || haveSpriteTwist )
      {   
        tokenPointerArray.push_back( spriteTwistParameter );
      }
      if ( haveSpriteScaleXArray || haveSpriteScaleX ) 
      {  
        tokenPointerArray.push_back( spriteScaleXParameter );
      }
      if ( haveSpriteScaleYArray || haveSpriteScaleY ) 
      {  
        tokenPointerArray.push_back( spriteScaleYParameter );
      }  
#endif

    } // case MPTSprites
    break;
		
		case MPTCloudy:
		{
      LIQDEBUGPRINTF( "-> Reading Cloudy Particles\n");

      // setup the arrays to store the data to pass the correct codes to the
      // implicit surface command
      m_codeArray.clear();
      m_floatArray.clear();
      m_stringArray.clear();

      LIQDEBUGPRINTF( "-> Reading Particle Data\n");

	  	// Assume same DSO call for all blobbies
	 		MPlug blobbyCodePlug = fnNode.findPlug( "liqCloudyCodes", &status );
			if ( status == MS::kSuccess ) 
			{
		  	MObject blobbyCodeObject;
		  	blobbyCodePlug.getValue( blobbyCodeObject );
		  	const MFnIntArrayData  blobbyCodeArrayData( blobbyCodeObject, &status );
		  	for ( unsigned i( 0 ); i < blobbyCodeArrayData.length(); i++ ) 
      		m_codeArray.push_back( blobbyCodeArrayData[ i ] );
		  
		  	MPlug blobbyFloatsPlug = fnNode.findPlug( "liqCloudyFloats", &status );
		  	MObject blobbyFloatsObject;
		  	blobbyFloatsPlug.getValue( blobbyFloatsObject );
		  	const MFnDoubleArrayData  blobbyFloatsArrayData( blobbyFloatsObject, &status );
		  	for ( unsigned i( 0 ); i < blobbyFloatsArrayData.length(); i++ ) 
      		m_floatArray.push_back( blobbyFloatsArrayData[ i ] );
		  
		  	MPlug blobbyStringsPlug = fnNode.findPlug( "liqCloudyStrings", &status );
		  	MObject blobbyStringsObject;
		  	blobbyStringsPlug.getValue( blobbyStringsObject );
		  	const MFnStringArrayData  blobbyStringsArrayData( blobbyStringsObject, &status );
		  	for ( unsigned i( 0 ); i < blobbyStringsArrayData.length(); i++ ) 
      		m_stringArray.push_back( blobbyStringsArrayData[ i ].asChar() );
		  
	  	} 
			else 
		{
	  	// Default to plain spheres
	  	m_codeArray.push_back( 1005 );
	  	m_codeArray.push_back( 0 );
			m_floatArray.push_back( 1.0 );
			m_floatArray.push_back( 0.0 );
			m_floatArray.push_back( 0.0 );
			m_floatArray.push_back( 0.0 );
	  }

      liqTokenPointer Pparameter;
      liqTokenPointer radiusParameter;

      Pparameter.set( "P", rPoint, true, false, m_numValidParticles );
      Pparameter.setDetailType( rVertex );
    
      radiusParameter.set( "radius", rFloat, true, false, m_numValidParticles );
      radiusParameter.setDetailType( rVertex );

      for ( unsigned part_num( 0 ); part_num < m_numValidParticles; part_num++ ) 
			{
		
        Pparameter.setTokenFloat( part_num,
                                  posArray[m_validParticles[part_num]].x,
                                  posArray[m_validParticles[part_num]].y,
                                  posArray[m_validParticles[part_num]].z );
        if ( haveRadiusArray ) 
          radiusParameter.setTokenFloat( part_num,
                                         radiusArray[m_validParticles[part_num]] );
        
        else 
          radiusParameter.setTokenFloat(part_num, radius);
        
      }
      tokenPointerArray.push_back( Pparameter );
      tokenPointerArray.push_back( radiusParameter );
		}
    break;
		
		case MPTNumeric:
    case MPTTube:
      // do nothing. These are not supported
      liquidMessage( "Numeric and Tube particle rendering types are not supported!", messageWarning );
      break;
  } // switch ( particleType )

  // and we add the Cs Parameter (if needed) after we've done everything
  // else
  if ( haveRgbArray ) 
  {
    liqTokenPointer CsParameter;

    CsParameter.set( "Cs", rColor, m_numValidParticles * m_multiCount );
    CsParameter.setDetailType( rVertex );

    for ( unsigned part_num( 0 ); part_num < m_numValidParticles * m_multiCount; part_num++ ) 
    {
      // For most of our parameters, we only have values for each "chunk"
      // (where a "chunk" is all the particles in a multi block)
      //
      unsigned part_chunk( part_num / m_multiCount );

      CsParameter.setTokenFloat( part_num,
                                 rgbArray[ m_validParticles[ part_chunk ] ].x,
                                 rgbArray[ m_validParticles[ part_chunk ] ].y,
                                 rgbArray[ m_validParticles[ part_chunk ] ].z );
    }
    tokenPointerArray.push_back( CsParameter );
  }
  // Handle per particle rotation if any
  if ( haveRotationArray ) 
	{
    liqTokenPointer rotationParameter;
    rotationParameter.set( "rotation", rColor, m_numValidParticles * m_multiCount );
    rotationParameter.setDetailType( rVertex );
    for ( unsigned part_num( 0 ); part_num < m_numValidParticles * m_multiCount; part_num++ ) 
		{
      // For most of our parameters, we only have values for each "chunk"
      // (where a "chunk" is all the particles in a multi block)
      //
      unsigned part_chunk( part_num / m_multiCount );
      rotationParameter.setTokenFloat( part_num,
                                 rotationArray[ m_validParticles[ part_chunk ] ].x,
                                 rotationArray[ m_validParticles[ part_chunk ] ].y,
                                 rotationArray[ m_validParticles[ part_chunk ] ].z );
    }
    tokenPointerArray.push_back( rotationParameter );
  }
  // And we add the Os Parameter (if needed).
  //
  if( haveOpacityArray ) 
  {
    liqTokenPointer OsParameter;

    OsParameter.set( "Os", rColor, m_numValidParticles * m_multiCount );
    OsParameter.setDetailType( rVarying );

    for ( unsigned part_num( 0 ); part_num < m_numValidParticles * m_multiCount; part_num++ ) 
    {
      // For most of our parameters, we only have values for each "chunk"
      // (where a "chunk" is all the particles in a multi block)
      //
      unsigned part_chunk( part_num / m_multiCount );

      // Fade out the even particles (the tails) if streaks.
      //
      if ( ( particleType == MPTStreak || particleType == MPTMultiStreak ) &&
           ( ( part_num & 0x01 ) == 0) )
      {
        OsParameter.setTokenFloat( part_num,
                                   opacityArray[ m_validParticles[ part_chunk ] ] * tailFade,
                                   opacityArray[ m_validParticles[ part_chunk ] ] * tailFade,
                                   opacityArray[ m_validParticles[ part_chunk ] ] * tailFade);
      } 
      else 
      {
        OsParameter.setTokenFloat( part_num,
                                   opacityArray[ m_validParticles[ part_chunk ] ],
                                   opacityArray[ m_validParticles[ part_chunk ] ],
                                   opacityArray[ m_validParticles[ part_chunk ] ]);
      }
    }
    tokenPointerArray.push_back( OsParameter );
  }

  liqTokenPointer idParameter;

  idParameter.set( "id", rFloat, m_numValidParticles * m_multiCount );
  idParameter.setDetailType( rVertex );

  for ( unsigned part_num( 0 ); part_num < m_numValidParticles * m_multiCount; part_num++ ) 
  {
    // For most of our parameters, we only have values for each "chunk"
    // (where a "chunk" is all the particles in a multi block)
    //
    unsigned part_chunk( part_num / m_multiCount );
    idParameter.setTokenFloat( part_num, particlesForSorting[ part_chunk ]->m_particleId );
  }
  tokenPointerArray.push_back( idParameter );
  liqTokenPointer velocityParameter;
  velocityParameter.set( "velocity", rVector, m_numValidParticles * m_multiCount );
  velocityParameter.setDetailType( rVertex );
  for ( unsigned part_num( 0 ); part_num < m_numValidParticles * m_multiCount; part_num++ ) 
	{
    // For most of our parameters, we only have values for each "chunk"
    // (where a "chunk" is all the particles in a multi block)
    //
    unsigned part_chunk( part_num / m_multiCount );
    velocityParameter.setTokenFloat( part_num, 
								velArray[ m_validParticles[ part_chunk ] ].x,
								velArray[ m_validParticles[ part_chunk ] ].y,
								velArray[ m_validParticles[ part_chunk ] ].z );
  }
  tokenPointerArray.push_back( velocityParameter );
  addAdditionalParticleParameters( partobj );
}

/** Write the RIB for this surface.
 */
void liqRibParticleData::write()
{
  LIQDEBUGPRINTF( "-> writing particles\n");

#ifdef DEBUG
  RiArchiveRecord( RI_COMMENT, "Number of Particles: %d", m_numValidParticles );
  RiArchiveRecord( RI_COMMENT, "Number of Discarded Particles: %d", m_numParticles - m_numValidParticles );
#endif

  unsigned numTokens( tokenPointerArray.size() );
  scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
  scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
  assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

  switch ( particleType ) 
  {
    case MPTBlobbies: 
      {
				// Build an array that can be given to RiBlobby
	  		vector< RtString > stringArray;
	  		for ( int i(0); i < m_stringArray.size(); i++ ) 
	  			stringArray.push_back( const_cast<char *>( m_stringArray[i].c_str()) );

        RiBlobbyV( m_numValidParticles,
                 m_codeArray.size(), const_cast< RtInt* >( &m_codeArray[0] ),
                 m_floatArray.size(), const_cast< RtFloat* >( &m_floatArray[0] ),
                 stringArray.size(), const_cast< RtString* >( &stringArray[0] ),
                 numTokens,
                 tokenArray.get(),
                 const_cast< RtPointer* >( pointerArray.get() ) );
        grain = 0;
      }
      break;

    case MPTMultiPoint:
    case MPTPoints:

#ifdef DELIGHT
    case MPTSpheres:
    case MPTSprites:
#endif
      {
        RiPointsV( m_numValidParticles * m_multiCount, numTokens, tokenArray.get(), pointerArray.get() );
      }
      break;

    case MPTMultiStreak:
    case MPTStreak: 
      {
        unsigned nStreaks( m_numValidParticles * m_multiCount / 2 );
        vector< RtInt > verts( nStreaks, 2 );
        // Alternatively:
        //   scoped_array< RtInt >verts( new RtInt[ nStreaks ] );
        //   fill( verts.get(), verts.get() + nStreaks, ( RtInt )2 );
        // Both ways are way faster than the frickin for() lop that was here before -- Moritz

        RiCurvesV( "linear", nStreaks, &verts[ 0 ], "nonperiodic", numTokens, tokenArray.get(), pointerArray.get() );
      }
      break;
#ifndef DELIGHT
    case MPTSpheres: 
      {
        int posAttr  = -1,
            radAttr  = -1,
            colAttr  = -1,
            opacAttr = -1;

        for ( unsigned i = 0; i < tokenPointerArray.size(); i++ )
        {
          const string tokenName( tokenPointerArray[i].getTokenName() );
          if ( "P" == tokenName )
            posAttr = i;
          else if ( "radius" == tokenName )
            radAttr = i;
          else if ( "Cs" == tokenName )
            colAttr = i;
          else if ( "Os" == tokenName )
            opacAttr = i;
        }

        for ( unsigned i = 0; i < m_numValidParticles; i++)
        {
					RiAttributeBegin();
          if ( colAttr != -1 )  RiColor( &((RtFloat*)pointerArray[colAttr])[i*3] );
          if ( opacAttr != -1 ) RiOpacity( &((RtFloat*)pointerArray[opacAttr])[i*3] );
          RiTransformBegin();
          RiTranslate(((RtFloat*)pointerArray[posAttr])[i*3+0],
                ((RtFloat*)pointerArray[posAttr])[i*3+1],
                ((RtFloat*)pointerArray[posAttr])[i*3+2]);

          RtFloat radius = ((RtFloat*)pointerArray[radAttr])[i];
          RiSphere(radius, -radius, radius, 360, RI_NULL);
          RiTransformEnd();
					RiAttributeEnd();
        }
      }
      break;

    case MPTSprites: 
      {
        int posAttr   = -1,
            numAttr    = -1,
            twistAttr  = -1,
            scaleXAttr = -1,
            scaleYAttr = -1,
            colAttr    = -1,
            opacAttr   = -1;

        for ( unsigned i( 0 ); i < tokenPointerArray.size(); i++ )
        {
          const string tokenName( tokenPointerArray[i].getTokenName() );
          if ( "P" == tokenName )
            posAttr = i;
          else if ( "spriteNum" == tokenName )
            numAttr = i;
          else if ( "spriteTwist" == tokenName )
            twistAttr = i;
          else if ( "spriteScaleX" == tokenName )
            scaleXAttr = i;
          else if ( "spriteScaleY" == tokenName )
            scaleYAttr = i;
          else if ( "Cs" == tokenName )
            colAttr = i;
          else if ( "Os" == tokenName )
            opacAttr = i;
        }

        MVector camUp( 0, 1, 0 );
        MVector camRight( 1, 0, 0 );
        MVector camEye( 0, 0, 1 );

        camUp    *= liqglo_currentJob.camera[0].mat.inverse();
        camRight *= liqglo_currentJob.camera[0].mat.inverse();
        camEye   *= liqglo_currentJob.camera[0].mat.inverse();

        for( unsigned ui( 0 ); ui < m_numValidParticles; ui++ ) 
        {
          MVector up( camUp );
          MVector right( camRight );

          float spriteRadiusX( 0.5 );
          float spriteRadiusY( 0.5 );
					RiAttributeBegin();

          if ( -1 != colAttr )  RiColor( &( ( RtFloat* )pointerArray[ colAttr ] )[ ui * 3 ] );
          if ( -1 != opacAttr ) RiOpacity( &( ( RtFloat* )pointerArray[ opacAttr ] )[ ui * 3 ] );

          if ( -1 != twistAttr ) 
          {
            float twist( -( ( RtFloat* )pointerArray[ twistAttr ] )[ ui ] * M_PI / 180 );
            MQuaternion twistQ( twist, camEye );
            right = camRight.rotateBy( twistQ );
            up    = camUp.rotateBy( twistQ );
          }

          if ( scaleXAttr != -1 ) spriteRadiusX *= ( ( RtFloat* )pointerArray[ scaleXAttr ] )[ ui ];
          if ( scaleYAttr != -1 ) spriteRadiusY *= ( ( RtFloat* )pointerArray[ scaleYAttr ] )[ ui ];

          if ( posAttr != -1 ) 
          {
            float *P( &( ( RtFloat* ) pointerArray[ posAttr ] )[ ui * 3 ] );
            float spriteNumPP = 0;
            if ( numAttr != -1 ) 
              spriteNumPP = ( ( RtFloat* )pointerArray[ numAttr ] )[ ui ];
              
            cout << ">>DBG write spriteNumPP = " << spriteNumPP << endl;
            
            float x0 = P[ 0 ] - spriteRadiusX * right[ 0 ] + spriteRadiusY * up[ 0 ];
            float y0 = P[ 1 ] - spriteRadiusX * right[ 1 ] + spriteRadiusY * up[ 1 ];
            float z0 = P[ 2 ] - spriteRadiusX * right[ 2 ] + spriteRadiusY * up[ 2 ];
            float x1 = P[ 0 ] + spriteRadiusX * right[ 0 ] + spriteRadiusY * up[ 0 ];
            float y1 = P[ 1 ] + spriteRadiusX * right[ 1 ] + spriteRadiusY * up[ 1 ];
            float z1 = P[ 2 ] + spriteRadiusX * right[ 2 ] + spriteRadiusY * up[ 2 ];
            float x2 = P[ 0 ] - spriteRadiusX * right[ 0 ] - spriteRadiusY * up[ 0 ];
            float y2 = P[ 1 ] - spriteRadiusX * right[ 1 ] - spriteRadiusY * up[ 1 ];
            float z2 = P[ 2 ] - spriteRadiusX * right[ 2 ] - spriteRadiusY * up[ 2 ];
            float x3 = P[ 0 ] + spriteRadiusX * right[ 0 ] - spriteRadiusY * up[ 0 ];
            float y3 = P[ 1 ] + spriteRadiusX * right[ 1 ] - spriteRadiusY * up[ 1 ];
            float z3 = P[ 2 ] + spriteRadiusX * right[ 2 ] - spriteRadiusY * up[ 2 ];

            float patch[ 12 ] = { x0, y0, z0,
                                  x1, y1, z1,
                                  x2, y2, z2,
                                  x3, y3, z3 };
            // !!! if not GENERIC_RIBLIB use RiPatch( "bilinear", "P", &patch, "float spriteNum", &spriteNum, RI_NULL );                                  
            // RiPatch( "bilinear", "P", &patch, "float spriteNum", (RtFloat*)&spriteNumPP, RI_NULL );
            // Patch "bilinear"  "P" [0.446265 0.316269 -0.647637 1.27725 0.316269 -1.20393 0.615752 -0.636188 -0.39446 1.44674 -0.636188 -0.950756 ]  "float spriteNum" [2 0 0 0 ]
            RiArchiveRecord( RI_VERBATIM, "Patch \"bilinear\" \"P\" [%f %f %f %f %f %f %f %f %f %f %f %f] \"float spriteNum\" [%f]", 
                                          x0, y0, z0,x1, y1, z1, x2, y2, z2,x3, y3, z3,
                                            spriteNumPP ); 
            RiAttributeEnd();
          } 
          else
          {   
            RiIdentity();
          }
        }
				// RiAttributeEnd();
      }
      break;

#endif // #ifndef DELIGHT

  	case MPTCloudy:  
		{
      int posAttr  = -1,
          radAttr  = -1,
          colAttr  = -1,
          opacAttr = -1,
		  rotAttr  = -1;

      for ( unsigned i = 0; i < tokenPointerArray.size(); i++ )
      {
        const string tokenName( tokenPointerArray[i].getTokenName() );
        if ( "P" == tokenName )
          posAttr = i;
        else if ( "radius" == tokenName )
          radAttr = i;
        else if ( "Cs" == tokenName )
          colAttr = i;
        else if ( "Os" == tokenName )
          opacAttr = i;
        else if ( "rotation" == tokenName )
          rotAttr = i;
      }
	  	// Build an array that can be given to RiBlobby
	  	vector< RtString > stringArray;
	  	for ( unsigned int i(0); i < m_stringArray.size(); i++ ) 
	  		stringArray.push_back( const_cast<char *>( m_stringArray[i].c_str()) );
	  
	  	scoped_array< RtToken > ithTokenArray( new RtToken[ numTokens ] );
	  	scoped_array< RtPointer > ithPointerArray( new RtPointer[ numTokens ] );
		
      for ( unsigned i = 0; i < m_numValidParticles; i++)
      {
  			assignIthTokenArraysV( tokenPointerArray, ithTokenArray.get(), ithPointerArray.get(), i );
	  		RiAttributeBegin();
        if ( colAttr != -1 )  RiColor( &((RtFloat*)pointerArray[colAttr])[i*3] );
        if ( opacAttr != -1 ) RiOpacity( &((RtFloat*)pointerArray[opacAttr])[i*3] );
        RiTransformBegin();
        RiTranslate(((RtFloat*)pointerArray[posAttr])[i*3+0],
              ((RtFloat*)pointerArray[posAttr])[i*3+1],
              ((RtFloat*)pointerArray[posAttr])[i*3+2]);
		
        if ( rotAttr != -1 )
        {
          RiRotate( (( RtFloat *) pointerArray[rotAttr])[i*3] * 360.0, 1.0, 0.0, 0.0 );
          RiRotate( (( RtFloat *) pointerArray[rotAttr])[i*3+1] * 360.0, 0.0, 1.0, 0.0 );
          RiRotate( (( RtFloat *) pointerArray[rotAttr])[i*3+2] * 360.0, 0.0, 0.0, 1.0 );
        }

        RtFloat radius = ((RtFloat*)pointerArray[radAttr])[i];
				RiScale( radius, radius, radius );
        //RiSphere(radius, -radius, radius, 360, RI_NULL);
				float dummy[] = { 0.0, 0.0, 0.0 }; // Worst case : three floats are needed
				RiBlobbyV( 1,
				m_codeArray.size(), const_cast< RtInt* >( &m_codeArray[0] ),
				m_floatArray.size(), const_cast< RtFloat* >( &m_floatArray[0] ),
				stringArray.size(), const_cast< RtString* >( &stringArray[0] ),
				numTokens, ithTokenArray.get(), ithPointerArray.get() );
//			"vertex color incandescence", (RtPointer *)( dummy ),
//			"vertex color Cs", (RtPointer *)( dummy ),
//			"vertex float selfshadow", (RtPointer *)( dummy ),
//			RI_NULL );
        RiTransformEnd();
				RiAttributeEnd();
      }
	  break;
    }

		case MPTNumeric:
    case MPTTube:
      // do nothing. These are not supported
      break;
  }
}

unsigned liqRibParticleData::granularity() const 
{
  switch ( particleType ) 
  {
    case MPTBlobbies:
    case MPTMultiPoint:
    case MPTPoints:
#ifdef DELIGHT
    case MPTSpheres:
    case MPTSprites:
#endif
    case MPTMultiStreak:
    case MPTStreak:
      return 1;

#ifndef DELIGHT
    case MPTSpheres:
    case MPTSprites:
#endif // #ifndef DELIGHT
      return m_numValidParticles;

    case MPTNumeric:
    case MPTCloudy:
    case MPTTube:
      // These are not supported
      break;
  }
  return 0;
}

/** Write the RIB for this surface -- grain by grain.
 */
bool liqRibParticleData::writeNextGrain()
{
  LIQDEBUGPRINTF( "-> writing particles\n");

#ifdef DEBUG
  RiArchiveRecord( RI_COMMENT, "Number of Particles: %d", m_numValidParticles );
  RiArchiveRecord( RI_COMMENT, "Number of Discarded Particles: %d", m_numParticles - m_numValidParticles );
#endif

  unsigned numTokens( tokenPointerArray.size() );
  scoped_array< RtToken > tokenArray( new RtToken[ numTokens ] );
  scoped_array< RtPointer > pointerArray( new RtPointer[ numTokens ] );
  assignTokenArraysV( tokenPointerArray, tokenArray.get(), pointerArray.get() );

  switch ( particleType ) 
  {
    case MPTBlobbies: 
      {
        // Build an array that can be given to RiBlobby
	  		vector< RtString > stringArray;
	  		for ( int i(0); i < m_stringArray.size(); i++ ) 
	  			stringArray.push_back( const_cast<char *>( m_stringArray[i].c_str() ) );
				
				RiBlobbyV( m_numValidParticles,
                 	 m_codeArray.size(), const_cast< RtInt* >( &m_codeArray[0] ),
                 	 m_floatArray.size(), const_cast< RtFloat* >( &m_floatArray[0] ),
                   stringArray.size(), const_cast< RtString* >( &stringArray[0] ),
                   numTokens,
                   tokenArray.get(),
                   const_cast< RtPointer* >( pointerArray.get() ) );

        grain = 0;
        return false;
      }
    case MPTMultiPoint:
    case MPTPoints:
#ifdef DELIGHT
    case MPTSpheres:
    case MPTSprites:
#endif
    {
      RiPointsV( m_numValidParticles*m_multiCount, numTokens, tokenArray.get(), pointerArray.get() );
      grain = 0;
      return false;
    }
    case MPTMultiStreak:
    case MPTStreak: 
      {
        unsigned nStreaks( m_numValidParticles * m_multiCount / 2 );
        vector< RtInt > verts( nStreaks, 2 );
        // Alternatively:
        //   fill( verts.get(), verts.get() + nStreaks, ( RtInt )2 );
        //   scoped_array< RtInt >verts( new RtInt[ nStreaks ] );

        RiCurvesV( "linear", nStreaks, &verts[ 0 ], "nonperiodic", numTokens, tokenArray.get(), pointerArray.get() );

        grain = 0;
        return false;
      }
#ifndef DELIGHT
    case MPTSpheres: 
      {
        if( m_numValidParticles > grain ) 
        {
          int posAttr  = -1,
              radAttr  = -1,
              colAttr  = -1,
              opacAttr = -1;

          for ( unsigned i( 0 ); i < tokenPointerArray.size(); i++ )
          {
            const string tokenName( tokenPointerArray[i].getTokenName() );
            if ( "P" == tokenName )
              posAttr = i;
            else if ( "radius" == tokenName )
              radAttr = i;
            else if ( "Cs" == tokenName )
              colAttr = i;
            else if ( "Os" == tokenName )
              opacAttr = i;
          }
          if ( colAttr != -1 )  RiColor( &( ( RtFloat* )pointerArray[ colAttr ] )[ grain * 3 ] );
          if ( opacAttr != -1 ) RiOpacity( &( ( RtFloat* )pointerArray[ opacAttr ] )[ grain * 3 ] );
          
          RiTransformBegin();
          RiTranslate((( RtFloat* )pointerArray[ posAttr ] )[ grain *3 + 0 ],
                (( RtFloat* )pointerArray[ posAttr ] )[ grain * 3 + 1 ],
                (( RtFloat* )pointerArray[ posAttr ] )[ grain * 3 + 2 ]);

          RtFloat radius( ( ( RtFloat* )pointerArray[ radAttr ] )[ grain ] );
          RiSphere( radius, -radius, radius, 360, RI_NULL );
          RiTransformEnd();

          ++grain;
          return true;
        } 
        else 
        {
          grain = 0;
          return false;
        }
      }

    case MPTSprites: 
      {
        if ( m_numValidParticles > grain ) 
        {
          int posAttr    = -1,
              numAttr    = -1,
              twistAttr  = -1,
              scaleXAttr = -1,
              scaleYAttr = -1,
              colAttr    = -1,
              opacAttr   = -1;

          for ( unsigned i( 0 ); i < tokenPointerArray.size(); i++ )
          {
            const string tokenName( tokenPointerArray[ i ].getTokenName() );
            if ( "P" == tokenName ) 
              posAttr = i;
            else if ( "spriteNum" == tokenName ) 
              numAttr = i;
            else if ( "spriteTwist" == tokenName ) 
              twistAttr = i;
            else if ( "spriteScaleX" == tokenName ) 
              scaleXAttr = i;
            else if ( "spriteScaleY" == tokenName ) 
              scaleYAttr = i;
            else if ( "Cs" == tokenName ) 
              colAttr = i;
            else if ( "Os" == tokenName ) 
              opacAttr = i;
          }
          MVector camUp( 0, 1, 0 );
          MVector camRight( 1, 0, 0 );
          MVector camEye( 0, 0, 1 );

          camUp    *= liqglo_currentJob.camera[0].mat.inverse();
          camRight *= liqglo_currentJob.camera[0].mat.inverse();
          camEye   *= liqglo_currentJob.camera[0].mat.inverse();

          //MGlobal::displayInfo( MString( "I: " ) + ( ( double ) grain ) );
          MVector up( camUp );
          MVector right( camRight );

          float spriteRadiusX = 0.5;
          float spriteRadiusY = 0.5;

          if ( colAttr != -1 )  RiColor( &( ( RtFloat* )pointerArray[ colAttr ] )[ grain * 3 ] );
          if ( opacAttr != -1 ) RiOpacity( &( ( RtFloat* )pointerArray[ opacAttr ] )[ grain * 3 ] );

          if ( twistAttr != -1 ) 
          {
            float twist( -( ( RtFloat* )pointerArray[ twistAttr ] )[ grain ] * M_PI / 180 );
            MQuaternion twistQ( twist, camEye );
            right = camRight.rotateBy( twistQ );
            up    = camUp.rotateBy( twistQ );
          }
          if ( scaleXAttr != -1 ) spriteRadiusX *= ( ( RtFloat* )pointerArray[ scaleXAttr ] )[ grain ];
          if ( scaleYAttr != -1 ) spriteRadiusY *= ( ( RtFloat* )pointerArray[ scaleYAttr ] )[ grain ];

          if ( posAttr != -1 ) 
          {
            float *P( &( ( RtFloat* ) pointerArray[ posAttr ] )[ grain * 3 ] );
            float spriteNum( (numAttr == -1) ? 0 : ( ( RtFloat* )pointerArray[ numAttr ] )[ grain ] );
            cout << ">DBG write next grain spriteNum = " << spriteNum << endl;

            float x0 = P[ 0 ] - spriteRadiusX * right[ 0 ] + spriteRadiusY * up[ 0 ];
            float y0 = P[ 1 ] - spriteRadiusX * right[ 1 ] + spriteRadiusY * up[ 1 ];
            float z0 = P[ 2 ] - spriteRadiusX * right[ 2 ] + spriteRadiusY * up[ 2 ];
            float x1 = P[ 0 ] + spriteRadiusX * right[ 0 ] + spriteRadiusY * up[ 0 ];
            float y1 = P[ 1 ] + spriteRadiusX * right[ 1 ] + spriteRadiusY * up[ 1 ];
            float z1 = P[ 2 ] + spriteRadiusX * right[ 2 ] + spriteRadiusY * up[ 2 ];
            float x2 = P[ 0 ] - spriteRadiusX * right[ 0 ] - spriteRadiusY * up[ 0 ];
            float y2 = P[ 1 ] - spriteRadiusX * right[ 1 ] - spriteRadiusY * up[ 1 ];
            float z2 = P[ 2 ] - spriteRadiusX * right[ 2 ] - spriteRadiusY * up[ 2 ];
            float x3 = P[ 0 ] + spriteRadiusX * right[ 0 ] - spriteRadiusY * up[ 0 ];
            float y3 = P[ 1 ] + spriteRadiusX * right[ 1 ] - spriteRadiusY * up[ 1 ];
            float z3 = P[ 2 ] + spriteRadiusX * right[ 2 ] - spriteRadiusY * up[ 2 ];

            float patch[ 12 ] = { x0, y0, z0,
                                  x1, y1, z1,
                                  x2, y2, z2,
                                  x3, y3, z3 };
            // RiPatch( "bilinear", "P", &patch, "float spriteNum", &spriteNum, RI_NULL );
             RiArchiveRecord( RI_VERBATIM, "Patch \"bilinear\" \"P\" [%f %f %f %f %f %f %f %f %f %f %f %f] \"float spriteNum\" [%f]", 
                                          x0, y0, z0,x1, y1, z1, x2, y2, z2,x3, y3, z3,
                                            spriteNum ); 
          } 
					else 
            RiIdentity();

          ++grain;
          return true;
        } 
        else 
        {
          grain = 0;
          return false;
        }
      }

#endif // #ifndef DELIGHT

    case MPTNumeric:
    case MPTCloudy:
    case MPTTube:
      // do nothing. These are not supported
      break;
  }
  return false;
}

/** Compare this curve to the other for the purpose of determining
 *  if it is animated.
 */
bool liqRibParticleData::compare( const liqRibData & otherObj ) const
{
  LIQDEBUGPRINTF( "-> comparing particles\n");

  if ( otherObj.type() != MRT_Particles ) return false;

  const liqRibParticleData & other = (liqRibParticleData&)otherObj;

  if ( m_numParticles != other.m_numParticles ) return false;

  return true;
}

/** Return the geometry type.
 */
ObjectType liqRibParticleData::type() const
{
  LIQDEBUGPRINTF( "-> returning particle type\n");
  return MRT_Particles;
}

/** This replaces the standard method for attaching custom attributes to a
 *  particle set to be passed into the RIB stream for access in a RMan
 *  shader.
 */
void liqRibParticleData::addAdditionalParticleParameters( MObject node )
{
  LIQDEBUGPRINTF("-> scanning for additional rman surface attributes \n");

  MFnDependencyNode nodeFn( node );

  addAdditionalFloatParameters( nodeFn );
  addAdditionalVectorParameters( nodeFn, "rmanP", rPoint );
  addAdditionalVectorParameters( nodeFn, "rmanV", rVector );
  addAdditionalVectorParameters( nodeFn, "rmanN", rNormal );
  addAdditionalVectorParameters( nodeFn, "rmanC", rColor );
}

void liqRibParticleData::addAdditionalFloatParameters( MFnDependencyNode &nodeFn )
{
  MStringArray foundAttributes = findAttributesByPrefix( "rmanF", nodeFn );
  MStatus  status;

  for ( int i = 0; i < foundAttributes.length(); i++ ) 
  {
    liqTokenPointer floatParameter;
    MString  currAttribute = foundAttributes[i];
    MString  cutString = currAttribute.substring(5, currAttribute.length());

    MPlug  fPlug = nodeFn.findPlug( currAttribute );
    MObject  plugObj;

    status = fPlug.getValue( plugObj );

    if ( plugObj.apiType() == MFn::kDoubleArrayData ) 
    {
      MFnDoubleArrayData attributeData( plugObj );

      floatParameter.set( cutString.asChar(), rFloat, m_numValidParticles );
      floatParameter.setDetailType(rVertex);

      for ( unsigned part_num( 0 ); part_num < m_numValidParticles; part_num++ ) 
        floatParameter.setTokenFloat( part_num, 
																			attributeData[ m_validParticles[ part_num ] ] );
      
    } 
    else 
    {
      float floatValue;

      fPlug.getValue( floatValue );

      floatParameter.set( cutString.asChar(), rFloat );
      floatParameter.setDetailType( rConstant );
      floatParameter.setTokenFloat( 0, floatValue );
    }

    tokenPointerArray.push_back( floatParameter );
  }
}

void liqRibParticleData::addAdditionalVectorParameters( MFnDependencyNode &nodeFn, const string& prefix, ParameterType type )
{
  MStringArray foundAttributes = findAttributesByPrefix( prefix.c_str(), nodeFn );
  MStatus  status;

  for ( unsigned i( 0 ); i < foundAttributes.length(); i++ ) 
  {
    liqTokenPointer vectorParameter;
    MString  currAttribute = foundAttributes[i];
    MString  cutString = currAttribute.substring(5, currAttribute.length());

    MPlug  vPlug = nodeFn.findPlug( currAttribute );
    MObject  plugObj;

    status = vPlug.getValue( plugObj );

    if ( plugObj.apiType() == MFn::kVectorArrayData ) 
    {
      MFnVectorArrayData  attributeData( plugObj );

      vectorParameter.set( cutString.asChar(), type, m_numValidParticles );
      vectorParameter.setDetailType( rVertex );

      for ( unsigned part_num( 0 ); part_num < m_numValidParticles; part_num++ ) 
      {
        vectorParameter.setTokenFloat( part_num,
                         attributeData[m_validParticles[part_num]].x,
                         attributeData[m_validParticles[part_num]].y,
                         attributeData[m_validParticles[part_num]].z );
      }

      tokenPointerArray.push_back( vectorParameter );
    } 
    else if ( plugObj.apiType() == MFn::kData3Double ) 
    {
      float x, y, z;
      vPlug.child(0).getValue( x );
      vPlug.child(1).getValue( y );
      vPlug.child(2).getValue( z );

      vectorParameter.set( cutString.asChar(), type );
      vectorParameter.setTokenFloat( 0, x, y, z );
      vectorParameter.setDetailType( rConstant );

      tokenPointerArray.push_back( vectorParameter );
    }
    // else ignore this attribute
  }
}
