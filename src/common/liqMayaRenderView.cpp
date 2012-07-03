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
** display server communicating with the liqmaya display driver
** ______________________________________________________________________
*/


#include <maya/MArgDatabase.h>
#include <maya/MGlobal.h>
#include <maya/MGlobal.h>
#include <maya/MRenderView.h>
#include <maya/MSelectionList.h>
#include <maya/MComputation.h>
#include <maya/M3dView.h>
#include <stdio.h>
#include <errno.h>

//#pragma pack(2)
#include "liqMayaRenderView.h"
//#pragma options align=reset



#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
//#include <iostream.h>
//#include <bzlib.h>

#define closesocket close

#else
#include <winsock.h>
#include <io.h>
typedef int socklen_t;
#endif

#include <vector>
#include <string>
#include <iostream>

using namespace std;

int	readSockData(int s,char *data,int n);

std::deque<string> liqMayaRenderCmd::m_lastBucketFiles;

inline int quantize( const float value, const float zero,const float one,const float min, const float max, const float dither );
int waitSocket( const int fd,const int seconds, const bool check_readable = true );

liqMayaRenderCmd::liqMayaRenderCmd()
{
	m_port = 6667;
	m_bLocalhost = true;
	m_bRenderFromFile = false;
	m_camera = "";
	m_quantize[0] = 0.0;
	m_quantize[1] = 255.0;
	m_quantize[2] = 0.0;
	m_quantize[3] = 255.0;
	m_timeout = 50;
	m_bGetRenderRegion = false;
#ifdef _WIN32
	WSADATA wsaData;
	// Init the winsock
	if ( WSAStartup( 0x202, &wsaData ) == SOCKET_ERROR )
	{
		WSACleanup();
		ERROR( "Winsock init error" );
	}
#endif
}

liqMayaRenderCmd::~liqMayaRenderCmd()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

MStatus liqMayaRenderCmd::doIt( const MArgList& args)
{
  MStatus stat = MS::kSuccess;
  MArgDatabase argData(syntax(), args);

	if ( argData.isFlagSet( "-lastRenderFiles" ) )
  {
		MStringArray res;
		setResult(res);
		if ( m_lastBucketFiles.size() > 0 )
			appendToResult( MStringArray((const char**)( &m_lastBucketFiles[0] ), (unsigned int)m_lastBucketFiles.size()));
			
		return MS::kSuccess;
	}
	if ( argData.isFlagSet( "-camera") ) argData.getFlagArgument( "-camera", 0, m_camera );

  m_bDoRegionRender = (argData.isFlagSet( "-doRegion"));

	if ( argData.isFlagSet( "-localhost" ) ) argData.getFlagArgument( "-localhost", 0, m_bLocalhost );
	if ( argData.isFlagSet( "-renderFromFile") ) argData.getFlagArgument( "-renderFromFile", 0, m_bRenderFromFile );
	if ( argData.isFlagSet( "-bucketFile" ) ) argData.getFlagArgument( "-bucketFile", 0, m_bucketFile );
	else		
		if ( m_bRenderFromFile )
    {
			ERROR( "-bucketFile must be set if -renderFromFile is on" );
			return MS::kFailure;
			
		}

	if ( argData.isFlagSet( "-port") ) argData.getFlagArgument( "-port", 0, m_port );
	if ( argData.isFlagSet( "-quantize") )
  {
		for ( int i(0) ; i< 4 ; i++ ) argData.getFlagArgument( "-quantize", i, m_quantize[i] );
	}
	if( argData.isFlagSet( "-timeout") ) argData.getFlagArgument( "-timeout", 0, m_timeout );
	m_bGetRenderRegion = argData.isFlagSet( "-renderRegion" );
  return redoIt();
}

