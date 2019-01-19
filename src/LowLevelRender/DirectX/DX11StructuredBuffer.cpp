#include "Pch.h"
#include "DX11StructuredBuffer.h"
#include "DX11CoreRender.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

DX11StructuredBuffer::DX11StructuredBuffer(ID3D11Buffer *bufIn, ID3D11ShaderResourceView *srvIn) : buf(bufIn), srv(srvIn)
{
	D3D11_BUFFER_DESC descBuf = {};
	bufIn->GetDesc(&descBuf);
	size = descBuf.ByteWidth;
}

DX11StructuredBuffer::~DX11StructuredBuffer()
{
	if (buf) { buf->Release(); buf = nullptr; }
	if (srv) { srv->Release(); srv = nullptr; }
	size = 0u;
}

API DX11StructuredBuffer::SetData(uint8 * data, size_t size)
{
	return S_OK;
}

API DX11StructuredBuffer::Reallocate(size_t newSize)
{
	if (newSize == size)
		return S_OK;

	if (buf) { buf->Release(); buf = nullptr; }
	if (srv) { srv->Release(); srv = nullptr; }

	return S_OK;
}
