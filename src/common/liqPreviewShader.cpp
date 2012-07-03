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

/* liquid command to export a shader ball with the selected shader */

// Renderman headers
extern "C" {
#include <ri.h>
}
#ifdef _WIN32
  #include <process.h>
#else
  #include <sys/wait.h>
#endif

// Maya headers
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MPlug.h>
#include <maya/MFnMesh.h>

#include <liqRenderer.h>
#include <liqProcessLauncher.h>
#include <liqPreviewShader.h>
#include <liqGlobalHelpers.h>
#include <liqShaderFactory.h>

// Standard/Boost headers
#include <boost/scoped_array.hpp>

extern int debugMode;
//#ifndef DELIGHT
extern liqRenderer liquidRenderer;
//#endif

#ifdef DELIGHT
static bool rendering( false );
/* Callback function for 3Delight. We don't write RIB but render directly in 3Delight.
 * However, we can't display the resulting image until the renderer is finished.
 * So we use 3Delight's progress callback mechanism to determine if the render's still
 * going.
 */
void progressCallBack( float done ) {
	if( done < 100. ) {
		rendering = true;
		//cout << done << endl << flush;
	} else {
		//cout << "Done!!!" << endl << flush;
		rendering = false;
	}
}
#endif

/**
 *  Creates a new instance of the command plug-in.
 */
void* liqPreviewShader::creator()
{
    return new liqPreviewShader();
}

MSyntax liqPreviewShader::syntax()
{
  MSyntax syn;

  syn.addFlag( "s",    "shader",           MSyntax::kString );
  syn.addFlag( "r",    "renderer",         MSyntax::kString );
  syn.addFlag( "dd",   "displayDriver",    MSyntax::kString );
  syn.addFlag( "dn",   "displayName",      MSyntax::kString );
  syn.addFlag( "ds",   "displaySize",      MSyntax::kLong );
  syn.addFlag( "sshn", "shortShaderNames" );
  syn.addFlag( "nbp",  "noBackPlane");
  syn.addFlag( "os",   "objectSize",       MSyntax::kDouble );
  syn.addFlag( "sr",   "shadingRate",      MSyntax::kDouble );
  syn.addFlag( "pxs",  "pixelSamples",     MSyntax::kLong );
  syn.addFlag( "p",    "pipe");
  syn.addFlag( "t",    "type");
  syn.addFlag( "pi",   "previewIntensity", MSyntax::kDouble );

  syn.addFlag( "sph",  "sphere");
  syn.addFlag( "tor",  "torus");
  syn.addFlag( "cyl",  "cylinder");
  syn.addFlag( "cub",  "cube");
  syn.addFlag( "pla",  "plane");
  syn.addFlag( "tea",  "teapot");
  syn.addFlag( "cst",  "custom",           MSyntax::kString );

  syn.addFlag( "cbk",  "customBackPlane",  MSyntax::kString );
  syn.addFlag( "clr",  "cleanRibs",  MSyntax::kString );

  return syn;
}


typedef struct liqPreviewShaderOptions
{
  string  shaderNodeName;
  string  displayDriver;
  string  displayName;
  string  renderCommand;
  string  backPlaneShader;
  bool    shortShaderName, backPlane, usePipe;
  int     displaySize;
  int     primitiveType;
  float   pixelSamples;
  float   objectScale;
  float   shadingRate;
  string  customRibFile;
  bool    fullShaderPath;
  string  type;
  float   previewIntensity;
  string  customBackplane;
  bool    cleanRibs;
} liqPreviewShaderOptions;

int liquidOutputPreviewShader( const string& fileName, const liqPreviewShaderOptions& options );

