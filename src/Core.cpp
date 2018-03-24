#include "Core.h"
#include "ResourceManager.h"
#include "DX11CoreRender.h"
#include "GLCoreRender.h"
#include "Wnd.h"
#include "Console.h"
#include "Events.h"

#include <iostream>
#include <fstream>


Core *_pCore;


std::string Core::_getFullLogPath()
{
	return std::string(_pDataPath) + "\\log.txt";
}

void Core::_main_loop()
{
	for (IUpdateCallback *callback : _update_callbacks)
		callback->Update();

	_pCoreRender->Clear();
	_pCoreRender->SwapBuffers();
}

void Core::_s_main_loop()
{
	_pCore->_main_loop();
}

Core::Core()
{
	_pCore = this;
	InitializeCriticalSection(&_cs);
}

Core::~Core()
{
	delete _pCoreRender;
	delete _pResMan;
	if (_pWnd) delete _pWnd;
	if (_pConsole) delete _pConsole;
	delete _evLog;
	delete _pDataPath;
}

API Core::Init(INIT_FLAGS flags, const char *pDataPath, WinHandle* externHandle)
{
	const bool createWindow = (flags & INIT_FLAGS::WINDOW_FLAG) != INIT_FLAGS::EXTERN_WINDOW;
	const bool createConsole = (flags & INIT_FLAGS::CREATE_CONSOLE_FLAG) == INIT_FLAGS::CREATE_CONSOLE;

	auto size = strlen(pDataPath);
	_pDataPath = new char[size + 1];
	strcpy(_pDataPath, pDataPath);

	std::ofstream log(_getFullLogPath());
	log.close();

	_evLog = new EventLog;

	Log("Start initialization engine...");

	_pResMan = new ResourceManager;	

	if (createWindow)
	{
		_pWnd = new Wnd(_s_main_loop);
		_pWnd->CreateAndShow();
	}

	if (createConsole)
	{
		_pConsole = new Console;
		_pConsole->Init(_pWnd->handle());
	}


	if ((flags & INIT_FLAGS::GRAPHIC_LIBRARY_FLAG) == INIT_FLAGS::DIRECTX11)
		_pCoreRender = new DX11CoreRender;
	else
		_pCoreRender = new GLCoreRender;

	if (createWindow)
		_pCoreRender->Init(_pWnd->handle());
	else
		_pCoreRender->Init(externHandle);

	_pResMan->Init();

	Log("Engine initialized");

	return S_OK;
}

API Core::GetSubSystem(ISubSystem *& pSubSystem, SUBSYSTEM_TYPE type)
{
	switch(type)
	{
		case SUBSYSTEM_TYPE::CORE_RENDER: pSubSystem = _pCoreRender; break;
		case SUBSYSTEM_TYPE::RESOURCE_MANAGER: pSubSystem = _pResMan; break;
		default: return S_FALSE;
	}

	return S_OK;
}

API Core::GetDataPath(const char *&pStr)
{
	pStr = _pDataPath;
	return S_OK;
}

API Core::Log(const char *pStr, LOG_TYPE type)
{
	EnterCriticalSection(&_cs);

	if (_pConsole)
		_pConsole->OutputTxt(pStr);

	std::ofstream log(_getFullLogPath(), std::ios::out | std::ios::app);
	log << pStr << std::endl;
	log.close();

	std::cout << pStr << std::endl;

	_evLog->Fire(pStr, type);

	LeaveCriticalSection(&_cs);

	return S_OK;
}

API Core::AddInitCallback(IInitCallback* pCallback)
{
	_init_callbacks.push_back(pCallback);
	return S_OK;
}

API Core::AddUpdateCallback(IUpdateCallback* pCallback)
{
	_update_callbacks.push_back(pCallback);
	return S_OK;
}

API Core::Start()
{
	for (IInitCallback *callback : _init_callbacks)
		callback->Init();

	if (_pWnd)
		_pWnd->StartMainLoop();

	return S_OK;
}

API Core::CloseEngine()
{
	Log("Core::CloseEngine()");

	_pResMan->FreeAllResources();

	_pCoreRender->Free();

#ifdef _DEBUG
	//system("pause");
#endif

	if (_pConsole)
		_pConsole->Destroy();

	if (_pWnd)
		_pWnd->Destroy();

	return S_OK;
}

API Core::GetLogPrintedEv(ILogEvent*& pEvent)
{
	pEvent = _evLog;
	return S_OK;
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
	Core* pCore;
	HRESULT hr;

	*ppvObj = 0;

	pCore = new Core;

	if (pCore == 0)
		return(E_OUTOFMEMORY);

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

