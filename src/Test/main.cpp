#include "Engine.h"

using namespace RENDER_MASTER;


int APIENTRY wWinMain(_In_ HINSTANCE _hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	ICore* pCore;

	if (GetCore(&pCore))
	{
		IResourceManager *resMan;

		if (SUCCEEDED(pCore->Init(INIT_FLAGS::CREATE_CONSOLE | INIT_FLAGS::OPENGL45, "resources", nullptr)))
		{
			pCore->GetSubSystem((ISubSystem**)&resMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);

			ResourcePtr<IModel> pModel = resMan->loadModel("box.fbx");

			pCore->Start(); // begin main loop
			pCore->CloseEngine();
		}

		FreeCore(pCore);
	}
	else
		MessageBox(nullptr, pErrorMessage, TEXT("Unable start engine"), MB_OK | MB_ICONERROR);

	return 0;
}



