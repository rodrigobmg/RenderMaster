#pragma once

#include "vector_math.h"

#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <Unknwn.h>

#include <iostream>

// comment this to build without FBX SDK
#define USE_FBX

#define API HRESULT

typedef unsigned int uint;
typedef unsigned char uint8;
typedef HWND WinHandle;

#define DEFINE_ENUM_OPERATORS(ENUM_NAME) \
inline ENUM_NAME operator|(ENUM_NAME a, ENUM_NAME b) \
{ \
	return static_cast<ENUM_NAME>(static_cast<int>(a) | static_cast<int>(b)); \
} \
inline ENUM_NAME operator&(ENUM_NAME a, ENUM_NAME b) \
{ \
	return static_cast<ENUM_NAME>(static_cast<int>(a) & static_cast<int>(b)); \
}

namespace RENDER_MASTER 
{
	class ISubSystem;
	class ILogEvent;
	class IInitCallback;
	class IUpdateCallback;
	enum class SUBSYSTEM_TYPE;

	//////////////////////
	// Core
	//////////////////////

	enum class INIT_FLAGS
	{
		WINDOW_FLAG = 0x0000000F, 
		EXTERN_WINDOW = 0x00000002, // engine uses client's created window		
		GRAPHIC_LIBRARY_FLAG = 0x000000F0,
		OPENGL45 = 0x00000010,
		DIRECTX11 = 0x00000020,		
		CREATE_CONSOLE_FLAG = 0x00000F00,
		NO_CREATE_CONSOLE = 0x00000100,  // no need create console
		CREATE_CONSOLE = 0x00000200 // engine should create console		
	};
	DEFINE_ENUM_OPERATORS(INIT_FLAGS)

	enum class LOG_TYPE
	{
		NORMAL,
		WARNING,
		FATAL
	};

	// {A97B8EB3-93CE-4A45-800D-367084CFB4B1}
	DEFINE_GUID(IID_Core,
		0xa97b8eb3, 0x93ce, 0x4a45, 0x80, 0xd, 0x36, 0x70, 0x84, 0xcf, 0xb4, 0xb1);

	class ICore : public IUnknown
	{
	public:

		virtual API Init(INIT_FLAGS flags, const char *pDataPath, const WinHandle* handle) = 0;
		virtual API Start() = 0;
		virtual API RenderFrame() = 0;
		virtual API GetSubSystem(ISubSystem *&pSubSystem, SUBSYSTEM_TYPE type) = 0;
		virtual API GetDataDir(const char *&pStr) = 0;
		virtual API GetWorkingDir(const char *&pStr) = 0;
		virtual API GetInstalledDir(const char *&pStr) = 0;
		virtual API Log(const char *pStr, LOG_TYPE type) = 0;
		virtual API AddInitCallback(IInitCallback *pCallback) = 0;
		virtual API AddUpdateCallback(IUpdateCallback *pCallback) = 0;
		virtual API CloseEngine() = 0;

		// Events
		virtual API GetLogPrintedEv(ILogEvent *&pEvent) = 0;
	};


	//////////////////////
	// Common
	//////////////////////

	enum class SUBSYSTEM_TYPE
	{
		FILESYSTEM,
		CORE_RENDER,
		RESOURCE_MANAGER,
		SCENE_MANAGER
	};

	class ISubSystem
	{
	public:
		virtual API GetName(const char *&pName) = 0;
	};

	enum class RES_TYPE
	{
		CORE_MESH,
		CORE_TEXTURE,
		CORE_SHADER,
		GAMEOBJECT,
		MODEL,
		CAMERA
	};

	class IResource
	{
	public:
		virtual API Free() = 0;
		virtual API GetType(RES_TYPE& type) = 0;
	};

	class IInitCallback
	{
	public:
		virtual API Init() = 0;
	};

	class IUpdateCallback
	{
	public:
		virtual API Update() = 0;
	};
	

	//////////////////////
	// Events
	//////////////////////

	class ILogEventSubscriber
	{
	public:
		virtual API Call(const char *pMessage, LOG_TYPE type) = 0;
	};

	class ILogEvent
	{
	public:

		virtual API Subscribe(ILogEventSubscriber *pSubscriber) = 0;
		virtual API Unsubscribe(ILogEventSubscriber *pSubscriber) = 0;
	};


	class IEventSubscriber
	{
	public:
		virtual API Call() = 0;
	};

	class IEvent
	{
	public:

		virtual API Subscribe(IEventSubscriber *pSubscriber) = 0;
		virtual API Unsubscribe(IEventSubscriber *pSubscriber) = 0;
	};


	//////////////////////
	// Core Render stuff
	//////////////////////

	enum class INPUT_ATTRUBUTE
	{
		NONE = 0,
		POSITION = 1 << 0,
		NORMAL = 1 << 1,
		TEX_COORD = 1 << 2,
		COLOR = 1 << 3
	};
	DEFINE_ENUM_OPERATORS(INPUT_ATTRUBUTE)

	enum class VERTEX_TOPOLOGY
	{
		LINES,
		TRIANGLES,
	};

