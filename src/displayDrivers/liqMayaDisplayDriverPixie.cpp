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
** Liquid display driver for the maya render view
** ______________________________________________________________________
*/

#define PtDspyError int
#define PkDspyErrorNone 0

#include "liqMayaDisplayDriverPixie.h"
#include <errno.h>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
using namespace std;
#define HERE  cout<<"at "<<__LINE__<<" in "<<__FUNCTION__<<endl;
#define INFO(EXPR,ENDL) cout<<#EXPR<<" "<<EXPR<<" ";if(ENDL)cout<<endl;
static int timeout = 30;
static int recoverFlag=0;
static int socketId = -1;

int sendSockData(int s,char * data,int n);

#ifndef _WIN32
#define closesocket close
#endif

#ifndef INTEGER_PARAMETER
#define INTEGER_PARAMETER FLOAT_PARAMETER
#endif


  
void	*displayStart( const char *name,int width,int height,int numSamples,const char *samples,TDisplayParameterFunction findParameter) 
{
	int i,rc;
  int origin[2];
  int originalSize[2];
	int status = 0;
	int port = 6667;
	
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR) 
	{
		WSACleanup();
		return NULL;
	}
#endif

	if (0 == width)
		width = 640;
	if (0 == height)
		height = 480;
	
	char hostname[32] = "localhost", *h;
	if(char *hostParam = (char *) findParameter("host",STRING_PARAMETER,1))
  {
		strcpy(hostname,hostParam);
	}

	if(int *originParam = (int*) findParameter("origin",INTEGER_PARAMETER,2))
  {
		origin[0]= originParam[0];
		origin[1]= originParam[0];
	} else {
		origin[0]= 0;
		origin[1]= 0;
	}
	
/*	if(PkDspyErrorNone!=DspyFindIntsInParamList("OriginalSize",&rc,originalSize,paramCount,parameters)) {
		originalSize[0]= width;
		originalSize[0]= height;
	}
*/

	if(int *portParam = (int *) findParameter("mayaDisplayPort",INTEGER_PARAMETER,1))
  {
		port = portParam[0];
	} else {
		port = 6667;
	}
	
/*	if(PkDspyErrorNone!=DspyFindIntInParamList("timeout",&timeout,paramCount,parameters)) {
		timeout = 30;
	}
*/

	imageInfo *imgSpecs = new imageInfo;
	imgSpecs->channels = numSamples;
	imgSpecs->width      = width;
	imgSpecs->height      = height;
	imgSpecs->xo = origin[0];
	imgSpecs->yo = origin[1];
	imgSpecs->wo = width;//originalSize[0];
	imgSpecs->ho = height;//originalSize[1];

	socketId = openSocket(hostname, port);
  printf("[d_liqmaya] openSocket = %d host = %s port = %d\n", socketId, hostname, port);
	if(socketId == -1)
  {
		#ifdef _WIN32
			WSACleanup();
		#endif

		delete imgSpecs;
		return NULL;//PkDspyErrorNoResource;
	}

	if(!waitSocket(socketId,timeout,false))	
  {
		#ifdef _WIN32
			WSACleanup();
		#endif

		cerr<<"[d_liqmaya] Error: timeout"<<endl;
		delete imgSpecs;
		return NULL;//PkDspyErrorNoResource;
	}
	status = sendSockData(socketId,(char*)imgSpecs,sizeof(imageInfo));
	if(!status)
  {
		#ifdef _WIN32
			WSACleanup();
		#endif

		perror("[d_liqmaya] Error: write(socketId,wh,2*sizeof(int))");
		delete imgSpecs;
		return NULL;//PkDspyErrorNoResource;
	}

	return (void*)imgSpecs;//PkDspyErrorNone;
}

int	displayData(void *im,int x,int y,int w,int h,float *data) 
{
	imageInfo *spec = (imageInfo*)im;
	PtDspyError status = sendData(socketId,x,x+w,y,y+h,spec->channels*sizeof(float),spec->channels,(BUCKETDATATYPE*)data);
	if(!status)
  {
		displayFinish(im);
	}
	return status;

}