MStatus liqMayaRenderCmd::redoIt()
{
  MStatus retStatus;

	//get the camera
	MDagPath camera;
	if ( m_camera != "" )
  {
		MObject node = getNode( m_camera, &retStatus );
		camera.getAPathTo(node);
	}
	else
	{
		M3dView view = M3dView::active3dView( &retStatus );
		CHECKERR( retStatus, "M3dView::active3dView" );
		retStatus = view.getCamera( camera );
		CHECKERR( retStatus, "M3dView::active3dView" );
	}
	//set the renderview
	retStatus = MRenderView::setCurrentCamera ( camera );
	CHECKERR( retStatus,"MRenderView::setCurrentCamera ( camera )" );
	if ( m_bRenderFromFile )
  {
		vector<bucket*> buckets;
		imageInfo imgInfo;
		retStatus = readBuckets ( m_bucketFile.asChar(), buckets,imgInfo );
		CHECKERR( retStatus, "readBuckets " << m_bucketFile.asChar() );
		MRenderView::startRender ( imgInfo.width, imgInfo.height, false, false );
		for( unsigned int i(0); i< buckets.size(); i++ ) renderBucket ( buckets[i], imgInfo );
	}
	else
	if ( m_bGetRenderRegion )
	{
		unsigned int rg[4];
		MRenderView::getRenderRegion(rg[0], rg[1], rg[2], rg[3]);
		setResult( MIntArray((int*)rg,4) );
		return MS::kSuccess;
	}
	else
  {
		int s ,slaveSocket,status = 0;
		//get the hostname
		int hostlen=32;
		char hostname[32] = "localhost";
		if( !m_bLocalhost )
    {
			status = gethostname(hostname,hostlen);
			CHECKERRNO( status, "[liqMayaRenderView] gethostname(hostname)", );
		}
		//create socket, bound to address and port
		s = createSocket(hostname,m_port);

		if ( s == -1 ) return MS::kFailure;
		//at this stage, I'm ready to receive data from the display driver...

		//check if displayDriver is ready
		if( !waitSocket( s, m_timeout, true ) )
    {
			ERROR("[liqMayaRenderView] timeout reached, display driver didn't respond in time. Aborting");
			closesocket(s);
			return MS::kFailure;
		}

		struct sockaddr_in clientName;
		int clientLength = sizeof(clientName);
		memset( &clientName, 0, sizeof( clientName ) );

		slaveSocket = accept( s,(struct sockaddr *) &clientName,(socklen_t*)(&clientLength));
		if ( -1 == slaveSocket ) 
    {
			perror( "[liqMayaRenderView] accept()" );
			closesocket(s);
			return MS::kFailure;
		}
		int val = 1;
		setsockopt( slaveSocket, IPPROTO_TCP, TCP_NODELAY, (const char *) &val, sizeof( int ) );
		#ifdef SO_NOSIGPIPE
		setsockopt( slaveSocket, SOL_SOCKET, SO_NOSIGPIPE, (const char *) &val, sizeof( int ) );
		#endif

		//get image name
		char imagename[128];
#ifdef _WIN32    
		memset( imagename, 0, 128*sizeof( char ) );
#else
	  bzero( imagename, 128*sizeof( char ) );
#endif    
		// get width/height/num channels
		// imageInfo imgInfo;
		imageInfo imgInfo;
#ifndef _WIN32
//		sleep(1);
#else
//		Sleep(1000);
#endif
		status = readSockData(slaveSocket, (char*)&imgInfo, sizeof(imageInfo));
		if ( -1 == status ) 
    {
			perror( "[liqMayaRenderView] read()" );
			closesocket(s);
			return MS::kFailure;
		}
    
		// printf("[liqMayaRenderView] imgInfo: %d %d %d %d %d %d (%d)\n", imgInfo.width, imgInfo.height, imgInfo.xo, imgInfo.yo, imgInfo.wo, imgInfo.ho, imgInfo.channels ); 

		if ( !m_bDoRegionRender ) 
      MRenderView::startRender ( imgInfo.wo, imgInfo.ho, false, true );
		else 
      MRenderView::startRegionRender ( imgInfo.wo, imgInfo.ho, 
																			 imgInfo.xo, imgInfo.yo, 
																			 imgInfo.xo + imgInfo.width,
																			 imgInfo.yo + imgInfo.height, false, true );
		vector<bucket*> buckets;
		bool bTestEnd;
		MComputation renderComputation;
		renderComputation.beginComputation();

		while ( true ) 
    {
			if ( renderComputation.isInterruptRequested() )
      {
				ERROR( "[liqMayaRenderView] render aborted" );
				break;
			}
			try
      {
				bucket *b = new bucket;
        retStatus = getBucket( slaveSocket, imgInfo.channels, b, bTestEnd );
				if ( retStatus != MS::kSuccess )
        {
					delete b;
					if ( bTestEnd ) break;
					else continue;
				}
				renderBucket( b, imgInfo );
				buckets.push_back( b );
			}
			catch(...)
			{
				ERROR( "[liqMayaRenderView] exception caught" );
				break;
			}
		}
		renderComputation.endComputation();
		closesocket( slaveSocket );
		closesocket( s );

		if ( m_bucketFile == "" )
    {
			char* tmp = getenv( "TEMP" );
			if ( tmp )
      {
				string tmpname( tmp );
				tmpname += "/liqRVXXXXXX";
        if ( mktemp( (char *)tmpname.c_str()) ) m_bucketFile = tmpname.c_str();
			}
		}
		if ( m_bucketFile != "" ) writeBuckets( m_bucketFile.asChar(), buckets, imgInfo );
		 
		// delete the buckets
		for ( unsigned int i(0); i< buckets.size(); i++ ) if ( buckets[ i ] ) delete buckets[ i ];
	  buckets.clear();
	}
	MRenderView::endRender();
  return MS::kSuccess;
}
//read a bucket from the connection, bucket should have been allocated before.
MStatus liqMayaRenderCmd::getBucket( const int socket, 
																		 const unsigned int numChannels,
																		 bucket* b,
																		 bool &theEnd )
{
	int stat;
	MStatus status = MS::kSuccess;
	theEnd = false;
	errno =0;
	if ( !waitSocket( socket, m_timeout, true ) )
  {
		ERROR( "[liqMayaRenderView] timeout reached, aborting" );
		return MS::kFailure;
	}

	//get bucket info + data size
	int bucketInfo[5];

	//stat = read(socket, bucketInfo, 5*sizeof(int));
  stat = readSockData( socket, (char*) bucketInfo, 5 * sizeof( int ) );
  
//	if (stat < 0) {
//		perror("[liqMayaRenderView] recv(slaveSocket)");
//		return MS::kFailure;
//	}
	if ( !stat )
  {
		perror( "[liqMayaRenderView] read(slaveSocket, bucketInfo)" );
		theEnd = true;
		return MS::kFailure;
	}
	bucket::bucketInfo info;
	info.left	= bucketInfo[0];
	info.right	= bucketInfo[1];
	info.bottom	= bucketInfo[2];
	info.top		= bucketInfo[3];
	info.channels = bucketInfo[4];
  
//  printf("[liqMayaRenderView] bucketInfo: %d %d %d %d (%d)\n", info.left, info.right, info.bottom, info.top, info.channels);

#if defined(_WIN32) 
	const unsigned int size = ( info.right - info.left ) * labs( info.bottom - info.top) * 
															numChannels * sizeof( BUCKETDATATYPE );
#else
	#if defined(OSX)	
		long int hi = std::abs( (long int)info.bottom - (long int)info.top ) ;
		const unsigned int size = ( info.right - info.left ) * hi * numChannels * sizeof( BUCKETDATATYPE );  
	#else
	  const unsigned int size = ( info.right - info.left) * labs( (long int)info.bottom - (long int)info.top ) * 
															  numChannels * sizeof( BUCKETDATATYPE );
	#endif
#endif
	
	

#if defined(OSX)  
//	printf("[liqMayaRenderView] sizeof(BUCKETDATATYPE) = %d\n", sizeof(BUCKETDATATYPE) );
//	printf("[liqMayaRenderView] abs(info.bottom-info.top) = %d\n", std::abs( (long int)info.bottom - (long int)info.top ) );
#else
//	printf("[liqMayaRenderView] sizeof(BUCKETDATATYPE) = %d\n", sizeof(BUCKETDATATYPE) );
//	printf("[liqMayaRenderView] labs(info.bottom-info.top) = %d\n", labs( info.bottom - info.top ) );
#endif


//	printf("[liqMayaRenderView] size = %d\n", size );
  
	if ( !size )
  {
		theEnd = true;
		return MS::kFailure;
	}
	//get the data
	BUCKETDATATYPE *data =  new BUCKETDATATYPE[ size ];
	if ( !data ) 
  {
		perror( "[liqMayaRenderView] cannot allocate memory for data" );
		return MS::kInsufficientMemory;
	}
	
  stat = readSockData( socket, (char*)data, size );
	if ( !stat )
  {
		perror( "[liqMayaRenderView] read()" );
		 return MS::kFailure;
	}
	else
	{
		if ( b->set( info, data ) )
    {
			perror( "[liqMayaRenderView] Error b->set(info,data" );
			status = MS::kFailure;
		}
		delete[] data;
	}
	return status;
}

