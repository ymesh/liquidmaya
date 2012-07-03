#ifndef _H_liquidRIBGen

#define _H_liquidRIBGen

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
** Liquid RibGen Header File  
** ______________________________________________________________________
*/

#include <ri.h>
#include <liquidRIBStatus.h>
#include "zlib.h"
class liquidRIBGen;

extern "C" RIPSTREAM *RIOutputStream;
extern "C" RIPSTREAM *RIInputStream;

typedef	enum
{
	RIPFORMAT_UNDEF,
	RIPFORMAT_ASCII,
	RIPFORMAT_BINARY
} RIPFormat;
	
extern "C" RIPFormat RipFormat;

typedef enum
{
 	RIPCOMPRESSION_UNDEF,
    RIPCOMPRESSION_NONE,
	RIPCOMPRESSION_GZIP
} RIPCompression;
			
extern "C" RIPCompression RipCompression;
typedef int RtError;

#define RIPPRECISION_UNDEF -1
#define RIPPRECISION_DEFAULT 6
extern "C" RtInt RipPrecision;
extern	"C" RtError RIPPushHandleContext(void);
extern	"C" RtVoid RIPInitTokenStore(void);
extern	"C" RtVoid RIPInitParamStore(void);
extern	"C" RtVoid RIPInitRequestEncoding(void);
extern	"C" RtVoid RIPInitPreDefinedItems(void);

void 
initRibGen( liquidRIBStatus * ribStatus )
{
	extern int RiNColorSamples;
	if ( ribStatus->binary ) {
		RipFormat = RIPFORMAT_BINARY;
	}	else { 
		RipFormat = RIPFORMAT_ASCII;
	}
	
	if ( ribStatus->compressed ) {
		RipCompression = RIPCOMPRESSION_GZIP;
	} else {
		RipCompression = RIPCOMPRESSION_NONE;
	}
	
  RipPrecision = RIPPRECISION_DEFAULT;
	
	RIPPushHandleContext();
	RiNColorSamples = 3;

	RIOutputStream = ribStatus->currentOStream;
	RIInputStream = ribStatus->currentIStream;
	if (RIOutputStream != NULL) {
		RIPInitTokenStore();
		RIPInitParamStore();
		RIPInitRequestEncoding();
		RIPInitPreDefinedItems();
	}
}

extern "C" liquidRIBGen    *RIBGenCreate();
extern "C" void       RIBGenDestroy( liquidRIBGen * );
extern "C" void RiFlush( void );

class liquidRIBGen
{
public:
//    virtual int SetArgs( int n, RtToken tokens[], RtPointer values[] ) = 0;
//    virtual void Bound( liquidRIBStatus *, RtBound b ) = 0;
    virtual int GenRIB( liquidRIBStatus * ) = 0;
		int _GenRIB( liquidRIBStatus * ribStatus ) {
			int returnValue = 0;
			/* initRibGen( ribStatus ); */
			returnValue = GenRIB( ribStatus );
			RiFlush();
			return returnValue;
		}
};

#endif
