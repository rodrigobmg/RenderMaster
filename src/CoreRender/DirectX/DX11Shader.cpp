#include "pch.h"
#include "DX11Shader.h"
#include "DX11CoreRender.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

extern vector<ConstantBuffer> ConstantBufferPool;

void DX11Shader::initShader(ShaderInitData& data, SHADER_TYPE type)
{
	ID3D11ShaderReflection* reflection = nullptr;
	D3D11Reflect(data.bytecode, data.size, &reflection);

	D3D11_SHADER_DESC shaderDesc;
	reflection->GetDesc(&shaderDesc);

	// each Constant Buffer
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

		vector<ConstantBuffer::ConstantBufferParameter> cbParameters;

		// each parameters
		for (uint j = 0; j < bufferDesc.Variables; j++)
		{
			ID3D11ShaderReflectionVariable *var = buffer->GetVariableByIndex(j);
			D3D11_SHADER_VARIABLE_DESC varDesc;
			var->GetDesc(&varDesc);

			uint bytesVariable = varDesc.Size;

			ConstantBuffer::ConstantBufferParameter p;
			p.name = varDesc.Name;
			p.bytes = varDesc.Size;
			p.offset = varDesc.StartOffset;
			p.elements = 1; // TODO: arrays		

			cbParameters.push_back(p);
		}

		// find existing buffer with same memory layout
		int indexFound = -1;
		{
			for (int j = 0; indexFound == -1 && j < ConstantBufferPool.size(); j++)
			{
				bool isEqual = true;

				if (ConstantBufferPool[j].name != bufferDesc.Name)
					isEqual = false;

				if (ConstantBufferPool[j].bytes != bufferDesc.Size)
					isEqual = false;

				if (ConstantBufferPool[j].parameters.size() != cbParameters.size())
					isEqual = false;

				for (int k = 0; isEqual && k < cbParameters.size(); k++)
				{
					if (ConstantBufferPool[j].parameters[k].name != cbParameters[k].name ||
						ConstantBufferPool[j].parameters[k].bytes != cbParameters[k].bytes ||
						ConstantBufferPool[j].parameters[k].offset != cbParameters[k].offset ||
						ConstantBufferPool[j].parameters[k].elements != cbParameters[k].elements)
					{
						isEqual = 0;
					}
				}

				if (isEqual)
					indexFound = j;
			}
		}

		//if (indexFound != -1) // buffer found
		//{
		//	_bufferIndicies.push_back(indexFound);

		//	for (int i = 0; i < parametersUBO.size(); i++)
		//	{
		//		_parameters[parametersUBO[i].name] = {(int)indexFound, (int)i};
		//	}
		//} else // not found => create new
		//{
		//	GLuint id;

		//	glGenBuffers(1, &id);
		//	glBindBuffer(GL_UNIFORM_BUFFER, id);
		//	vector<char> data(bytesUBO, '\0');
		//	glBufferData(GL_UNIFORM_BUFFER, bytesUBO, &data[0], GL_DYNAMIC_DRAW);
		//	glBindBuffer(GL_UNIFORM_BUFFER, 0);

		//	_bufferIndicies.push_back(UBOpool.size());

		//	for (int i = 0; i < parametersUBO.size(); i++)
		//	{
		//		_parameters[parametersUBO[i].name] = {(int)UBOpool.size(), (int)i};
		//	}

		//	UBOpool.emplace_back(std::move(UBO(id, bytesUBO, nameUBO, parametersUBO)));			
		//}
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
	if (v.pointer.pVertex)		{ v.pointer.pVertex->Release();		v.pointer.pVertex = nullptr; }
	if (f.pointer.pFragment)	{ f.pointer.pFragment->Release();	f.pointer.pFragment = nullptr; }
	if (g.pointer.pGeometry)	{ g.pointer.pGeometry->Release();	g.pointer.pGeometry = nullptr; }
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