MStatus liqMayaRenderCmd::renderBucket( const bucket* b, const imageInfo &imgInfo )
{
	MStatus status;
	unsigned int nPixels, lineSize, n, x, y;

	// printf("[liqMayaRenderView] renderBucket...\n");
  
  if ( !b ) return MS::kFailure;
	const bucket::bucketInfo &binfo = b->getInfo();
	const unsigned int &left	  = binfo.left;
	const unsigned int &right	  = binfo.right	;
	const unsigned int &bottom	= binfo.bottom;
	const unsigned int &top		  = binfo.top;
	const unsigned int &channels = binfo.channels;

	const BUCKETDATATYPE *data =  b->getPixels();
	
	if ( !data ) return MS::kFailure;
	
	nPixels = ( right - left ) * ( top - bottom );
	RV_PIXEL *pixels = new RV_PIXEL[ nPixels ];
	if ( !pixels ) 
  {
		ERROR( "[liqMayaRenderView] cannot allocate memory for pixels" );
		return  MS::kInsufficientMemory;
	}
#ifdef _WIN32	
  memset( pixels, 0, nPixels * sizeof( RV_PIXEL ) );
#else
	bzero( pixels, nPixels * sizeof( RV_PIXEL ) );
#endif  
	RV_PIXEL* img_ptr = pixels;
	lineSize = right - left;
	pixels = img_ptr + nPixels;
	
	for ( y = bottom, n = 1 ; y < top ;  y++, n++ ) 
  {
		pixels = img_ptr + nPixels - n * lineSize;
		for ( x = left ; x < right ; x++ ) 
    {
			(*pixels).r = quantize( *( data ),   m_quantize[0], m_quantize[1], m_quantize[2], m_quantize[3], 0.5 );
			(*pixels).g = quantize( *( data + 1),m_quantize[0], m_quantize[1], m_quantize[2], m_quantize[3], 0.5 );
			(*pixels).b = quantize( *( data + 2),m_quantize[0], m_quantize[1], m_quantize[2], m_quantize[3], 0.5 );
			
			if( imgInfo.channels > 3 )
				(*pixels).a = quantize( *( data + 3), m_quantize[0], m_quantize[1], m_quantize[2], m_quantize[3], 0.5 );
      data += channels;
      ++pixels;
		}
	}
	pixels = img_ptr;

	const unsigned int &Xo = imgInfo.xo;
	const unsigned int &Yo = imgInfo.yo;
	const unsigned int &height = imgInfo.ho;
	
	MRenderView::updatePixels ( Xo + left , Xo + right - 1, height -Yo -top, height - bottom - Yo - 1 , pixels );
	MRenderView::refresh ( Xo + left, Xo + right - 1, height - Yo - top, height - Yo -bottom - 1 );

	delete[] pixels;

	return status;
}

