#include "pch.h"
#include "DX11Shader.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

void DX11Shader::initShader(ShaderInitData& data, SHADER_TYPE type)
{
	ID3D11ShaderReflection* reflection = nullptr;
	D3D11Reflect(data.bytecode, data.size, &reflection);

	D3D11_SHADER_DESC shaderDesc;
	reflection->GetDesc(&shaderDesc);

	for(unsigned int i = 0; i < shaderDesc.ConstantBuffers; ++i)
	{			
		ID3D11ShaderReflectionConstantBuffer* buffer = reflection->GetConstantBufferByIndex(i);

		D3D11_SHADER_BUFFER_DESC bufferDesc;
		buffer->GetDesc(&bufferDesc);

		unsigned int registerIndex = 0;
		for(unsigned int k = 0; k < shaderDesc.BoundResources; ++k)
		{
			D3D11_SHADER_INPUT_BIND_DESC ibdesc;
			reflection->GetResourceBindingDesc(k, &ibdesc);

			if(!strcmp(ibdesc.Name, bufferDesc.Name))
				registerIndex = ibdesc.BindPoint;
		}

		////
		////Add constant buffer
		////
		//ConstantShaderBuffer* shaderbuffer = new ConstantShaderBuffer(register_index, Engine::String.ConvertToWideStr(bdesc.Name), buffer, &bdesc);
		//mShaderBuffers.push_back(shaderbuffer);
	}

}

DX11Shader::DX11Shader(ShaderInitData& vs, ShaderInitData& fs, ShaderInitData& gs)
{
	initShader(vs, SHADER_TYPE::SHADER_VERTEX);
	initShader(fs, SHADER_TYPE::SHADER_FRAGMENT);

	if (gs.pointer)
		initShader(gs, SHADER_TYPE::SHADER_GEOMETRY);
}

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