	// At minimum position attribute must be present
	// Stride is step in bytes to move along the array from vertex to vertex 
	// Offset also specified in bytes
	// Stride and offset defines two case:
	//
	// 1) Interleaved
	//
	// x1, y1, z1, UVx1, UVy1, Nx1, Ny1, Nz1,   x2, y2, z2, UVx2, UVy2, Nx2, Ny2, Nz2, ...
	// positionOffset = 0, positionStride = 32,
	// texCoordOffset = 12, texCoordStride = 32,
	// normalOffset = 20, normalStride = 32
	//
	// 2) Tightly packed attributes
	//
	// x1, y2, z1, x2, y2, z2, ...   UVx1, UVy1, UVx2, UVy2, ...  Nx1, Ny1, Nz1, Nx2, Ny2, Nz2, ...
	// positionOffset = 0, positionStride = 12,
	// texCoordOffset = vertexNumber * 12, texCoordStride = 8,
	// normalOffset = vertexNumber * (12 + 8), normalStride = 12

	struct MeshDataDesc
	{
		uint8 *pData{nullptr};

		uint numberOfVertex{0};

		uint positionOffset{0};
		uint positionStride{0};

		bool normalsPresented{false};
		uint normalOffset{0};
		uint normalStride{0};

		bool texCoordPresented{false};
		uint texCoordOffset{0};
		uint texCoordStride{0};

		bool colorPresented{false};
		uint colorOffset{0};
		uint colorStride{0};

	};

	enum class MESH_INDEX_FORMAT
	{
		NOTHING,
		INT32,
		INT16
	};

	enum class SHADER_VARIABLE_TYPE
	{
		SVT_INT,
		SVT_FLOAT,
		SVT_VECTOR3,
		SVT_VECTOR4,
		SVT_MATRIX3X3,
		SVT_MATRIX4X4,
	};

	struct MeshIndexDesc
	{
		MeshIndexDesc() : pData(nullptr), number(0), format(MESH_INDEX_FORMAT::NOTHING) {}

		uint8 *pData;
		uint number;
		MESH_INDEX_FORMAT format;
	};

	class ICoreMesh : public IResource
	{
	public:
		virtual API GetNumberOfVertex(uint &vertex) = 0;
		virtual API GetAttributes(INPUT_ATTRUBUTE &attribs) = 0;
		virtual API GetVertexTopology(VERTEX_TOPOLOGY &topology) = 0;
	};

	struct ShaderText
	{
		const char** pVertText{nullptr};
		const char** pGeomText{nullptr};
		const char** pFragText{nullptr};
		int vertNumLines{0};
		int geomNumLines{0};
		int fragNumLines{0};
	};

	class ICoreShader : public IResource
	{
	};

	class ICoreTexture : public IResource
	{
	};

	class ICoreRender : public ISubSystem
	{
	public:
		virtual API Init(const WinHandle* handle) = 0;
		virtual API CreateMesh(ICoreMesh *&pMesh, const MeshDataDesc &dataDesc, const MeshIndexDesc &indexDesc, VERTEX_TOPOLOGY mode) = 0;
		virtual API CreateShader(ICoreShader *&pShader, const ShaderText& shaderDesc) = 0;
		virtual API SetShader(const ICoreShader *pShader) = 0;
		virtual API SetUniform(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type) = 0;
		virtual API SetUniformArray(const char *name, const void *pData, const ICoreShader *pShader, SHADER_VARIABLE_TYPE type, uint number) = 0;
		virtual API SetMesh(const ICoreMesh* mesh) = 0;
		virtual API Draw(ICoreMesh *mesh) = 0;
		virtual API SetDepthState(int enabled) = 0;
		virtual API SetViewport(uint w, uint h) = 0;
		virtual API Clear() = 0;
		virtual API SwapBuffers() = 0;
		virtual API Free() = 0;
	};


	//////////////////////
	// Game Objects
	//////////////////////
	
	class IGameObject : public IResource
	{
	public:
		virtual API SetPosition(const vec3& pos) = 0;
		virtual API SetRotation(const vec3& rot) = 0;
		virtual API GetModelMatrix(mat4& mat) = 0;
	};

	class ICamera : public IGameObject
	{
	public:
		virtual API GetViewProjectionMatrix(mat4& mat, float aspect) = 0;
	};

	class IModel : public IGameObject
	{
	public:
		virtual API GetMesh(ICoreMesh *&pMesh, uint idx) = 0;
		virtual API GetMeshesNumber(uint& number) = 0;
	};

	//////////////////////
	// Scene Manager
	//////////////////////
	class ISceneManager : public ISubSystem
	{
	public:
		virtual API AddGameObject(IGameObject* pGameObject) = 0;
		virtual API GetGameObjectsNumber(uint& number) = 0;
		virtual API GetGameObject(IGameObject *&pGameObject, uint idx) = 0;
		virtual API GetCamera(ICamera *&pCamera) = 0;
	};


	//////////////////////
	// Resource Manager
	//////////////////////

	enum class DEFAULT_RES_TYPE
	{
		NONE,
		PLANE,
		AXES
	};

