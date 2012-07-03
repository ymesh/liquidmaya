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


#if !defined(__D_LIQMAYA_H__)
#define __D_LIQMAYA_H__

#include <stdlib.h>

#ifdef WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif



typedef void *PtDspyImageHandle;

#define PkDspyNone			 0
#define PkDspyFloat32    1
#define PkDspyUnsigned32 2
#define PkDspySigned32   3
#define PkDspyUnsigned16 4
#define PkDspySigned16   5
#define PkDspyUnsigned8  6
#define PkDspySigned8    7

typedef enum PtDspyError
{
  PkDspyErrorNone         = 0,  // Successful completion
  PkDspyErrorNoMemory     = 1,  // Unable to allocate memory
  PkDspyErrorUnsupported  = 2,  // Unsupported operation requested
  PkDspyErrorBadParams    = 3,  // Bad parameters
  PkDspyErrorNoResource   = 4,  // No resource available, file not found, etc.
  PkDspyErrorUndefined    = 5  // No other error messages appropriate
} PtDspyError;

typedef enum PtDspyQueryType
{
  PkOverwriteQuery  = 0,
  PkSizeQuery       = 1,
} PtDspyQueryType;


typedef struct PtDspyOverwriteInfo
{
  int   overwrite;
  int   interactive;
} PtDspyOverwriteInfo;

typedef struct PtDspySizeInfo
{
  int   Width;
  int   Height;
  float AspectRatio;
} PtDspySizeInfo;

typedef struct UserParameter
{
  char *name;
  unsigned char ptype, nelements; //,  pad0, pad1;
  void *data;
  int size;
} UserParameter;

typedef struct PtDspyDevFormat
{
  char * name;
  unsigned type;
} PtDspyDevFormat;

typedef enum PtFlagStuff
{
  PkDspyFlagsWantsScanlineOrder    = 0x0001,
  PkDspyFlagsWantsEmptyBuckets     = 0x0002,
  PkDspyFlagsWantsNullEmptyBuckets = 0x0004,
  PkDspyFlagsWantsPrepass = 0x10000
} PtFlagStuff;

#define PkFloat 102
#define PkInt 105
#define PkString 115


EXPORT  PtDspyError DspyImageOpen(PtDspyImageHandle    *image,
                                     const char           *drivername,
                                     const char           *filename,
                                     int                  width,
                                     int                  height,
                                     int                  paramCount,
                                     const UserParameter  *parameters,
                                     int                  formatCount,
                                     PtDspyDevFormat      *format,
                                     PtFlagStuff          *flagstuff);

EXPORT PtDspyError DspyImageQuery(PtDspyImageHandle image,
                                      PtDspyQueryType   type,
                                      size_t            size,
                                      void              *p);

EXPORT PtDspyError DspyImageClose(PtDspyImageHandle image);

EXPORT PtDspyError DspyImageData(PtDspyImageHandle image,
                                     int xmin,
                                     int xmax_plusone,
                                     int ymin,
                                     int ymax_plusone,
                                     int entrysize,
                                     const unsigned char *data);

EXPORT PtDspyError DspyImageDelayClose(PtDspyImageHandle image);



#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#else
#include <winsock.h>
#include <io.h>
#endif
#include <stdlib.h>
#include <string.h>

//#pragma pack(2)
#include "liqBucket.h"

int openSocket(const char *host, const int port) ;
PtDspyError sendData(const int socket,
			 const int xmin,
			 const int xmax_plusone,
			 const int ymin,
			 const int ymax_plusone,
			 const int entrysize,
			 const int channels,
			 const BUCKETDATATYPE *data) ;


//check if socket is ready for a connection
int waitSocket(const int fd,const int seconds, const bool check_readable = true)
{
  fd_set fds;
  struct timeval tv;
  FD_ZERO(&fds);
  FD_SET(fd,&fds);
  tv.tv_sec = seconds;
	tv.tv_usec = 0;

	fd_set *fds_r = &fds;
	fd_set *fds_w = NULL;
	if(!check_readable)
  {
		fds_r = NULL;
		fds_w = &fds;
	}
  int rc = select(fd+1, fds_r,fds_w, NULL, &tv);
  if (rc < 0)
    return -1;

  return FD_ISSET(fd,&fds) ? 1 : 0;
}

//#pragma options align=reset

#endif        //  #if !defined(__D_LIQMAYA_H__)

