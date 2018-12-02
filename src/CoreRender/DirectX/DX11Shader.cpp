#include "pch.h"
#include "DX11Shader.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

DX11Shader::~DX11Shader()
{
	if (pVertex) { pVertex->Release(); pVertex = nullptr;	}
	if (pFragment) { pFragment->Release(); pFragment = nullptr; }
	if (pGeometry) { pGeometry->Release(); pGeometry = nullptr; }
}

API DX11Shader::SetFloatParameter(const char * name, float value)
{
	return S_OK;
}

API DX11Shader::SetVec4Parameter(const char * name, const vec4 * value)
{
	return S_OK;
}

API DX11Shader::SetMat4Parameter(const char * name, const mat4 * value)
{
	return S_OK;
}

API DX11Shader::SetUintParameter(const char * name, uint value)
{
	return S_OK;
}

API DX11Shader::FlushParameters()
{
	return S_OK;
}


