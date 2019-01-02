#include "pch.h"
#include "Common.h"
#include "Core.h"
#include "Filesystem.h"
#include "ResourceManager.h"
#include "DX11CoreRender.h"
#include "GLCoreRender.h"
#include "MainWindow.h"
#include "Console.h"
#include "Render.h"
#include "SceneManager.h"
#include "Input.h"

using std::wstring;

Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)


Core::Core(const mchar *pWorkingDir, const mchar *pInstalledDir)
{
	_pCore = this;
	
	InitializeCriticalSection(&_cs);

	_pWorkingDir = NativeToUTF8(pWorkingDir);
	_pInstalledDir = NativeToUTF8(pInstalledDir);

	_pfSystem = std::make_unique<FileSystem>();
	_pConsole = std::make_unique<Console>();
	_pResMan = std::make_unique<ResourceManager>();
}

Core::~Core()
{
}

API Core::Init(INIT_FLAGS flags, const mchar *pDataPath, const WindowHandle* externHandle)
{
	_pDataDir = NativeToUTF8(pDataPath);

	if (is_relative(_pDataDir.c_str()))
		_pDataDir = make_absolute(_pDataDir.c_str(), _pInstalledDir.c_str());

	int exist = 0;
	_pfSystem->DirectoryExist(_pDataDir.c_str(), &exist);

	if (!exist)
	{
		std::cout << "Core::Init(): directory" << _pDataDir.c_str() << " doesn't exist\n" << std::endl;
		return E_ABORT;
	}

	const bool createConsole = (flags & INIT_FLAGS::CREATE_CONSOLE_FLAG) == INIT_FLAGS::CREATE_CONSOLE;
	_pConsole->Init(createConsole);

	Log("Start initialization engine...");
	LogFormatted("Working directory:    %s", LOG_TYPE::NORMAL, _pWorkingDir.c_str());
	LogFormatted("Installed directory:  %s", LOG_TYPE::NORMAL, _pInstalledDir.c_str());
	LogFormatted("Data directory:       %s", LOG_TYPE::NORMAL, _pDataDir.c_str());

	const bool createWindow = (flags & INIT_FLAGS::WINDOW_FLAG) != INIT_FLAGS::EXTERN_WINDOW;
	if (createWindow)
	{
		_pMainWindow = std::make_unique<MainWindow>(s_mainLoop);
		_pMainWindow->AddMessageCallback(s_messageCallback);
		_pMainWindow->CreateAndShow();
	}

	_pfSystem->Init(string(_pDataDir));

	_pInput = std::make_unique<Input>();

	if ((flags & INIT_FLAGS::GRAPHIC_LIBRARY_FLAG) == INIT_FLAGS::DIRECTX11)
		_pCoreRender = std::make_unique<DX11CoreRender>();
	else
		_pCoreRender = std::make_unique<GLCoreRender>();

	int VSync = 1;
	if (int(flags & INIT_FLAGS::VSYNC_FLAG))
		VSync = (flags & INIT_FLAGS::VSYNC_FLAG) == INIT_FLAGS::VSYNC_ON;

	if (createWindow)
		_pCoreRender->Init(_pMainWindow->handle(), get_msaa_samples(flags), VSync);
	else
		_pCoreRender->Init(externHandle, get_msaa_samples(flags), VSync);

	_pResMan->Init();

	_pSceneManager = std::make_unique<SceneManager>();
	_pSceneManager->Init();

	_pRender = std::make_unique<Render>(_pCoreRender.get());
	_pRender->Init();

	if (createWindow)
		_pMainWindow->Show();

	_updateCallbacks.push_back(std::bind(&Core::_update, this));

	Log("Engine initialized");

	return S_OK;
}

API Core::Start()
{
	for (IInitCallback *callback : _initCallbacks)
		callback->Init();

	//timer_fps.Start();

	if (_pMainWindow)
	{
		uint w, h;
		_pMainWindow->GetDimension(w, h);
		_pCoreRender->SetViewport(w, h);
		_pMainWindow->StartMainLoop();
	}

	return S_OK;
}

API Core::Update()
{
	internalUpdate();
	return S_OK;
}

API Core::RenderFrame(const WindowHandle* extern_handle, const ICamera *pCamera, const FrameMode *mode)
{
	_pCoreRender->MakeCurrent(extern_handle);

#ifdef WIN32
	RECT r;
	GetWindowRect(*extern_handle, &r);
	int w = r.right - r.left;
	int h = r.bottom - r.top;
#else
	assert(false); // not impl
#endif // WIN32
	
	_pCoreRender->SetViewport(w, h);
	_pRender->RenderFrame(pCamera, (int64_t)extern_handle, mode);

	return S_OK;
}

