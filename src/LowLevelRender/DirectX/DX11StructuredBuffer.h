#pragma once
#include "Common.h"

class DX11StructuredBuffer : public ICoreStructuredBuffer
{
	ID3D11Buffer *buf = nullptr;
	ID3D11ShaderResourceView *srv = nullptr;
	size_t size = 0;

public:
	DX11StructuredBuffer(ID3D11Buffer *bufIn, ID3D11ShaderResourceView *srvIn);
	virtual ~DX11StructuredBuffer();

	API SetData(uint8 *data, size_t size) override;
	API Reallocate(size_t newSize) override;
};

