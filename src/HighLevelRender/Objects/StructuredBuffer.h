#pragma once
#include "Common.h"

class StructuredBuffer : public IStructuredBuffer
{
	ICoreStructuredBuffer *_coreStructuredBuffer = nullptr;

public:
	StructuredBuffer(ICoreStructuredBuffer *buf) : _coreStructuredBuffer(buf) {}
	virtual ~StructuredBuffer();

	API GetCoreBuffer(ICoreStructuredBuffer **bufOut) override;
	API SetData(uint8 *data, size_t size) override;
	API Reallocate(size_t newSize) override;

	RUNTIME_ONLY_RESOURCE_HEADER
};
