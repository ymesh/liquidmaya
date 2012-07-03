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

//#include <liquid.h>
#include <stdio.h>
#include <string>
#include <strstream>

extern "C" {
#include <ri.h>
}
#include <liqTokenPointer.h>
#include <liquid.h>

#include <string>
#include <boost/shared_array.hpp>

using namespace std;
using namespace boost;

extern int debugMode;


const string liqTokenPointer::detailType[] = {
  "uniform",
  "varying",
  "vertex",
  "constant",
  "facevarying",
  "facevertex"
};


liqTokenPointer::liqTokenPointer()
{
  m_pType        = rFloat;
  m_isArray      = false;
  m_isUArray     = false;
  m_isString     = false;
  m_isFull       = false;
  m_arraySize    = 0;
  m_uArraySize   = 0;
  m_eltSize      = 0;
  m_tokenSize    = 0;
  m_stringSize   = 0;
  m_dType        = rUndefined;
}

liqTokenPointer::liqTokenPointer( const liqTokenPointer &src )
{
  LIQDEBUGPRINTF( "-> copy constructing additional ribdata: %s\n", src.m_tokenName.c_str() );

  m_isArray       = false;
  m_isUArray      = false;
  m_isString      = false;
  m_isFull        = false;
  m_arraySize     = 0;
  m_uArraySize    = 0;
  m_eltSize       = 0;
  m_tokenSize     = 0;
  m_stringSize    = 0;

  if ( src.m_isUArray )
    // Moritz: this is the baddy: src.m_arraySize wasn't divided by m_uArraySize!
    //         m_uArraySize actually is only used in the set() method to calculate
    //         m_arraySize and in getRiDeclare()
    // Todo:   Find a less Italian (non-spaghetticode) solution to this.
    set( src.m_tokenName, src.m_pType, src.m_arraySize, src.m_uArraySize );
  else
    set( src.m_tokenName, src.m_pType, src.m_arraySize );
  m_dType = src.m_dType;

  if ( m_pType != rString ) 
	{
    //setTokenFloats( src.m_tokenFloats.get() );
    m_tokenFloats = src.m_tokenFloats; // Superfast: just increase shared_array's reference counter. However, ownership is shared now! Test if this works everywhere accross Liquid.
  } 
	else 
    m_tokenString = src.m_tokenString;
  LIQDEBUGPRINTF("-> done copy constructing additional ribdata: %s\n", src.m_tokenName.c_str() );
}

liqTokenPointer & liqTokenPointer::operator=( const liqTokenPointer &src)
{
  LIQDEBUGPRINTF("-> copying additional ribdata: %s\n", src.m_tokenName.c_str() );

  reset();
  if ( src.m_isUArray ) set( src.m_tokenName, src.m_pType, src.m_arraySize, src.m_uArraySize );
  else                  set( src.m_tokenName, src.m_pType, src.m_arraySize );
  m_dType = src.m_dType;
  if ( m_pType != rString ) 
	{
    //setTokenFloats( src.m_tokenFloats.get() );
    m_tokenFloats = src.m_tokenFloats; 
		// Superfast: just increase shared_array's reference counter. However, ownership is shared now! 
		// Test if this works everywhere accross Liquid.
  } 
	else 
    m_tokenString = src.m_tokenString;

  LIQDEBUGPRINTF("-> done copying additional ribdata: %s\n", src.m_tokenName.c_str() );
  return *this;
}

liqTokenPointer::~liqTokenPointer()
{
  LIQDEBUGPRINTF("-> freeing additional ribdata: %s\n ", m_tokenName.c_str() );

  //if( m_tokenFloats ) { lfree( m_tokenFloats ); m_tokenFloats = NULL; }
  //resetTokenString();

};

void liqTokenPointer::reset()
{
  //if( m_tokenFloats ) { lfree( m_tokenFloats ); m_tokenFloats = NULL; }
  m_tokenFloats.reset();
  m_tokenString.clear();
  m_isArray      = false;
  m_isUArray     = false;
  m_isString     = false;
  m_isFull       = false;
  m_arraySize    = 0;
  m_uArraySize   = 0;
  m_eltSize      = 0;
  m_tokenSize    = 0;
  m_stringSize   = 0;
  m_pType        = rFloat;
  m_tokenName[0] = '\0';
  m_dType        = rUndefined;
}

bool liqTokenPointer::set( const string& name, ParameterType ptype )
{
  return set( name, ptype, 0, 0 );
}