MStatus liqMayaRenderCmd::undoIt()
{
  return MS::kSuccess;
}

void* liqMayaRenderCmd::creator()
{
  return new liqMayaRenderCmd();
}

bool liqMayaRenderCmd::isUndoable() const
{
  return false;
}

MSyntax liqMayaRenderCmd::newSyntax()
{
  MSyntax syntax;
  syntax.enableQuery( true );
	syntax.addFlag( "-c","-camera", MSyntax::kString );
	syntax.addFlag( "-p", "-port", MSyntax::kLong );
	syntax.addFlag( "-l", "-localhost", MSyntax::kBoolean );
	syntax.addFlag( "-qz", "-quantize", MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble, MSyntax::kDouble );
	syntax.addFlag( "-t", "-timeout", MSyntax::kLong );
	syntax.addFlag( "-rff", "-renderFromFile", MSyntax::kBoolean );
	syntax.addFlag( "-bf", "-bucketFile", MSyntax::kString );
	syntax.addFlag( "-lr", "-lastRenderFiles");
	syntax.addFlag( "-rg", "-renderRegion");
	syntax.addFlag( "-drg", "-doRegion");
  syntax.useSelectionAsDefault( false );
  return syntax;
}


int liqMayaRenderCmd::createSocket( const char *hostname, const unsigned int port ) 
{
	int backlog = 5;
	int serverSocket = 0, on = 1, status = 0;
	struct hostent *hostPtr = NULL;
	struct sockaddr_in serverName;

	serverSocket = socket(PF_INET, SOCK_STREAM,IPPROTO_TCP);
	if ( -1 == serverSocket ) 
  {
		perror( "socket()" );
		return -1;
	}

	status = setsockopt( serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof( on ) );
	if ( -1 == status ) 
    perror( "[liqMayaRenderCmd] setsockopt(...,SO_REUSEADDR,...)" );
//	status = fcntl(serverSocket,F_SETFL, O_NONBLOCK);
//	if (-1 == status) perror("[liqMayaRenderCmd] fcntl(serverSocket, O_NONBLOCK)");

	linger lng;
	lng.l_onoff = 1;
	lng.l_linger = 30;
	
	status = setsockopt( serverSocket, SOL_SOCKET, SO_LINGER, (const char *) &lng, sizeof( linger ) );
	if ( -1 == status ) 
    perror( "[liqMayaRenderCmd] setsockopt(...,SO_LINGER,...)" );

	hostPtr = gethostbyname( hostname );
	if ( NULL == hostPtr ) 
  {
		hostPtr = gethostbyaddr( hostname, strlen( hostname ), AF_INET );
		if ( NULL == hostPtr ) 
    {
			perror( "[liqMayaRenderCmd] Cannot resolve server address" );
			return -1;
		}
	}

	(void)memset( &serverName, 0, sizeof( serverName ) );
	(void)memcpy( &serverName.sin_addr, hostPtr->h_addr, hostPtr->h_length );

	serverName.sin_family = AF_INET;
	/* network-order */
	serverName.sin_port = htons( port );

	status = bind( serverSocket, ( struct sockaddr *) &serverName, sizeof( serverName ) );
	if (-1 == status ) 
  {
		perror( "[liqMayaRenderCmd] bind()" );
		return -1;
	}

	status = listen( serverSocket, backlog );
	if ( -1 == status ) 
  {
		perror( "[liqMayaRenderCmd] listen()" );
		return -1;
	}

	return serverSocket;
}

