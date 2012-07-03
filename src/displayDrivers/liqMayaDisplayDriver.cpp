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


#include "liqMayaDisplayDriver.h"
#include <errno.h>

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;
using namespace std;
#define HERE  cout<<"at "<<__LINE__<<" in "<<__FUNCTION__<<endl;
#define INFO(EXPR,ENDL) cout<<#EXPR<<" "<<EXPR<<" ";if(ENDL)cout<<endl;
int timeout = 30;
static int recoverFlag=0;
static int socketId = -1;

int sendSockData(int s,char * data,int n);


#ifndef _WIN32
#define closesocket close
#endif


PtDspyError
DspyImageOpen(PtDspyImageHandle *pvImage,
              const char *drivername,
              const char *filename,
              int width,
              int height,
              int paramCount,
              const UserParameter *parameters,
              int formatCount,
              PtDspyDevFormat *format,
              PtFlagStuff *flagstuff) {
	int i,origin[2],originalSize[2],rc;

	int status =0;
	int port = 6667;

	if (0 == width)
		width = 640;
	if (0 == height)
		height = 480;

	//format can be rgb, rgba, rgbaz or rgbz
	char* rgba[5] = {"r","g","b","a","z"};


#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(0x202,&wsaData) == SOCKET_ERROR) 
	{
		WSACleanup();
		return PkDspyErrorNoResource;
	}
#endif

	PtDspyDevFormat *outformat = new PtDspyDevFormat[formatCount];
	PtDspyDevFormat *f_ptr = outformat;
	for(i=0;i<formatCount;i++,++f_ptr)
  {
		f_ptr->type = PkDspyFloat32;
		f_ptr->name = rgba[i];
	}

	DspyReorderFormatting(formatCount,format,formatCount,outformat);

	char hostname[32] = "localhost", *h;
	if(PkDspyErrorNone==DspyFindStringInParamList("host",&h,paramCount,parameters))
		strcpy(hostname,h);
	
	if(PkDspyErrorNone!=DspyFindIntsInParamList("origin",&rc,origin,paramCount,parameters)) 
  {
		origin[0]= 0;
		origin[1]= 0;
	}
	
  if(PkDspyErrorNone!=DspyFindIntsInParamList("OriginalSize",&rc,originalSize,paramCount,parameters)) 
  {
		originalSize[0]= width;
		originalSize[1]= height;
	}

	if(PkDspyErrorNone!=DspyFindIntInParamList("mayaDisplayPort",&port,paramCount,parameters)) 
		port = 6667;
	
	if(PkDspyErrorNone!=DspyFindIntInParamList("timeout",&timeout,paramCount,parameters)) 
		timeout = 30;
	
	imageInfo *imgSpecs = new imageInfo;
	imgSpecs->channels = formatCount;
	imgSpecs->width      = width;
	imgSpecs->height      = height;
	imgSpecs->xo = origin[0];
	imgSpecs->yo = origin[1];
	imgSpecs->wo = originalSize[0];
	imgSpecs->ho = originalSize[1];

	*pvImage = imgSpecs;
	socketId = openSocket(hostname, port);
	if(socketId == -1)
	{
		#ifdef _WIN32
			WSACleanup();
		#endif
		delete imgSpecs;
		return PkDspyErrorNoResource;
	}

	if(!waitSocket(socketId,timeout,false))	
	{
		#ifdef _WIN32
			WSACleanup();
		#endif
		cerr<<"[d_liqmaya] Error: timeout"<<endl;
		delete imgSpecs;
		return PkDspyErrorNoResource;
	}
	status = sendSockData(socketId,(char*)imgSpecs,sizeof(imageInfo));

	if(status == false){
		#ifdef _WIN32
			WSACleanup();
		#endif
		perror("[d_liqmaya] Error: write(socketId,wh,2*sizeof(int))");
		delete imgSpecs;
		return PkDspyErrorNoResource;
	}
	return PkDspyErrorNone;
}


PtDspyError
DspyImageQuery(PtDspyImageHandle pvImage,
               PtDspyQueryType querytype,
               int datalen,
               void *data) {
	if ((datalen == 0) || (NULL == data))
		return PkDspyErrorBadParams;


	switch (querytype) {
	case PkOverwriteQuery: {
			PtDspyOverwriteInfo overwriteInfo;

			if (datalen > (int)sizeof(overwriteInfo))
				datalen = sizeof(overwriteInfo);
			overwriteInfo.overwrite = 1;
			overwriteInfo.interactive = 0;
			memcpy(data, &overwriteInfo, datalen);
			break;
		}

	case PkSizeQuery: {
			PtDspySizeInfo sizeInfo;

			if (datalen > (int)sizeof(sizeInfo))
				datalen = sizeof(sizeInfo);

			memcpy(data, &sizeInfo, datalen);
			break;
		}

	case PkSupportsCheckpointing: {
			PtDspySupportsCheckpointing chkpt;
			if (datalen > (int)sizeof(chkpt))
				datalen = sizeof(chkpt);
			//
			// checkpointing
			chkpt.canCheckpoint=1;
			recoverFlag=1;

			memcpy(data, &chkpt, datalen);

			break;
		}

	case PkRenderingStartQuery: {
			PtDspyRenderingStartQuery rsq;
			/* where can we recover to? */
			if (datalen > (int)sizeof(rsq))
				datalen = sizeof(rsq);

			rsq.x=0;
			// rsq.y=image->recoverLine;
			memcpy(data, &rsq, datalen);
			break;
		}

	default :
		return PkDspyErrorUnsupported;
	}

	return PkDspyErrorNone;
}