bool liqTokenPointer::set( const string& name, ParameterType ptype, unsigned int arraySize )
{
  return set( name, ptype, arraySize, 0 );
}

bool liqTokenPointer::set( const string& name, ParameterType ptype, bool asArray, bool asUArray, unsigned int arraySize )
{
  // philippe : passing arraySize when asUArray is true fixed the float array export problem
  // TO DO : replace occurences of this function with non-obsolete ones
  //return set( name, ptype, arraySize, asUArray ? 2 : 0 );

  return set( name, ptype, asArray ? arraySize : 1, asUArray ? arraySize : 0 );
}

bool liqTokenPointer::set( const string& name, ParameterType ptype, unsigned int arraySize, unsigned int uArraySize )
{
  setTokenName( name );
  m_pType = ptype;

  if ( m_pType!=rString && m_pType!=rShader ) 
	{
    resetTokenString();

    // define element size based on parameter type
    switch ( m_pType ) 
		{
      case rFloat:
        m_eltSize = 1;
        break;
      case rPoint:
      case rColor:
      case rNormal:
      case rVector:
        m_eltSize = 3;
        break;
      case rHpoint:
        m_eltSize = 4;
        break;
      case rString: // Useless but prevent warning at compile time
      case rShader: // Useless but prevent warning at compile time
        m_eltSize = 0;
        break;
      case rMatrix:
        m_eltSize = 16;
        break;
    }

    // check how much we need if we have an array
    m_isArray = arraySize != 0;
    unsigned neededSize;
    if ( m_isArray ) 
		{
      m_arraySize  = arraySize;
      m_isUArray   = bool( uArraySize );
      m_uArraySize = uArraySize;
      neededSize = m_arraySize * m_eltSize;
      if( m_isUArray )
        neededSize *= uArraySize;
    } 
		else 
		{
      m_arraySize = 0;
      neededSize = m_eltSize;
    }
    // allocate whatever we need
    // Check if we already got enough space
    if ( m_tokenSize < neededSize ) 
		{
      LIQDEBUGPRINTF("-> allocating memory 1 for: %s\n", name.c_str() );
      m_tokenFloats = shared_array< RtFloat >( new RtFloat[ neededSize ] );
      if ( ! m_tokenFloats ) 
			{
        m_tokenSize = 0;
        cerr << "Error : liqTokenPointer out of memory for " << neededSize << " bytes" << endl;
        return false;
      }
      m_tokenSize = neededSize;
    } 
		else if ( neededSize ) 
		{
      LIQDEBUGPRINTF("-> allocating memory 2 for: %s\n", name.c_str() );
      m_tokenFloats = shared_array< RtFloat >( new RtFloat[ neededSize ] );
      if ( ! m_tokenFloats ) 
			{
        cerr << "Error : liqTokenPointer out of memory for " << neededSize << " bytes" << endl;
        return false;
      }
      m_tokenSize = neededSize;
    }
    LIQDEBUGPRINTF( "-> array size: %d\n", arraySize );
    LIQDEBUGPRINTF( "-> needed %u got %d\n", neededSize, m_tokenSize );
  } 
	else 
	{
    // STRINGS ARE A SPECIAL CASE
    // Space is now allocated upfront

    m_tokenFloats.reset();
    resetTokenString();

    m_isUArray = false;
    m_isArray  = arraySize != 0;

    if ( m_isArray ) 
		{
      // init array size
      m_arraySize = arraySize;
      m_tokenSize = 0;
      // init pointer array
      m_tokenString.resize( m_arraySize );
    } 
		else 
		{
      m_arraySize   = 0; // useless
      m_tokenSize   = 0;
      m_tokenString.resize( 1 );
    }
  }
  return true;
}

int liqTokenPointer::reserve( unsigned int size )
{
  if( m_pType != rString ) 
	{
    if( m_arraySize != size )
    {
      // Only augment allocated memory if needed, do not reduce it
      unsigned long neededSize( size * m_eltSize );
      if ( m_isUArray ) neededSize *= m_uArraySize;
      if ( m_tokenSize < neededSize ) 
			{
        shared_array< RtFloat > tmp( new RtFloat[ neededSize ] );
        memcpy( tmp.get(), m_tokenFloats.get(), m_tokenSize );
        m_tokenFloats = tmp;
        m_tokenSize = neededSize;
      }
    }
  } 
	else 
    m_tokenString.resize( size );
  
  m_arraySize = size;
  return m_arraySize;
}