	class IProgressSubscriber
	{
	public:
		virtual API ProgressChanged(uint i) = 0;
	};

	class IResourceManager : public ISubSystem
	{
	public:

		virtual API LoadModel(IModel *&pMesh, const char *pFileName, IProgressSubscriber *pProgress) = 0;
		virtual API LoadShaderText(ShaderText &pShader, const char* pVertName, const char* pGeomName, const char* pFragName) = 0;
		virtual API GetDefaultResource(IResource *&pRes, DEFAULT_RES_TYPE type) = 0;
		virtual API GetNumberOfResources(uint& number) = 0;
		virtual API AddToList(IResource *pResource) = 0;
		virtual API GetRefNumber(IResource *pResource, uint& number) = 0;
		virtual API DecrementRef(IResource *pResource) = 0;
		virtual API RemoveFromList(IResource *pResource) = 0;
		virtual API FreeAllResources() = 0;
	};

	
	//////////////////////
	// Filesystem
	//////////////////////

	enum class FILE_OPEN_MODE
	{
		READ = 0b000000000000000000000001,
		WRITE = 0b000000000000000000000010,
		APPEND = 0b000000000000000000000100,
		BINARY = 0b000000000000000000001000,
	};
	DEFINE_ENUM_OPERATORS(FILE_OPEN_MODE)

	class IFile
	{
	public:

		virtual API Read(uint8 *pMem, uint bytes) = 0;
		virtual API ReadStr(char *pStr, uint& str_bytes) = 0;
		virtual API IsEndOfFile(int &eof) = 0;
		virtual API Write(const uint8 *pMem, uint bytes) = 0;
		virtual API WriteStr(const char *pStr) = 0;
		virtual API FileSize(uint &size) = 0;
		virtual API CloseAndFree() = 0;
	};

	class IFileSystem : public ISubSystem
	{
	public:

		virtual API OpenFile(IFile *&pFile, const char *fullPath, FILE_OPEN_MODE mode) = 0;
		virtual API FileExist(const char *fullPath, int &exist) = 0;
		virtual API GetName(const char *&pName) = 0;
	};


	
	//////////////////////
	// COM stuff
	//////////////////////

	namespace
	{
		const TCHAR *pErrorMessage = TEXT("success");
	}

	inline bool GetCore(ICore*& pCore)
	{
		//cout << "Initializing COM" << endl;

		if (FAILED(CoInitialize(NULL)))
		{
			pErrorMessage = TEXT("Unable to initialize COM");
			std::cout << pErrorMessage << std::endl;
			return false;
		}

		const char* szProgID = "RenderMaster.Component.1";
		WCHAR  szWideProgID[128];
		CLSID  clsid;
		long lLen = MultiByteToWideChar(CP_ACP,
			0,
			szProgID,
			(int)strlen(szProgID),
			szWideProgID,
			sizeof(szWideProgID));

		szWideProgID[lLen] = '\0';
		HRESULT hr;
		hr = ::CLSIDFromProgID(szWideProgID, &clsid);
		if (FAILED(hr))
		{
			//TCHAR buf[260];
			//swprintf(buf, TEXT("Unable to get CLSID from ProgID. HR = %02X"), hr);
			pErrorMessage = TEXT("Unable to get CLSID from ProgID RenderMaster.Component.1");
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << pErrorMessage << "HR = " << hr << std::endl;
			return false;
		}

		IClassFactory* pCFactory;
		// Get the class factory for the Math class

		hr = CoGetClassObject(clsid,
			CLSCTX_INPROC,
			NULL,
			IID_IClassFactory,
			(void**)&pCFactory);
		if (FAILED(hr))
		{
			pErrorMessage = TEXT("Failed to GetClassObject server instance. HR = ");
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << pErrorMessage << hr << std::endl;
			return false;
		}

		// using the class factory interface create an instance of the
		// component and return the IExpression interface.
		IUnknown* pUnk;
		hr = pCFactory->CreateInstance(NULL, IID_IUnknown, (void**)&pUnk);

		//// Release the class factory
		pCFactory->Release();

		if (FAILED(hr))
		{
			pErrorMessage = TEXT("Failed to create server instance. HR = ");
			std::cout.setf(std::ios::hex, std::ios::basefield);
			std::cout << pErrorMessage << hr << std::endl;
			return false;
		}

		//cout << "Instance created" << endl;

		pCore = NULL;
		hr = pUnk->QueryInterface(IID_Core, (LPVOID*)&pCore);
		pUnk->Release();

		//hr = CoCreateInstance(CLSID_Math,         // CLSID of coclass
		//	NULL,                    // not used - aggregation
		//	CLSCTX_ALL,    // type of server
		//	IID_IMath,          // IID of interface
		//	(void**)&pCore);

		if (FAILED(hr))
		{
			pErrorMessage = TEXT("QueryInterface() for IID_Core failed");
			std::cout << pErrorMessage << std::endl;
			return false;
		}
		return true;
	}

	inline void FreeCore(ICore *pCore)
	{
		pCore->Release();
		//cout << "Shuting down COM" << endl;
		CoUninitialize();
	}

}