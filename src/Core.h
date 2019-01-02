#pragma once
#include "Common.h"

class MainWindow;
class FileSystem;
class ResourceManager;
class Console;
class Render;
class SceneManager;

DEFINE_GUID(CLSID_Core,
	0xa889f560, 0x58e4, 0x11d0, 0xa6, 0x8a, 0x0, 0x0, 0x83, 0x7e, 0x31, 0x0);

class Core : public ICore
{
	string _pDataDir;
	string _pWorkingDir;
	string _pInstalledDir;

	// subsystems
	unique_ptr<Console> _pConsole;
	unique_ptr<MainWindow> _pMainWindow;
	unique_ptr<FileSystem>_pfSystem;
	unique_ptr<ResourceManager> _pResMan;
	unique_ptr<ICoreRender>_pCoreRender;
	unique_ptr<Render> _pRender;
	unique_ptr<SceneManager>_pSceneManager;
	unique_ptr<IInput> _pInput;

	CRITICAL_SECTION _cs{};

	vector<IInitCallback *> _initCallbacks;
	vector<std::function<void()>> _updateCallbacks;

	long _lRef{0};

	std::chrono::steady_clock::time_point start;

	void internalUpdate();
	void _update();
	float updateFPS();
	void mainLoop();
	void static s_mainLoop();
	void messageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	static void s_messageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	void setWindowCaption(int is_paused, int fps);

	int windowActive = 1;	

	int64_t _frame = 0;
	float dt = 0.0f;

	FrameMode defaultMode;

public:

	Core(const mchar *workingDir, const mchar *installedDir);
	virtual ~Core();

	template <typename... Arguments>
	void LogFormatted(const char *pStr, LOG_TYPE type, Arguments ...args)
	{
		static char buf[1000];
		assert(strlen(pStr) < 1000);
		sprintf(buf, pStr, args...);
		Log(buf, type);
	}
	MainWindow* mainWindow() { return _pMainWindow.get(); }
	void AddUpdateCallback(std::function<void()>&& calback) { _updateCallbacks.push_back(std::forward<std::function<void()>>(calback)); }
	float getDt() { return dt; }
	void Log(const char *pStr, LOG_TYPE type = LOG_TYPE::NORMAL);
	Console *getConsoole() { return _pConsole.get(); }
	int64_t frame() { return _frame; }

	API Init(INIT_FLAGS flags, const mchar *pDataPath, const WindowHandle* externHandle) override;
	API Start() override;
	API Update() override;
	API RenderFrame(const WindowHandle* externHandle, const ICamera *pCamera, const FrameMode *mode) override;
	API GetSubSystem(OUT ISubSystem **pSubSystem, SUBSYSTEM_TYPE type) override;
	API GetDataDir(OUT const char **pStr) override;
	API GetWorkingDir(OUT const char **pStr) override;
	API GetInstalledDir(OUT const char **pStr) override;
	API AddInitCallback(IInitCallback *pCallback) override;
	API AddUpdateCallback(IUpdateCallback *pCallback) override;
	API ReleaseEngine() override;

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