MObject getNode( MString name, MStatus *returnStatus )
{
	MObject node;
	MSelectionList list;

	*returnStatus=MGlobal::getSelectionListByName( name, list );

	if ( MS::kSuccess != *returnStatus )
  {
		ERROR( "[liqMayaRenderView] Cound't get node :" + name + ". There might be multiple nodes called " + name );
		return node;
	}

	*returnStatus = list.getDependNode( 0, node );

	if ( MS::kSuccess != *returnStatus )
  {
		ERROR( "[liqMayaRenderView] Cound't get node :" + name +". There might be multiple nodes called " + name );
		return MObject::kNullObj;
	}
	return node;
}


MStatus liqMayaRenderCmd::writeBuckets( const char* file, const vector<bucket*> &buckets, const imageInfo &info ) const
{
	MStatus status;
	unsigned int i;
	FILE *fh = fopen ( file, "wb" );
	if( !fh )
	{
		ERROR( "[liqMayaRenderCmd] couldn't open " + file + " for writing" );
		return MS::kFailure;
	}
	fwrite ( &info, sizeof( imageInfo ), 1, fh );
	if ( ferror( fh ) )
  { 
    ERROR( "[liqMayaRenderCmd] error writing imageInfo" ); 
    fclose ( fh ); 
    return MS::kFailure;
  }

	unsigned int numBuckets = buckets.size();
	fwrite ( &numBuckets, sizeof( unsigned int ), 1, fh );
	if ( ferror(fh) )
  { 
    fclose ( fh ); 
    return MS::kFailure;
  }

	for(i = 0 ; i< buckets.size() ; i++ )
  {
		const bucket *b = buckets[i];
		if ( !b ) continue;
		fwrite ( &b->getInfo(), sizeof( bucket::bucketInfo ), 1, fh );
		if ( ferror( fh ) )
		{
			ERROR( "[liqMayaRenderCmd] error writing bucketInfo" );
			status = MS::kFailure;
			break;
		}
		unsigned int size = b->getSize();
		fwrite ( b->getPixels(), size, 1, fh );
		if ( ferror( fh ) )
		{
			status = MS::kFailure;
			break;
		}
	}
	if ( m_lastBucketFiles.size() == 15 ) //remember the last 15 files.
		m_lastBucketFiles.pop_front();
	m_lastBucketFiles.push_back( file );

	fclose ( fh );
	return MS::kSuccess;
}

