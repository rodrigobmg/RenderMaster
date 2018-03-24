#pragma once
#include "Common.h"

class Wnd;
class ResourceManager;
class Console;
class EventLog;

DEFINE_GUID(CLSID_Core,
	0xa889f560, 0x58e4, 0x11d0, 0xa6, 0x8a, 0x0, 0x0, 0x83, 0x7e, 0x31, 0x0);

class Core : public ICore
{
	long _lRef{0};
	Console *_pConsole{nullptr};
	Wnd *_pWnd{nullptr};
	ResourceManager *_pResMan{nullptr};
	ICoreRender *_pCoreRender{nullptr};
	EventLog *_evLog{nullptr};
	CRITICAL_SECTION _cs;
	char *_pDataPath;
	std::vector<IInitCallback *> _init_callbacks;
	std::vector<IUpdateCallback *> _update_callbacks;
	std::string _getFullLogPath();

	void _main_loop();
	void static _s_main_loop();

public:

	Core();
	~Core();

	template <typename... Arguments>
	void LogFormatted(const char *pStr, LOG_TYPE type, Arguments ... args)
	{
		char buf[300];
		sprintf_s(buf, pStr, args...);
		Log(buf, type);
	}

	API Init(INIT_FLAGS flags, const char *pDataPath, WinHandle* handle) override;
	API GetSubSystem(ISubSystem *&pSubSystem, SUBSYSTEM_TYPE type) override;
	API GetDataPath(const char *&pStr) override;
	API Log(const char *pStr, LOG_TYPE type = LOG_TYPE::NORMAL) override;
	API AddInitCallback(IInitCallback *pCallback) override;
	API AddUpdateCallback(IUpdateCallback *pCallback) override;
	API Start() override;
	API CloseEngine() override;
	API GetLogPrintedEv(ILogEvent *&pEvent) override;

	STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;
};


class CoreClassFactory : public IClassFactory
{
	long m_lRef;
	long g_lLocks1;

public:

	CoreClassFactory() : m_lRef(0), g_lLocks1(0) {}

	// IUnknown 
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppv);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	// IClassFactory
	virtual STDMETHODIMP CreateInstance(LPUNKNOWN pUnk, REFIID riid, void** ppv);
	virtual STDMETHODIMP LockServer(BOOL fLock);
};