#ifndef _WIN32
void liquidNewPreview( const liqPreviewShaderOptions& options )
{
  int val = 0;
  int ret;
  if( options.shaderNodeName.empty() || options.displayDriver.empty() || options.renderCommand.empty() )
  {
    cerr << "Invalid options for shader preview" << endl;
    pthread_exit( ( void * )&val);
  }

  fflush( NULL );
  // Open a pipe to a command
  FILE *fp = popen( options.renderCommand.c_str(), "w");
  if( !fp )
  {
    MString str( MString( "Opening pipe to " ) + options.renderCommand.c_str() );
    perror( str.asChar() );
    pthread_exit( ( void * )&val);
  }
  val = fork();
  if( val == -1 ) // Parent with error
  {
    perror( "Fork for RIB output" );
    pthread_exit( ( void * )&val);
  }
  else if( val == 0 ) // Child
  {
    int fd = fileno(fp);
    // Redirect stdout to pipe
    // Warning : messages should be sent to sdterr until stdout is restored
    ret = dup2( fd, 1 );
    if( ret < 0 )
    {
      perror( "Pipe redirect failed " );
      pthread_exit( ( void * )&val);
    }
    // And output RIB stdout
    cout << "# Outputing " << options.shaderNodeName << endl;
    liquidOutputPreviewShader( string(), options );
    _exit( 0 );
  }
  cerr << "Waiting for process" << val << " to finish " << endl;
  ret = waitpid( val, NULL, 0 );
  cerr << "Waiting for " << options.renderCommand << " to finish" << endl;
  pclose( fp );   	// Wait until render finish
  val = 1;	// Set a "all is ok" returned value
  //cout << "Stdout is still open" << endl;
  LIQDEBUGPRINTF("-> Thread preview is done.\n" );
  pthread_exit( ( void * )&val);
}
#endif // ifndef _WIN32