MStatus liqMayaRenderCmd::readBuckets( const char* file, vector<bucket*> &buckets, imageInfo &imgInfo ) const
{
	MStatus status;
	
	FILE *fh = fopen ( file,"rb" );
	if ( !fh ) 
  {
		ERROR( "[liqMayaRenderCmd] Error: couldn't open " + file + " for reading" );
		return MS::kFailure;
	}
	fread ( &imgInfo, sizeof( imageInfo ), 1, fh );
	unsigned int size;
	fread ( &size, sizeof( unsigned int ), 1, fh );
	if ( ferror( fh ) )
  {
		ERROR( "[liqMayaRenderCmd] Error: failed reading data from " + file );
		fclose ( fh );
		return MS::kFailure;
	}
	buckets.clear();
	for ( unsigned int i(0) ; i < size ; i++ )
  {
		bucket::bucketInfo info;
		
		fread ( &info, sizeof( bucket::bucketInfo ), 1, fh );
		if ( ferror( fh ) ) 
    {
			ERROR( "[liqMayaRenderCmd] Error: failed reading data from " + file );
			status = MS::kFailure;
			break;
		}
		unsigned int datasize = ( info.right - info.left ) * ( info.top - info.bottom ) * imgInfo.channels * sizeof( BUCKETDATATYPE );
		BUCKETDATATYPE *data = new BUCKETDATATYPE[ datasize ];
		if ( !data )
    {
			ERROR( "[liqMayaRenderCmd] Error: failed reading data from " + file );
			status = MS::kInsufficientMemory;
			break;
		}
		fread ( data, datasize, 1, fh );
		if ( ferror( fh ) ) 
    {
			ERROR( "[liqMayaRenderCmd] Error: failed reading data from " + file );
			status = MS::kFailure;
			break;
		}
		bucket* b = new bucket;
		if ( !b->set( info, data ) ) buckets.push_back( b );
		delete[] data;
	}
	fclose ( fh );
	return status;
}

inline int quantize( const float value, const float zero,const float one,const float min, const float max, const float dither )
{
	int result = ( int )( zero + value * ( one - zero ) + dither );
	if ( result < min ) result = ( int )min;
	if ( result > max ) result = ( int )max;
	return result;
}

int waitSocket ( const int fd, const int seconds, const bool check_readable )
{
  fd_set fds;
  struct timeval tv;
  FD_ZERO( &fds );
  FD_SET( fd, &fds );
  tv.tv_sec = seconds;
	tv.tv_usec = 0;

	fd_set *fds_r = &fds;
	fd_set *fds_w = NULL;
	if ( !check_readable )
  {
		fds_r = NULL;
		fds_w = &fds;
	}
  int rc = select( fd + 1, fds_r, fds_w, NULL, &tv );
  if ( rc < 0 ) return -1;

  return FD_ISSET( fd, &fds ) ? 1 : 0;
}

int readSockData ( int s, char *data, int n ) 
{
	int	i,j;

	j	= n;
	#ifdef MSG_NOSIGNAL
		i	= recv ( s, data, j, MSG_NOSIGNAL );
	#else
		i	= recv ( s, data, j, 0 );
	#endif

	if ( i <= 0 ) 
  {
		perror( "[liqMayaRenderCmd] Connection broken (1)" );
		return false;
	}

	while ( i < j ) 
  {
		data	+=	i;
		j	-=	i;

		#ifdef MSG_NOSIGNAL
			i		=	recv ( s, data, j, MSG_NOSIGNAL );
		#else
			i		=	recv ( s, data, j, 0 );
		#endif

		if (i <= 0 ) 
    {
			perror( "[liqMayaRenderCmd] Connection broken (2)" );
			return false;
		}
	}
	return true;
}

