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
** Contributor(s): Samy Ben Rabah.
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
** Bucket handling Header
** ______________________________________________________________________
*/

#if !defined(__BUCKET_H__)
#define __BUCKET_H__

#include <vector>

typedef struct {
   int channels;
   int width;
   int height;
   int xo, yo;
   int wo,ho;
} imageInfo;

typedef float BUCKETDATATYPE;

class bucket{
	public:

	bucket();
	virtual ~bucket();

	typedef struct bucketInfo{
		unsigned int left;
		unsigned int right;
		unsigned int bottom;
		unsigned int top;
		unsigned int channels;
	}BUCKETINFO;

	void free();

	int set(const bucketInfo &info, const BUCKETDATATYPE *pixels );

	unsigned int getSize() const {return (m_info.right-m_info.left)*(m_info.top-m_info.bottom)*m_info.channels*sizeof(BUCKETDATATYPE);}
	BUCKETDATATYPE *getPixels() const{return m_pixels;}
	const bucketInfo& getInfo() const {return m_info;}

	private:
		BUCKETDATATYPE *m_pixels;
		bucketInfo m_info;
};

#endif