PtDspyError DspyImageData(PtDspyImageHandle pvImage,
                          int xmin,
                          int xmax_plusone,
                          int ymin,
                          int ymax_plusone,
                          int entrysize,
                          const unsigned char *data) {

	// if(recoverFlag && (ymin<image->recoverLine))return PkDspyErrorNone;

	imageInfo *spec = (imageInfo*)pvImage;
	PtDspyError status = sendData(socketId,xmin,xmax_plusone,ymin,ymax_plusone,entrysize,spec->channels,(BUCKETDATATYPE*)data);
	if(status != PkDspyErrorNone)
		DspyImageClose(pvImage);
	return status;

}


PtDspyError DspyImageClose(PtDspyImageHandle pvImage) {
	bucket::bucketInfo binfo;
#ifdef _WIN32
	memset(&binfo,0,sizeof(bucket::bucketInfo));
#else
	bzero(&binfo,sizeof(bucket::bucketInfo));
#endif
    sendSockData(socketId, (char*) &binfo,sizeof(bucket::bucketInfo));

	closesocket(socketId);
#ifdef _WIN32
	WSACleanup();
#endif

	return PkDspyErrorNone;
}


PtDspyError sendData(const int socket,
			 const int xmin,
			 const int xmax_plusone,
			 const int ymin,
			 const int ymax_plusone,
			 const int entrysize,
			 const int numChannels,
			 const BUCKETDATATYPE *data) {
	int status =0;
	int size = (xmax_plusone-xmin)*(ymax_plusone-ymin)*numChannels*sizeof(BUCKETDATATYPE);
	bucket::bucketInfo binfo;
	binfo.left      = xmin;
	binfo.right     = xmax_plusone;
	binfo.bottom    = ymin;
	binfo.top       = ymax_plusone;
	binfo.channels  = numChannels;

	if(!waitSocket(socket,timeout,false)){
		cerr<<"[d_liqmaya] Error: timeout reached, data cannot be sent"<<endl;
		return PkDspyErrorUndefined;
	}

	status = sendSockData(socket, (char*)&binfo,sizeof(bucket::bucketInfo));
	if(status == false){
		perror("[d_liqmaya] Error: write(socket,bucketInfo)");
		return PkDspyErrorNoResource;
	}
	if(!waitSocket(socket,timeout,false)){
		cerr<<"[d_liqmaya] Error: timeout reached, data cannot be sent"<<endl;
		return PkDspyErrorUndefined;
	}
	status = sendSockData(socket, (char*)data,size);
	if(status == false){
		perror("[d_liqmaya] Error: write(socket,data)");
		return PkDspyErrorNoResource;
	}
	return PkDspyErrorNone;

}


int openSocket(const char *host, const int port)
{
	struct hostent *hostPtr = NULL;
	struct sockaddr_in serverName;
	int clientSocket = socket(PF_INET, SOCK_STREAM,IPPROTO_TCP);
	if (-1 == clientSocket) {
		perror("[d_liqmaya] Error: socket()");
		return -1;
	}

	hostPtr = gethostbyname(host);
	if (NULL == hostPtr) {

		hostPtr = gethostbyaddr(host,strlen(host), AF_INET);
		if (NULL == hostPtr) {
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
    if(status != -1) 
      break; 
#ifndef _WIN32
		sleep(1);
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
  int valLen = sizeof(int);
  status = setsockopt(clientSocket,IPPROTO_TCP,TCP_NODELAY,(const char *) &val,sizeof(int));
  
  #ifdef SO_NOSIGPIPE
    val = 1;
    setsockopt(clientSocket,SOL_SOCKET,SO_NOSIGPIPE,(const char *) &val,sizeof(int));
  #endif

	return clientSocket;

}

int sendSockData(int s,char * data,int n){
	int i,j;

	j	= n;
	i	= send(s,data,j,0);
	if (i <= 0) {
		perror("[d_liqmaya] Connection broken");
		return false;
	}

	// If we could not send the entire data, send the rest
	while(i < j) 
  {
		data	+=	i;
		j		-=	i;
		i		= send(s,data,j,0);
		if (i <= 0) 
    {
			perror("[d_liqmaya] Connection broken (sending rest)");
			return false;
		}
	}
	return true;
}