void liqTokenPointer::setDetailType( DetailType dType )
{
  m_dType = dType;
}

DetailType liqTokenPointer::getDetailType() const
{
  return m_dType;
}

ParameterType liqTokenPointer::getParameterType() const
{
  return m_pType;
}

void liqTokenPointer::setTokenFloat( unsigned int i, RtFloat val )
{
  assert( m_tokenSize > i );
  m_tokenFloats[i] = val;
}

void liqTokenPointer::setTokenFloat( unsigned int i, unsigned int uIndex, RtFloat val )
{
  assert( m_tokenSize > ( i * m_uArraySize + uIndex ) );
  setTokenFloat( i * m_uArraySize + uIndex, val );
}

void liqTokenPointer::setTokenFloat( unsigned int i, RtFloat x, RtFloat y , RtFloat z )
{
  assert( m_tokenSize > ( m_eltSize * i + 2 ) );
  m_tokenFloats[ m_eltSize * i + 0 ] = x;
  m_tokenFloats[ m_eltSize * i + 1 ] = y;
  m_tokenFloats[ m_eltSize * i + 2 ] = z;
}

void liqTokenPointer::setTokenFloats( const RtFloat* vals )
{
  LIQDEBUGPRINTF( "-> copying data\n" );
  if ( m_isArray || m_isUArray ) 
	{
    //m_tokenFloats = shared_array< RtFloat >( new RtFloat[ m_tokenSize ] );
    memcpy( m_tokenFloats.get(), vals, m_tokenSize * sizeof( RtFloat) );
  } 
	else 
	{
    //m_tokenFloats = shared_array< RtFloat >( new RtFloat[ m_eltSize ] );
    memcpy( m_tokenFloats.get(), vals, m_eltSize * sizeof( RtFloat) );
  }
}

void liqTokenPointer::setTokenFloats( const shared_array< RtFloat > vals )
{
  m_tokenFloats = vals;
}

const RtFloat* liqTokenPointer::getTokenFloatArray() const
{
  return m_tokenFloats.get();
}

const shared_array< RtFloat > liqTokenPointer::getTokenFloatSharedArray() const
{
  return m_tokenFloats;
}

void liqTokenPointer::setTokenFloat( unsigned int i, RtFloat x, RtFloat y , RtFloat z, RtFloat w )
{
  assert( m_tokenSize > ( m_eltSize * i + 3 ) );
  m_tokenFloats[ m_eltSize * i + 0 ] = x;
  m_tokenFloats[ m_eltSize * i + 1 ] = y;
  m_tokenFloats[ m_eltSize * i + 2 ] = z;
  m_tokenFloats[ m_eltSize * i + 3 ] = w;
}


void liqTokenPointer::setTokenFloat( unsigned int i, RtFloat x1, RtFloat y1 , RtFloat z1, RtFloat w1, RtFloat x2, RtFloat y2 , RtFloat z2, RtFloat w2, RtFloat x3, RtFloat y3 , RtFloat z3, RtFloat w3, RtFloat x4, RtFloat y4 , RtFloat z4, RtFloat w4 )
{
  assert( m_tokenSize > ( m_eltSize * i + 15 ) );
  m_tokenFloats[ m_eltSize * i + 0 ] = x1;
  m_tokenFloats[ m_eltSize * i + 1 ] = y1;
  m_tokenFloats[ m_eltSize * i + 2 ] = z1;
  m_tokenFloats[ m_eltSize * i + 3 ] = w1;
  m_tokenFloats[ m_eltSize * i + 4 ] = x2;
  m_tokenFloats[ m_eltSize * i + 5 ] = y2;
  m_tokenFloats[ m_eltSize * i + 6 ] = z2;
  m_tokenFloats[ m_eltSize * i + 7 ] = w2;
  m_tokenFloats[ m_eltSize * i + 8 ] = x3;
  m_tokenFloats[ m_eltSize * i + 9 ] = y3;
  m_tokenFloats[ m_eltSize * i + 10 ] = z3;
  m_tokenFloats[ m_eltSize * i + 11 ] = w3;
  m_tokenFloats[ m_eltSize * i + 12 ] = x4;
  m_tokenFloats[ m_eltSize * i + 13 ] = y4;
  m_tokenFloats[ m_eltSize * i + 14 ] = z4;
  m_tokenFloats[ m_eltSize * i + 15 ] = w4;
}

