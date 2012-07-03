
#ifndef liqRibboxNode_H
#define liqRibboxNode_H

#include <liquid.h>


#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MImage.h>
#include <maya/MSwatchRenderBase.h>
#include <maya/MFnDependencyNode.h>

#include <liqNodeSwatch.h>




class liqRibboxNode : public MPxNode
{
	public:
                      liqRibboxNode();
    virtual          ~liqRibboxNode();

    //virtual MStatus   compute( const MPlug&, MDataBlock& );
    virtual void      postConstructor();

    static  void *    creator();
    static  MStatus   initialize();

    //  Id tag for use with binary file format
    static  MTypeId   id;
    liqNodeSwatch*    renderSwatch;

	private:

    // Input attributes
    static MObject aRmanShader;
    static MObject aRmanShaderLong;
    static MObject aRibbox;

    // Output attributes
    static MObject aOutColor;

    bool    swatchInit;
};




#endif
