#pragma once
#include "Common.h"

class Shader : public IShader
{
	ICoreShader *_coreShader = nullptr;

	const char *vert = nullptr;
	const char *geom = nullptr;
	const char *frag = nullptr;

public:
	Shader(ICoreShader *s, const char *vertIn, const char *geomIn, const char *fragIn) :
		_coreShader(s), vert(vertIn), geom(geomIn), frag(fragIn) {}
	virtual ~Shader();

	API GetCoreShader(ICoreShader **shaderOut) override;
	API GetVert(OUT const char **textOut) override;
	API GetGeom(OUT const char **textOut) override;
	API GetFrag(OUT const char **textOut) override;
	API SetFloatParameter(const char* name, float value) override;
	API SetVec4Parameter(const char* name, const vec4 *value) override;
	API SetMat4Parameter(const char* name, const mat4 *value) override;
	API SetUintParameter(const char* name, uint value) override;
	API FlushParameters() override;


	RUNTIME_ONLY_RESOURCE_HEADER
};
