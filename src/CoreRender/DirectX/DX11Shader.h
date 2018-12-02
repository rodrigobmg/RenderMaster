#pragma once
#include "Common.h"

class DX11Shader : public ICoreShader
{
	ID3D11VertexShader *pVertex = nullptr;
	ID3D11GeometryShader *pGeometry = nullptr;
	ID3D11PixelShader *pFragment = nullptr;

public:

	DX11Shader(ID3D11VertexShader* pVeretxIn, ID3D11GeometryShader* pGeometryIn, ID3D11PixelShader* pFragmentIn) : 
		pVertex(pVeretxIn), pGeometry(pGeometryIn), pFragment(pFragmentIn){}
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