API Core::GetSubSystem(OUT ISubSystem **pSubSystem, SUBSYSTEM_TYPE type)
{
	switch(type)
	{
		case SUBSYSTEM_TYPE::CORE_RENDER: *pSubSystem = _pCoreRender.get(); break;
		case SUBSYSTEM_TYPE::RESOURCE_MANAGER: *pSubSystem = _pResMan.get(); break;
		case SUBSYSTEM_TYPE::FILESYSTEM: *pSubSystem = _pfSystem.get(); break;
		case SUBSYSTEM_TYPE::INPUT: *pSubSystem = _pInput.get(); break;
		case SUBSYSTEM_TYPE::SCENE_MANAGER: *pSubSystem = _pSceneManager.get(); break;
		case SUBSYSTEM_TYPE::RENDER: *pSubSystem = _pRender.get(); break;
		case SUBSYSTEM_TYPE::CONSOLE: *pSubSystem = _pConsole.get(); break;
		default:
			LOG_WARNING("Core::GetSubSystem() unknown subsystem");
			return E_FAIL;
	}

	return S_OK;
}

void Core::mainLoop()
{
	internalUpdate();

	ICamera *cam;
	_pSceneManager->GetDefaultCamera(&cam);

	_pRender->RenderFrame(cam, 0, &defaultMode);
	_pCoreRender->SwapBuffers();
}

void Core::s_mainLoop()
{
	_pCore->mainLoop();
}

void Core::messageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void* pData)
{
	switch (type)
	{
	case WINDOW_MESSAGE::SIZE:
		if (_pCoreRender)
			_pCoreRender->SetViewport(param1, param2);
		//LogFormatted("Window size changed: x=%i y=%i", LOG_TYPE::NORMAL, param1, param2);
		break;

	case WINDOW_MESSAGE::WINDOW_UNMINIMIZED:
		if (_pMainWindow)
			_pMainWindow->SetPassiveMainLoop(0);
		if (_pConsole)
			_pConsole->Show();
		break;

	case WINDOW_MESSAGE::WINDOW_MINIMIZED:
		if (_pMainWindow)
			_pMainWindow->SetPassiveMainLoop(1);
		if (_pConsole)
			_pConsole->Hide();
		break;

	case WINDOW_MESSAGE::WINDOW_REDRAW:
		mainLoop();
		break;

	case WINDOW_MESSAGE::APPLICATION_ACTIVATED:
		if (_pMainWindow)
		{			
			_pMainWindow->SetPassiveMainLoop(0);
		}
		break;

	case WINDOW_MESSAGE::APPLICATION_DEACTIVATED:
		if (_pMainWindow)
		{
			setWindowCaption(1, 0);
			_pMainWindow->SetPassiveMainLoop(1);
		}
		break;

	case WINDOW_MESSAGE::WINDOW_CLOSE:
		if (_pConsole)
			_pConsole->BringToFront();
		break;

	default:
		break;
	}
}

void Core::s_messageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void* pData)
{
	_pCore->messageCallback(type, param1, param2, pData);
}

void Core::setWindowCaption(int is_paused, int fps)
{
	if (!_pMainWindow)
		return;

	wstring title = wstring(L"Test");

	if (is_paused)
		title += wstring(L" [Paused]");
	else
	{
		string fps_str = std::to_string(fps);
		title += wstring(L" [") + wstring(fps_str.begin(), fps_str.end()) + wstring(L"]");
	}

	_pMainWindow->SetCaption(title.c_str());
}

API Core::GetDataDir(OUT const char **pStr)
{
	*pStr = _pDataDir.c_str();
	return S_OK;
}

API Core::GetWorkingDir(OUT const char **pStr)
{
	*pStr = _pWorkingDir.c_str();
	return S_OK;
}

API Core::GetInstalledDir(OUT const char **pStr)
{
	*pStr = _pInstalledDir.c_str();
	return S_OK;
}

void Core::Log(const char *pStr, LOG_TYPE type)
{
	_pConsole->Log(pStr, type);
}

API Core::AddInitCallback(IInitCallback* pCallback)
{
	_initCallbacks.push_back(pCallback);
	return S_OK;
}

API Core::AddUpdateCallback(IUpdateCallback* pCallback)
{
	_updateCallbacks.push_back(std::bind(&IUpdateCallback::Update, pCallback));
	return S_OK;
}

