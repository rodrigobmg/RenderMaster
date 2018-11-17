#pragma once
#include "Common.h"

class Mesh : public IMesh
{
	ICoreMesh *_coreMesh = nullptr;

public:
	Mesh(ICoreMesh *m) : _coreMesh(m) {}
	Mesh(ICoreMesh *m, int isShared, const string& fileIn) : _coreMesh(m), _isShared(isShared), _file(fileIn) {}
	virtual ~Mesh(); 

	API GetCoreMesh(OUT ICoreMesh **meshOut) override;

	BASE_RESOURCE_HEADER
};
