#pragma once
#include "Common.h"

namespace fs = std::experimental::filesystem;


class FileSystem final: public IFileSystem
{
	string _dataPath;

public:
	FileSystem();
	virtual ~FileSystem(){}

	void Init(const string& dataPath);

	API OpenFile(OUT IFile **pFile, const char* path, FILE_OPEN_MODE mode) override;
	API FileExist(const char *fullPath, OUT int *exist) override;
	API DirectoryExist(const char *fullPath, OUT int *exist) override;
	API GetName(OUT const char **pName) override;
};


class File final : public IFile
{
	std::fstream _file;
	fs::path _fsPath;

public:
	File(const std::ios_base::openmode& fileMode, const fs::path& path);

	API Read(OUT uint8 *pMem, uint bytes) override;
	API ReadStr(OUT char *pStr, OUT uint *bytes) override;
	API IsEndOfFile(OUT int *eof) override;
	API Write(const uint8 *pMem, uint bytes) override;
	API WriteStr(const char *pStr) override;
	API FileSize(OUT uint *size) override;
	API CloseAndFree() override;
};