API Core::ReleaseEngine()
{
	Log("Start closing engine...");

	if (_pMainWindow)
	{
		_pMainWindow->Destroy();
		_pMainWindow.reset();
	}

	_pSceneManager->Free();
	_pRender->Free();
	_pCoreRender->UnbindAllTextures();
	_pCoreRender->SetShader(nullptr);
	_pCoreRender->SetMesh(nullptr);
	// TODO: unbind all rendertargets
	_pResMan->Free();
	_pCoreRender->Free();

	_pSceneManager.reset();
	_pRender.reset();
	_pResMan.reset();
	_pCoreRender.reset();

	Log("Engine closed");

	if (_pConsole)
		_pConsole->Destroy();

	return S_OK;
}

void Core::internalUpdate()
{
	updateFPS();

	for (auto &callback : _updateCallbacks)
		callback();

	_frame++;
}

void Core::_update()
{
	if (!_pConsole)
		return;

	static int tilda = 0;
	int tilda_old = tilda;
	_pInput->IsKeyPressed(&tilda, KEYBOARD_KEY_CODES::KEY_GRAVE);
	if (tilda_old && !tilda)
	{
		int is_visible = _pConsole->IsVisible();
		if (is_visible)
			_pConsole->Hide();
		else
			_pConsole->Show();
	}
}

float Core::updateFPS()
{
	static const float upd_interv = 0.3f;
	static float accum = 0.0f;

	std::chrono::duration<float> _durationSec = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start);
	dt = _durationSec.count();

	accum += dt;

	if (accum > upd_interv)
	{
		accum = 0.0f;
		int fps = static_cast<int>(1.0f / dt);
		setWindowCaption(0, fps);		
	}

	start = std::chrono::steady_clock::now();

	return dt;
}

HRESULT Core::QueryInterface(REFIID riid, void ** ppv)
{
	if (riid == IID_Core || riid == IID_IUnknown)
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

ULONG Core::AddRef()
{
	InterlockedIncrement(&_lRef);
	return 0;
}

ULONG Core::Release()
{
	InterlockedDecrement(&_lRef);
	if (_lRef == 0)
	{
		delete this;
		return 0;
	}
	else
		return _lRef;
}

HRESULT CoreClassFactory::QueryInterface(REFIID riid, void ** ppv)
{
	*ppv = 0;

	if (riid == IID_IUnknown || riid == IID_IClassFactory)
		*ppv = this;

	if (*ppv)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG CoreClassFactory::AddRef()
{
	return InterlockedIncrement(&m_lRef);
}

STDMETHODIMP_(ULONG) CoreClassFactory::Release()
{
	if (InterlockedDecrement(&m_lRef) == 0)
	{
		delete this;
		return 0;
	}

	return m_lRef;
}

STDMETHODIMP CoreClassFactory::CreateInstance(LPUNKNOWN pUnk, REFIID riid, void ** ppvObj)
{
	ICore* pCore;
	HRESULT hr;
	*ppvObj = 0;
	
	// Recieve working directory from Registry
	
	auto get_registry_value = [](TCHAR *key) -> wstring
	{
		std::wstring BaseKey(TEXT("SOFTWARE\\Classes\\CLSID\\{A889F560-58E4-11d0-A68A-0000837E3100}\\InProcServer32\\"));

		DWORD buffer_size = 0;

		RegGetValue(HKEY_LOCAL_MACHINE, BaseKey.c_str(), key, RRF_RT_REG_SZ, 0, nullptr, // pvData == nullptr ? Request buffer size
			&buffer_size);

		const DWORD buffer_length = buffer_size / sizeof(WCHAR); // length in WCHAR's
		std::unique_ptr<WCHAR[]> text_buffer (new WCHAR[buffer_length]);

		RegGetValue(HKEY_LOCAL_MACHINE, BaseKey.c_str(), key, RRF_RT_REG_SZ, 0, text_buffer.get(), &buffer_size);

		return wstring(text_buffer.get());
	};
	
	wstring workingDir = get_registry_value(TEXT("WorkingDir"));
	wstring installedDir = get_registry_value(TEXT("InstalledDir"));

	pCore = new Core(workingDir.c_str(), installedDir.c_str());


	if (pCore == 0)
		return E_OUTOFMEMORY;

	hr = pCore->QueryInterface(riid, ppvObj);

	if (FAILED(hr))
		delete pCore;

	return hr;
}

STDMETHODIMP CoreClassFactory::LockServer(BOOL fLock)
{
	if (fLock)
		InterlockedIncrement(&g_lLocks1);
	else
		InterlockedDecrement(&g_lLocks1);

	return S_OK;
}