MStatus	liqPreviewShader::doIt( const MArgList& args )
{
  MStatus status;

  liqPreviewShaderOptions preview;
  preview.shortShaderName = false;
  preview.usePipe = false;
  preview.backPlane = true;
  preview.displaySize = 128;
  preview.primitiveType = SPHERE;
  preview.objectScale = 1.;
  preview.shadingRate = 1.;
  preview.pixelSamples = 3.;
  preview.customRibFile.clear();
  preview.fullShaderPath = false;
  preview.type.clear();
  preview.previewIntensity = 1.;
  preview.customBackplane.clear();
  preview.cleanRibs = 1;

  string displayDriver( "framebuffer" );
  string displayName( "liqPreviewShader" );
  string shaderNodeName;

  string renderCommand;
#ifndef DELIGHT
  liquidRenderer.setRenderer();
  renderCommand = liquidRenderer.renderPreview.asChar();
#endif

  for ( unsigned i( 0 ); i < args.length(); i++ ) 
	{
    MString arg( args.asString( i, &status ) );

    if ( ( arg == "-tea" ) || ( arg == "-teapot" ) )  
		{
      preview.primitiveType = TEAPOT;
    } 
		else if ( ( arg == "-cube" ) || ( arg == "-box" ) ) 
		{
      preview.primitiveType = CUBE;
    } 
		else if ( arg == "-plane" )  
		{
      preview.primitiveType = PLANE;
    } 
		else if ( arg == "-torus" )  
		{
      preview.primitiveType = TORUS;
    } 
		else if ( arg == "-cylinder" ) 
		{
      preview.primitiveType = CYLINDER;
    } 
		else if ( arg == "-custom" )  
		{
      preview.primitiveType = CUSTOM;
      i++;
      preview.customRibFile = args.asString( i, &status ).asChar();
    } 
		else if ( ( arg == "-s" ) || ( arg == "-shader" ) ) 
		{
      i++;
      shaderNodeName = args.asString( i, &status ).asChar();
    } 
		else if ( ( arg == "-r" ) || ( arg == "-renderer" ) ) 
		{
      i++;
      renderCommand = args.asString( i, &status ).asChar();
    } 
		else if ( ( arg == "-dd" ) || ( arg == "-displayDriver" ) )  
		{
      i++;
      displayDriver = args.asString( i, &status ).asChar();
    } 
		else if ( ( arg == "-dn" ) || ( arg == "-displayName" ) ) 
		{
      i++;
      displayName = args.asString( i, &status ).asChar();
    } 
		else if ( ( arg == "-ds" ) || ( arg == "-displaySize" ) ) 
		{
      i++;
      MString argValue = args.asString( i, &status );
      preview.displaySize = argValue.asInt();
    } 
		else if ( ( arg == "-sshn" ) || ( arg == "-shortShaderNames" ) ) 
		{
      preview.shortShaderName = true;
    } 
		else if ( ( arg == "-p" ) || ( arg == "-pipe" ) ) 
		{
      preview.usePipe = true;
    } 
		else if ( ( arg == "-nbp" ) || ( arg == "-noBackPlane" ) ) 
		{
      preview.backPlane = false;
    } 
		else if ( ( arg == "-os" ) || ( arg == "-objectSize" ) ) 
		{
      i++;
      MString argValue = args.asString( i, &status );
      preview.objectScale = ( float )argValue.asDouble();
    } 
		else if ( ( arg == "-sr" ) || ( arg == "-shadingRate" ) ) 
		{
      i++;
      MString argValue = args.asString( i, &status );
      preview.shadingRate = ( float )argValue.asDouble();
    } 
		else if ( ( arg == "-t" ) || ( arg == "-type" ) ) 
		{
      i++;
      preview.type = args.asString( i, &status ).asChar();
    } 
		else if ( ( arg == "-pi" ) || ( arg == "-previewIntensity" ) ) 
		{
      i++;
      MString argValue = args.asString( i, &status ).asChar();
      preview.previewIntensity = ( float )argValue.asDouble();
    } 
		else if ( arg == "-cbk" || arg == "-customBackPlane" )  
		{
      i++;
      preview.customBackplane = args.asString( i, &status ).asChar();
    } 
		else if ( arg == "-clr" || arg == "-cleanRibs" )  
		{
      i++;
      preview.cleanRibs = args.asInt( i, &status );
    }
  }

  // Check values
  if ( shaderNodeName.empty() ) 
	{
    cerr << "Need a shader name for previews" << endl;
    return MS::kFailure;
  }

#ifndef DELIGHT
  if ( renderCommand.empty() ) 
	{
    cerr << "Need a render command for previews" << endl;
    return MS::kFailure;
  }
#endif
  string last3letters( shaderNodeName.substr( shaderNodeName.length()-3 ) );
  if ( string( liquidRenderer.shaderExtension.asChar() ) == last3letters ) 
    preview.fullShaderPath = true;

  preview.shaderNodeName = shaderNodeName;
  preview.renderCommand = renderCommand;
  preview.displayDriver = displayDriver;
  preview.displayName = displayName;
  //cout << "Display: " << displayName << endl;
  string tempString( liquidSanitizePath( getEnvironment( "LIQUIDHOME" ) ) );

  if ( tempString.empty() ) preview.backPlaneShader = "null";
  else                      preview.backPlaneShader = "liquidchecker"; // tempString + "/shaders/liquidchecker";
  

#ifdef DELIGHT
  liquidOutputPreviewShader( string(), preview ); // 3Delight doesn't need a RIB
#endif

#ifndef DELIGHT
	#ifndef _WIN32 // Pipes don't work right under bloody Windoze
  if ( preview.usePipe ) 
	{
    LIQDEBUGPRINTF( "-> Creating thread preview.\n" );
    pthread_t prevthread;
    if ( pthread_create( & prevthread, NULL, (void *(*)(void *)) liquidNewPreview, ( void * ) &preview ) ) 
		{
      perror( "Thread create" );
      return MS::kFailure;
    }
    void * threadreturn;
    // Wait for end of rendering thread
    // must do so to make sure local variables always exist for the renderer
    pthread_join( prevthread, &threadreturn  );
    LIQDEBUGPRINTF( "-> End of thread preview.\n" );
  } 
	else
	#endif
  {
	#ifdef _WIN32
    // Bad, better use global from liqRibTranslator
    tempString = getEnvironment( "TEMP" );
    if ( tempString.empty() ) 
		{
      tempString = getEnvironment( "TMP" );
      if ( tempString.empty() ) 
			{
        liquidMessage( "Cannot write preview RIB. Please define either 'TMP' or 'TEMP' environment variables and restart Maya.", messageError );
        return MS::kFailure;
      }
    }
    string tempRibName( tempString + "/liqPreviev.rib" ); // We just add a slash. No need to have nice looking paths here
	#endif
	#ifndef _WIN32
    string tempRibName;
    char name[ L_tmpnam ];
    tmpnam( name );
    tempRibName += string( name ) + ".liqTmp.rib";
	#endif
    liquidOutputPreviewShader( tempRibName, preview );
	#ifdef _WIN32
  // liqProcessLauncher::execute( renderCommand.c_str(), tempRibName.c_str(), "", true );
  _spawnlp( _P_WAIT, preview.renderCommand.c_str(), preview.renderCommand.c_str(), tempRibName.c_str(), NULL );
	FILE* fid( fopen( string( displayName + ".done" ).c_str(), "w" ) );
	if ( fid ) fclose( fid );
	#endif
	#ifndef _WIN32
  system( string( preview.renderCommand + " " + tempRibName + ";touch " + displayName + ".done&" ).c_str() );
	if ( preview.cleanRibs )
	{
		//printf("DELETE %s \n", tempRibName.c_str());
		int delRez = unlink( tempRibName.c_str() );
		//printf("DELETE %s.done \n", displayName.c_str());
		//delRez += unlink( displayName + ".done" );
	}
	fflush( stdout );
	fflush( stderr );
	#endif
  }
#else // #ifndef DELIGHT
	#ifdef _WIN32
	FILE* fid( fopen( string( displayName + ".done" ).c_str(), "w" ) );
	if ( fid ) fclose( fid );
	#endif
#endif
  return MS::kSuccess;
}