string liqTokenPointer::getTokenString() const
{
  return m_tokenString[0];
}

void liqTokenPointer::setTokenString( unsigned int i, const string& str )
{
  assert( i >= m_arraySize );

  m_tokenString[ i ] = str;
}

void liqTokenPointer::resetTokenString()
{
  m_tokenString.clear();
  m_tokenStringArray.reset();
}

void liqTokenPointer::setTokenName( const string& name )
{
  m_tokenName = name;
}

string liqTokenPointer::getTokenName() const
{
  // Hmmmm should we handle token without name ?
  return m_tokenName;
}

const string& liqTokenPointer::getDetailedTokenName()
{
  // Hmmmm should we handle token without name ?
#ifdef PRMAN
  // Philippe : in PRMAN, declaring P as a vertex point is not necessary and it make riCurves generation fail.
  // so when the token is P, we just skip the type declaration.
  if ( "P" != m_tokenName )
    detailedTokenName = getRiDeclare() + " ";
  else
		detailedTokenName = "";
#else
  detailedTokenName = getRiDeclare() + " ";
#endif
  detailedTokenName += m_tokenName;
  return detailedTokenName;
}

const RtPointer liqTokenPointer::getRtPointer()
{
  if ( m_pType == rString ) 
	{
    m_tokenStringArray = shared_array< RtString >( new RtString[ m_tokenString.size() ] );

		for ( unsigned i( 0 ); i < m_tokenString.size(); i++ ) 
			m_tokenStringArray[ i ] = const_cast< RtString >( m_tokenString[ i ].c_str() );
	
    return ( RtPointer )m_tokenStringArray.get();
  } 
	else 
    return ( RtPointer )m_tokenFloats.get();
}

// Return a RtPointer for a token corresponding to the ith element of a primitive
const RtPointer liqTokenPointer::getIthRtPointer( unsigned int i )
{
  if( m_pType == rString ) 
	{
    assert( m_arraySize > i );
    m_tokenStringArray = shared_array< RtString >( new RtString[ 1 ] );
		m_tokenStringArray[ 0 ] = const_cast< RtString >( m_tokenString[ i ].c_str() );
    return ( RtPointer )m_tokenStringArray.get();
  } 
	else 
	{
    if( m_isArray || m_isUArray ) 
		{
  	  assert( m_tokenSize > ( m_eltSize * i ) );
      return ( RtPointer ) ( m_tokenFloats.get() + m_eltSize * i);
    } 
		else 
      return ( RtPointer )m_tokenFloats.get(); // uniform ...
  }
}

string liqTokenPointer::getRiDeclare() const
{
	string type;
	switch ( m_pType )
	{
	case rString:
		type = "string";
		break;
	case rShader:
		type = "shader";
		break;
	case rMatrix:
		type = "matrix";
		break;
	case rFloat:
		type = "float";
		break;
	case rHpoint:
		type = "hpoint";
		break;
	case rPoint:
		type = "point";
		break;
	case rVector:
		type = "vector";
		break;
	case rNormal:
		type = "normal";
		break;
	case rColor:
		type = "color";
		break;
	}
	LIQDEBUGPRINTF("liqTokenPointer :: TYPE=%s _ m_isUArray=%d _ m_isArray=%d _ m_uArraySize=%d _ m_arraySize=%d \n", type.c_str(), m_isUArray, m_isArray, m_uArraySize, m_arraySize);
	if ( m_pType == rString || m_pType == rShader )
	{
		if ( m_isArray )
		{
			strstream declare;
			declare << "[" << m_arraySize << "]" << ends;
			type += declare.str();
		}
	}
	else
	{
		if ( m_isUArray )
		{
			strstream declare;
			declare << "[" << m_uArraySize << "]" << ends;
			type += declare.str();
		}
	}
	if ( rUndefined != m_dType ) type = detailType[ m_dType ] + " " + type;
	return type;
}

liqTokenPointer::operator bool() const
{
  return m_tokenFloats;
}

bool liqTokenPointer::empty() const
{
  return !m_tokenFloats;
}

bool liqTokenPointer::isBasicST() const
{
  // Not st or, if it is, face varying
  if ( m_tokenName[0] != 's' || m_tokenName[1] != 't' || 
			 m_tokenName.length() != 2 || m_dType == rFaceVarying || m_dType == rFaceVertex ) {
    return false;
  }
  return true;
}

