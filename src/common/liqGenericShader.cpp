#include <liqGenericShader.h>

#include <liqShaderFactory.h>

liqGenericShader::liqGenericShader()
{
	shaderHandler = "None";
	hasErrors = false;
	name = "";
	shaderHandler = liqShaderFactory::instance().getUniqueShaderHandler();
	outputInShadow = true;
	m_outputAllParameters = false;
	rmColor[0] = 1.0;
	rmColor[1] = 1.0;
	rmColor[2] = 1.0;
	rmOpacity[0] = 1.0;
	rmOpacity[1] = 1.0;
	rmOpacity[2] = 1.0;
}


liqGenericShader::liqGenericShader( const liqGenericShader& src )
{
	name					= src.name;
	hasErrors				= src.hasErrors;
	shaderHandler			= src.shaderHandler;
	m_mObject				= src.m_mObject;
	outputInShadow			= src.outputInShadow;
	m_outputAllParameters	= src.m_outputAllParameters;
}


liqGenericShader::liqGenericShader(MObject shaderObj, bool outputAllParameters )
{
	MStatus status;
	outputInShadow = false;
	hasErrors = false;
	m_outputAllParameters = outputAllParameters;
	MFnDependencyNode shaderNode( shaderObj, &status );
	if ( status != MS::kSuccess )
	{
		hasErrors = true;
		return;
	}
	m_mObject = shaderObj;
	name = shaderNode.name().asChar();
	shaderHandler = shaderNode.name();
}


liqGenericShader::~liqGenericShader()
{
}

RtColor& liqGenericShader::getColor()
{
	return rmColor;
}

RtColor& liqGenericShader::getOpacity()
{
	return rmOpacity;
}
