#pragma once
#include "Common.h"

class ShaderFile : public IShaderFile
{
	const char *text = nullptr;

public:
	ShaderFile(const char *textIn, const string& filePath) :
		text(textIn), _file(filePath)	{}

	virtual ~ShaderFile();

	API GetText(OUT const char **textOut) override { *textOut = text; return S_OK; }
	API SetText(const char *textIn) override;

	SHARED_ONLY_RESOURCE_HEADER
};
