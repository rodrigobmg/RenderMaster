#pragma once
#include "Common.h"

struct ShaderInitData
{
	void *pointer;
	unsigned char *bytecode;
	size_t size;
};

class DX11Shader : public ICoreShader
{
	ID3D11VertexShader *pVertex = nullptr;
	ID3D11GeometryShader *pGeometry = nullptr;
	ID3D11PixelShader *pFragment = nullptr;

	void initShader(ShaderInitData& data, SHADER_TYPE type);

public:

	DX11Shader(ShaderInitData& vs, ShaderInitData& fs, ShaderInitData& gs);
	virtual ~DX11Shader();

	ID3D11VertexShader*		vs() const { return pVertex; }
	ID3D11GeometryShader*	gs() const { return pGeometry; }
	ID3D11PixelShader*		fs() const { return pFragment; }

	API SetFloatParameter(const char* name, float value) override;
	API SetVec4Parameter(const char* name, const vec4 *value) override;
	API SetMat4Parameter(const char* name, const mat4 *value) override;
	API SetUintParameter(const char* name, uint value) override;
	API FlushParameters() override;
};

