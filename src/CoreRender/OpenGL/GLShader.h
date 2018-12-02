#pragma once
#include "Common.h"

class GLShader : public ICoreShader
{
	GLuint _programID = 0u;
	GLuint _vertID = 0u;
	GLuint _geomID = 0u;
	GLuint _fragID = 0u;

	struct Buffer
	{
		GLuint UBO = 0u;
	};

	struct Parameter
	{
		Buffer *buffer = nullptr;

	};

	std::unordered_map<string, Parameter> _parameters;

public:

	GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID);
	virtual ~GLShader();

	GLuint programID() const { return _programID; }
	
	API SetFloatParameter(const char* name, float value) override;
	API SetVec4Parameter(const char* name, const vec4 *value) override;
	API SetMat4Parameter(const char* name, const mat4 *value) override;
	API SetUintParameter(const char* name, uint value) override;
	API FlushParameters() override;
};