int sendSockData(int s,char * data,int n)
{
	int i,j;
	
	j	= n;
	#ifdef MSG_NOSIGNAL
	i	= send(s,data,j,MSG_NOSIGNAL);
	#else
	i	= send(s,data,j,0);
	#endif
  
  printf("[d_liqmaya] s = %d n = %d i = %d\n", s, n, i);
  
	if (i <= 0) 
  {
		perror("[d_liqmaya] Connection broken");
		return false;
	}

	// If we could not send the entire data, send the rest
	while(i < j) 
  {
		data	+=	i;
		j		-=	i;

		#ifdef MSG_NOSIGNAL
		i	= send(s,data,j,MSG_NOSIGNAL);
		#else
		i	= send(s,data,j,0);
		#endif
		
		if (i <= 0) 
    {
			perror("[d_liqmaya] Connection broken (sending rest)");
			return false;
		}
	}
	return true;
}

void	displayFinish(void *im) 
{
	bucket::bucketInfo binfo;
#ifdef _WIN32
	memset(&binfo,0,sizeof(bucket::bucketInfo));
#else
	bzero(&binfo,sizeof(bucket::bucketInfo));
#endif
	sendSockData(socketId, (char*) &binfo,sizeof(bucket::bucketInfo));

#ifdef _WIN32
	WSACleanup();
#endif
	closesocket(socketId);
}


PtDspyError sendData(const int socket,
			 const int xmin,
			 const int xmax_plusone,
			 const int ymin,
			 const int ymax_plusone,
			 const int entrysize,
			 const int numChannels,
			 const BUCKETDATATYPE *data) 
{
	int status =0;
	int size = (xmax_plusone-xmin)*(ymax_plusone-ymin)*numChannels*sizeof(BUCKETDATATYPE);
	bucket::bucketInfo binfo;
	binfo.left      = xmin;
	binfo.right     = xmax_plusone;
	binfo.bottom    = ymin;
	binfo.top       = ymax_plusone;
	binfo.channels  = numChannels;
  
  printf( "[d_liqmaya] bucketInfo: %d %d %d %d (%d)\n", binfo.left,binfo.right,binfo.bottom,binfo.top, binfo.channels ); 
  printf( "[d_liqmaya] send bucket size = %d \n", size ); 
  

	if(!waitSocket(socket,timeout,false))
  {
		cerr<<"[d_liqmaya] Error: timeout reached, data cannot be sent"<<endl;
		return false;
	}

	status = sendSockData(socket, (char*)&binfo,sizeof(bucket::bucketInfo));
	if(!status)
  {
		perror("[d_liqmaya] Error: write(socket,bucketInfo)");
		return false;
	}
	
	if(!waitSocket(socket,timeout,false))
  {
		cerr<<"[d_liqmaya] Error: timeout reached, data cannot be sent"<<endl;
		return false;
	}
	status = sendSockData(socket, (char*)data,size);
	if(!status)
  {
		perror("[d_liqmaya] Error: write(socket,data)");
		return false;
	}
	return true;

}


int openSocket(const char *host, const int port)
{

	struct hostent *hostPtr = NULL;
	struct sockaddr_in serverName;
	int clientSocket = socket(PF_INET, SOCK_STREAM,IPPROTO_TCP);
	if (-1 == clientSocket) 
  {
		perror("[d_liqmaya] Error: socket()");
		return -1;
	}

	hostPtr = gethostbyname(host);
	if (NULL == hostPtr) {

		hostPtr = gethostbyaddr(host,strlen(host), AF_INET);
		if (NULL == hostPtr) 
    {
			perror("[d_liqmaya] Error: resolving server address");
			return -1;
		}
	}

	serverName.sin_family = AF_INET;
	serverName.sin_port = htons(port);
	memcpy(&serverName.sin_addr,hostPtr->h_addr,hostPtr->h_length);
	errno = 0;
  int status;
  int counter = 10;
  
  while ( counter-- )
  {  
	  status = connect(clientSocket,(struct sockaddr*) &serverName,sizeof(serverName));
    if ( status != -1 ) 
      break; 
#ifndef _WIN32
		sleep(2);
#else
  	Sleep(1000);
#endif 	
	}
  
  if (-1 == status)
  {
    perror("[d_liqmaya] Error: connect()");
    return -1;
  }
   
  int val = 1;
  #ifdef SO_NOSIGPIPE
	setsockopt(clientSocket,SOL_SOCKET,SO_NOSIGPIPE,(const char *) &val,sizeof(int));
	#endif
	val = 1;
	setsockopt(clientSocket,IPPROTO_TCP,TCP_NODELAY,(const char *) &val,sizeof(int));
	
	return clientSocket;
}