/**
 * Writes preview RIB into fileName for a shader
 * If fileName is RI_NULL : output to stdout / directly to renderer
 * returns 1 on success
 */
int liquidOutputPreviewShader( const string& fileName, const liqPreviewShaderOptions& options )
{
	// clear shaders
	liqShaderFactory::instance().clearShaders();

  char* shaderFileName;
  liqShader currentShader;
  MObject	shaderObj;
    
  if ( options.fullShaderPath ) 
	{
		// a full shader path was specified
    //cout <<"[liquid] got full path for preview !"<<endl;

    //shaderFileName = const_cast<char*>(options.shaderNodeName);

    string tmp( options.shaderNodeName );
    currentShader.file = tmp.substr( 0, tmp.length() - 5 );

    if ( options.type == "surface" ) currentShader.shader_type = SHADER_TYPE_SURFACE;
    else if ( options.type == "displacement" ) currentShader.shader_type = SHADER_TYPE_DISPLACEMENT;

    //cout <<"[liquid]   options.shaderNodeName = " << options.shaderNodeName << endl;
    //cout <<"[liquid]   options.type = "<<options.type<<endl;

  } 
	else 
	{
    // a shader node was specified

    MSelectionList shaderNameList;
    shaderNameList.add( options.shaderNodeName.c_str() );
    shaderNameList.getDependNode( 0, shaderObj );
    if( shaderObj == MObject::kNullObj )
    {
      MGlobal::displayError( string( "Can't find node for " + options.shaderNodeName ).c_str() );
      RiEnd();
      return 0;
    }
    currentShader = liqShader( shaderObj );
  }
	MFnDependencyNode assignedShader( shaderObj );

  // Get the Pathes in globals node
  MObject globalObjNode;
  MString liquidShaderPath = "",liquidTexturePath = "",liquidProceduralPath = "";
  MStatus status;
  MSelectionList globalList;
	
	// get the current project directory
  MString liquidProjectDir;
  MGlobal::executeCommand( "workspace -q -rd", liquidProjectDir );
	
  status = globalList.add( "liquidGlobals" );
  if ( globalList.length() > 0 ) 
	{
    status.clear();
    status = globalList.getDependNode( 0, globalObjNode );
    MFnDependencyNode globalNode( globalObjNode );
		liquidGetPlugValue( globalNode, "shaderPath", liquidShaderPath, status );
    liquidGetPlugValue( globalNode, "texturePath", liquidTexturePath, status );
    liquidGetPlugValue( globalNode, "proceduralPath", liquidProceduralPath, status );
  }
  if ( fileName.empty() ) 
	{
    RiBegin( NULL );
#ifdef DELIGHT
    //RtPointer callBack( progressCallBack );
    //RiOption( "statistics", "progresscallback", &callBack, RI_NULL );
#endif
  } 
	else 
    RiBegin( (RtToken)fileName.c_str() );
  

  string liquidHomeDir = liquidSanitizeSearchPath( getEnvironment( "LIQUIDHOME" ) );
  string shaderPath( "&:@:.:~:" + liquidHomeDir + "/lib/shaders" );
  string texturePath( "&:@:.:~:" + liquidHomeDir + "/lib/textures" );
  string proceduralPath( "&:@:.:~:" + liquidHomeDir + "/lib/shaders" );

  string projectDir = string( liquidSanitizeSearchPath( liquidProjectDir ).asChar() );
	
  if ( liquidProjectDir != "" )
  {
		shaderPath += ":" + projectDir;	
    texturePath += ":" + projectDir;
    proceduralPath += ":" + projectDir;
  }
  if ( liquidShaderPath != "" )
		shaderPath += ":" + string( liquidSanitizeSearchPath( parseString( liquidShaderPath, false ) ).asChar());	
  if ( liquidTexturePath != "" )
		texturePath += ":" + string( liquidSanitizeSearchPath( parseString( liquidTexturePath, false) ).asChar());	
	if ( liquidProceduralPath != "" )
		proceduralPath += ":" + string( liquidSanitizeSearchPath( parseString( liquidProceduralPath, false) ).asChar());	
  
  RtString list( const_cast< RtString >( shaderPath.c_str() ) );
  RiOption( "searchpath", "shader", &list, RI_NULL );
	
  RtString texPath( const_cast< RtString >( texturePath.c_str() ) );
  if( texPath[ 0 ] ) RiOption( "searchpath","texture", &texPath, RI_NULL );
  
  RtString procPath( const_cast< RtString >( proceduralPath.c_str() ) );
  if( procPath[ 0 ] ) RiOption( "searchpath","procedural", &procPath, RI_NULL );

  RiShadingRate( ( RtFloat )options.shadingRate );
  RiPixelSamples( options.pixelSamples, options.pixelSamples );

#ifdef PRMAN
  if ( MString( "PRMan" ) == liquidRenderer.renderName )
	RiPixelFilter( RiCatmullRomFilter, 4., 4. );
#elif defined( DELIGHT )
  if ( MString( "3Delight" ) == liquidRenderer.renderName )
    RiPixelFilter( RiSeparableCatmullRomFilter, 4., 4. );
//    RiPixelFilter( RiMitchellFilter, 4., 4.);
#else
  RiPixelFilter( RiCatmullRomFilter, 4., 4. );
#endif

  RiFormat( ( RtInt )options.displaySize, ( RtInt )options.displaySize, 1.0 );
  if ( options.backPlane ) 
	{
    RiDisplay( const_cast< RtString >( options.displayName.c_str() ),
               const_cast< RtString >( options.displayDriver.c_str() ), RI_RGB, RI_NULL );
  } 
	else 
	{ // Alpha might be useful
    RiDisplay( const_cast< RtString >( options.displayName.c_str() ),
               const_cast< RtString >( options.displayDriver.c_str() ), RI_RGBA, RI_NULL );
  }
  RtFloat fov( 22.5 );
  RiProjection( "perspective", "fov", &fov, RI_NULL );
  RiTranslate( 0, 0, 2.75 );
  RiExposure(1, currentShader.m_previewGamma);
  RiWorldBegin();
  RiReverseOrientation();
  RiTransformBegin();
  RiRotate( -90., 1., 0., 0. );
  RiCoordinateSystem( "_environment" );
  RiTransformEnd();
  RtLightHandle ambientLightH, directionalLightH;
  RtFloat intensity;
  intensity = 0.05 * (RtFloat)options.previewIntensity;
  ambientLightH = RiLightSource( "ambientlight", "intensity", &intensity, RI_NULL );
  intensity = 0.9 * (RtFloat)options.previewIntensity;
  RtPoint from;
  RtPoint to;
  from[0] = -1.; from[1] = 1.5; from[2] = -1.;
  to[0] = 0.; to[1] = 0.; to[2] = 0.;
  RiTransformBegin();
    RiRotate( 55.,  1, 0, 0 );
    RiRotate( 30.,  0, 1, 0 );
    directionalLightH = RiLightSource( "liquiddistant", "intensity", &intensity, RI_NULL );
  RiTransformEnd();
  intensity = 0.2f * (RtFloat)options.previewIntensity;
  from[0] = 1.3f; from[1] = -1.2f; from[2] = -1.;
  RiTransformBegin();
    RiRotate( -50.,  1, 0, 0 );
    RiRotate( -40.,  0, 1, 0 );
    directionalLightH = RiLightSource( "liquiddistant", "intensity", &intensity, RI_NULL );
  RiTransformEnd();

  RiAttributeBegin();

/*!!! liqShader::writeRibAttributes do this work now	
	// output displacement bound
  float displacementBounds = 0.;
	liquidGetPlugValue( assignedShader, "displacementBound", displacementBounds, status);
  
  if ( displacementBounds > 0. ) 
	{
    RtString coordsys = "shader";
    // RiAttribute( "displacementbound", (RtToken) "sphere", &displacementBounds, "coordinatesystem", &coordsys, RI_NULL );
    RiAttribute( "displacementbound", "coordinatesystem", &coordsys, RI_NULL );
    RiAttribute( "displacementbound", "sphere", &displacementBounds, RI_NULL );
  }
*/

  LIQ_GET_SHADER_FILE_NAME( shaderFileName, options.shortShaderName, currentShader );

	// output shader space
  MString shadingSpace;
	liquidGetPlugValue( assignedShader, "shaderSpace", shadingSpace, status);
  
  if ( shadingSpace != "" ) 
	{
    RiTransformBegin();
    RiCoordSysTransform( (char*) shadingSpace.asChar() );
  }

  RiTransformBegin();
  // Rotate shader space to make the preview more interesting
  RiRotate( 60., 1., 0., 0. );
  RiRotate( 60., 0., 1., 0. );
  RtFloat scale( 1.f / ( RtFloat )options.objectScale );
  RiScale( scale, scale, scale );

	if ( currentShader.shader_type == SHADER_TYPE_SURFACE || currentShader.shader_type == SHADER_TYPE_SHADER )
	{
    RiColor( currentShader.rmColor );
    RiOpacity( currentShader.rmOpacity );
		//cout << "Shader: " << shaderFileName << endl;
		if ( options.fullShaderPath ) 
			RiSurface( shaderFileName, RI_NULL );
		else
		{
			// its one less as the tokenPointerArray has a preset size of 1 not 0
			//int shaderParamCount = currentShader.tokenPointerArray.size() - 1;
			//RiSurfaceV( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
			liqShader liqAssignedShader( shaderObj );
			liqAssignedShader.write( options.shortShaderName, 0, SHADER_TYPE_SURFACE);
		}
  } 
	else if ( currentShader.shader_type == SHADER_TYPE_DISPLACEMENT ) 
	{
    RtToken Kd = "Kd";
    RtFloat KdValue( 1. );
#ifdef GENERIC_RIBLIB    
    // !!! current ribLib has wrong interpretation of RiSurface parameters 
    RiSurface( "plastic", Kd, &KdValue, RI_NULL );
#else
    RiSurface( "plastic", &Kd, &KdValue, RI_NULL );
#endif    
		if ( options.fullShaderPath ) 
			RiDisplacement( shaderFileName, RI_NULL );
		else 
		{
			//int shaderParamCount = currentShader.tokenPointerArray.size() - 1;
			//RiDisplacementV( shaderFileName, shaderParamCount, tokenArray.get(), pointerArray.get() );
			liqShader liqAssignedShader( shaderObj );
			liqAssignedShader.write( (bool)options.shortShaderName, (unsigned int)0 );
		}
  }
  RiTransformEnd();
  if ( shadingSpace != "" ) RiTransformEnd();

 switch ( options.primitiveType ) 
 {
    case CYLINDER: 
		{
      RiReverseOrientation();
      RiScale( 0.95, 0.95, 0.95 );
      RiRotate( 60., 1., 0., 0. );
      RiTranslate( 0., 0., -0.05 );
      RiCylinder( 0.5, -0.3, 0.3, 360., RI_NULL );
      RiTranslate( 0., 0., 0.3f );
      RiTorus( 0.485, 0.015, 0., 90., 360., RI_NULL );
      RiDisk( 0.015, 0.485, 360., RI_NULL );
      RiTranslate( 0., 0., -0.6 );
      RiTorus( 0.485, 0.015, 270., 360., 360., RI_NULL );
      RiReverseOrientation();
      RiDisk( -0.015, 0.485, 360., RI_NULL );
      break;
    }
    case TORUS: 
		{
      RiRotate( 45., 1., 0., 0. );
      RiTranslate( 0., 0., -0.05 );
      RiReverseOrientation();
      RiTorus( 0.3f, 0.2f, 0., 360., 360., RI_NULL );
      break;
    }
    case PLANE: 
		{
      RiScale( 0.5, 0.5, 0.5 );
      RiReverseOrientation();
      static RtPoint plane[4] = {
        { -1.,  1.,  0. },
        {  1.,  1.,  0. },
        { -1., -1.,  0. },
        {  1., -1.,  0. }
      };
      RiPatch( RI_BILINEAR, RI_P, (RtPointer) plane, RI_NULL );
      break;
    }
    case TEAPOT: 
		{
      RiTranslate( 0.06f, -0.18f, 0. );
      RiRotate( -120., 1., 0., 0. );
      RiRotate( 130., 0., 0., 1. );
      RiScale( 0.2f, 0.2f, 0.2f );
			RiArchiveRecord( RI_VERBATIM, "Geometry \"teapot\"" );
      break;
    }
    case CUBE: 
		{
      /* Lovely cube with rounded corners and edges */
      RiScale( 0.35f, 0.35f, 0.35f );
      RiRotate( 60., 1., 0., 0. );
      RiRotate( 60., 0., 0., 1. );

      RiTranslate( 0.11f, 0., -0.08f );

      RiReverseOrientation();

      static RtPoint top[ 4 ] = { { -0.95, 0.95, -1. }, { 0.95, 0.95, -1. }, { -0.95, -0.95, -1. },  { 0.95, -0.95, -1. } };
      RiPatch( RI_BILINEAR, RI_P, ( RtPointer ) top, RI_NULL );

      static RtPoint bottom[ 4 ] = { { 0.95, 0.95, 1. }, { -0.95, 0.95, 1. }, { 0.95, -0.95, 1. }, { -0.95, -0.95, 1. } };
      RiPatch( RI_BILINEAR, RI_P, ( RtPointer ) bottom, RI_NULL );

      static RtPoint right[ 4 ] = { { -0.95, -1., -0.95 }, { 0.95, -1., -0.95 }, { -0.95, -1., 0.95 }, { 0.95, -1., 0.95 } };
      RiPatch( RI_BILINEAR, RI_P, ( RtPointer ) right, RI_NULL );

      static RtPoint left[ 4 ] = { { 0.95, 1., -0.95 }, { -0.95, 1., -0.95 }, { 0.95, 1., 0.95 }, { -0.95, 1., 0.95 } };
      RiPatch( RI_BILINEAR, RI_P, ( RtPointer ) left, RI_NULL );

      static RtPoint front[ 4 ] = { {-1., 0.95, -0.95 }, { -1., -0.95, -0.95 }, { -1., 0.95, 0.95 }, { -1., -0.95, 0.95 } };
      RiPatch( RI_BILINEAR, RI_P, ( RtPointer ) front, RI_NULL );

      static RtPoint back[ 4 ] = { { 1., -0.95, -0.95 }, { 1., 0.95, -0.95 }, { 1., -0.95, 0.95 }, { 1., 0.95, 0.95 } };
      RiPatch( RI_BILINEAR, RI_P, ( RtPointer ) back, RI_NULL );

      RiTransformBegin();
      RiTranslate( 0.95, 0.95, 0. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( 0.95, -0.95, 0. );
      RiRotate( -90., 0., 0., 1. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( -0.95, 0.95, 0. );
      RiRotate( 90., 0., 0., 1. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( -0.95, -0.95, 0. );
      RiRotate( 180., 0., 0., 1. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( 0., 0., 0.95 );

      RiTransformBegin();

      RiTransformBegin();
      RiTranslate( 0.95, 0.95, 0. );
      RiSphere( 0.05, 0., 0.05, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( 0.95, -0.95, 0. );
      RiRotate( -90., 0., 0., 1. );
      RiSphere( 0.05, 0., 0.05, 90., RI_NULL );
      RiTransformEnd();

      RiRotate( 180., 0., 0., 1. );

      RiTransformBegin();
      RiTranslate( 0.95, 0.95, 0. );
      RiSphere( 0.05, 0., 0.05, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( 0.95, -0.95, 0. );
      RiRotate( -90., 0., 0., 1. );
      RiSphere( 0.05, 0., 0.05, 90., RI_NULL );
      RiTransformEnd();

      RiTransformEnd();

      RiRotate( 90., 1., 0., 0. );

      RiTransformBegin();
      RiTranslate( 0.95, 0., 0. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( -0.95, 0., 0. );
      RiRotate( 90., 0., 0., 1. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiRotate( 90., 0., 1., 0. );

      RiTransformBegin();
      RiTranslate( 0.95, 0.,  0. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( -0.95, 0., 0. );
      RiRotate( 90., 0., 0., 1. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( 0., 0., -0.95 );

      RiTransformBegin();

      RiTransformBegin();
      RiTranslate( 0.95, 0.95, 0. );
      RiSphere( 0.05, -0.05, 0., 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( 0.95, -0.95, 0. );
      RiRotate( -90., 0., 0., 1. );
      RiSphere( 0.05, -0.05, 0., 90., RI_NULL );
      RiTransformEnd();

      RiRotate( 180., 0., 0., 1. );

      RiTransformBegin();
      RiTranslate( 0.95, 0.95, 0. );
      RiSphere( 0.05, -0.05, 0., 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( 0.95, -0.95, 0. );
      RiRotate( -90., 0., 0., 1. );
      RiSphere( 0.05, -0.05, 0., 90., RI_NULL );
      RiTransformEnd();

      RiTransformEnd();

      RiRotate( 90., 1., 0., 0. );

      RiTransformBegin();
      RiTranslate( -0.95, 0.,  0. );
      RiRotate( 180., 0., 0., 1. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( 0.95, 0.,  0. );
      RiRotate( -90., 0., 0., 1. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiRotate( 90., 0., 1., 0. );

      RiTransformBegin();
      RiTranslate( 0.95, 0.,  0. );
      RiRotate( -90., 0., 0., 1. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformBegin();
      RiTranslate( -0.95, 0.,  0. );
      RiRotate( 180., 0., 0., 1. );
      RiCylinder( 0.05, -0.95, 0.95, 90., RI_NULL );
      RiTransformEnd();

      RiTransformEnd();

      break;
    }
    case CUSTOM: 
		{
      //cout <<"custom : "<<options.customRibFile<<endl;
      if ( fileExists( options.customRibFile.c_str() ) ) 
			{
        RiReadArchive( const_cast< RtToken >( options.customRibFile.c_str() ), NULL, RI_NULL );
      }
      break;
    }
    case SPHERE:
    default: 
		{
      RiRotate( 60., 1., 0., 0. );
      RiReverseOrientation();
      RiSphere( 0.5, -0.5, 0.5, 360., RI_NULL );
      break;
    }
  }

  RiAttributeEnd();
  /*
   * Backplane
   */
  if ( options.backPlane ) 
	{
    if ( options.customBackplane.empty() ) 
		{
      RiAttributeBegin();
      RiScale( 0.91, 0.91, 0.91 );
      if ( SHADER_TYPE_DISPLACEMENT == currentShader.shader_type ) 
			{
        RtColor bg = { 0.698, 0.698, 0. };
        RiColor( bg );
      } 
			else 
        RiSurface( const_cast< RtToken >( options.backPlaneShader.c_str() ), RI_NULL );

			RtInt visible = 1;
      RtString transmission = "transparent";

      RiAttribute( "visibility", ( RtToken ) "camera", &visible, ( RtToken ) "trace", &visible, ( RtToken ) "transmission", ( RtPointer ) &transmission, RI_NULL );
      static RtPoint backplane[4] = {
        { -1.,  1.,  2. },
        {  1.,  1.,  2. },
        { -1., -1.,  2. },
        {  1., -1.,  2. }
      };
      RiPatch( RI_BILINEAR, RI_P, (RtPointer) backplane, RI_NULL );
      RiAttributeEnd();
    } 
		else 
		{
      if ( fileExists( options.customBackplane.c_str() ) ) 
			{
        RiAttributeBegin();
          RiScale( 1., 1., -1. );
          RiReadArchive( const_cast< RtString >( options.customBackplane.c_str() ), NULL, RI_NULL );
        RiAttributeEnd();
      }
    }
  }

  RiWorldEnd();

/* this caused maya to hang up under windoof - Alf
#ifdef _WIN32
//	Wait until the renderer is done
	while( !fileFullyAccessible( options.displayName.c_str() ) );
#endif
*/
  RiEnd();

  fflush( NULL );

  LIQDEBUGPRINTF("-> Shader Preview RIB output done.\n" );

  return 1;
}




