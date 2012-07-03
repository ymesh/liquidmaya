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
** Contributor(s): Alf Kraus.
**
**
** The RenderMan (R) Interface Procedures and Protocol are:
** Copyright 1988, 1989, Pixar
** All Rights Reserved
**
**
** RenderMan (R) is a registered trademark of Pixar
**
**
** ______________________________________________________________________
**
** Liquid Ri Mel commands
**
** ______________________________________________________________________
** 
*/

#include <maya/MDGModifier.h>
#include <maya/MPxCommand.h>
#include <maya/MFloatArray.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>


class RIArchiveBegin : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIArchiveBegin; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIArchiveEnd : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIArchiveEnd; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIArchiveRecord : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIArchiveRecord; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIAtmosphere : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIAtmosphere; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIAttribute : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIAttribute; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIAttributeBegin : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIAttributeBegin; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIAttributeEnd : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIAttributeEnd; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIClipping : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIClipping; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIColor : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIColor; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIConcatTransform : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIConcatTransform; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RICropWindow : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RICropWindow; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIDepthOfField : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIDepthOfField; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIDetail : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIDetail; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIDetailRange : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIDetailRange; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIDisplacement : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIDisplacement; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIDisplay : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIDisplay; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIDisplayChannel : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIDisplayChannel; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIExterior : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIExterior; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIFormat : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIFormat; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIFrameBegin : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIFrameBegin; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIFrameEnd : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIFrameEnd; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIGeometricApproximation : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIGeometricApproximation; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIGeometry : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIGeometry; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIHider : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIHider; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIIdentity : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIIdentity; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIIlluminate : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIIlluminate; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIInterior : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIInterior; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RILightSource : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RILightSource; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIMatte : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIMatte; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIMotionBegin : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIMotionBegin; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIMotionEnd : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIMotionEnd; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIObjectBegin : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIObjectBegin; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIObjectEnd : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIObjectEnd; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIOpacity : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIOpacity; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIOption : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIOption; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIOrientation : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIOrientation; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIPixelSamples : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIPixelSamples; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIProcedural : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIProcedural; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIProjection : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIProjection; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIReadArchive : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIReadArchive; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIRelativeDetail : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIRelativeDetail; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIResource : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIResource; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIResourceBegin : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIResourceBegin; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIResourceEnd : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIResourceEnd; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIReverseOrientation : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIReverseOrientation; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIRotate : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIRotate; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIScale : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIScale; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIScreenWindow : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIScreenWindow; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIShadingInterpolation : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIShadingInterpolation; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIShadingRate : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIShadingRate; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIShutter : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIShutter; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RISides : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RISides; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RISkew : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RISkew; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RISolidBegin : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RISolidBegin; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RISolidEnd : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RISolidEnd; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RISphere : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RISphere; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RISurface : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RISurface; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RITranslate : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RITranslate; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RITransformBegin : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RITransformBegin; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RITransformEnd : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RITransformEnd; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIWorldBegin : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIWorldBegin; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};

class RIWorldEnd : public MPxCommand {
public: 
	virtual MStatus	doIt( const MArgList &args );
	virtual MStatus undoIt();
 	virtual MStatus redoIt();
	virtual bool isUndoable() const { return true; } 
	static void*	creator() { return new RIWorldEnd; }
 	static MSyntax	newSyntax();
private:
	MDGModifier dgMod;
};
