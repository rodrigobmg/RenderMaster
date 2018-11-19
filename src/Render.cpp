#include "pch.h"
#include "Render.h"
#include "Core.h"
#include "SceneManager.h"
//#include "Preprocessor.h"
#include "simplecpp.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

/////////////////////////
// Render
/////////////////////////

IShader* Render::_get_shader(const ShaderRequirement &req)
{
	IShader *pShader = nullptr;

	auto it = _shaders_pool.find(req);

	if (it != _shaders_pool.end())
	{
		WRL::ComPtr<IShader>& shaderPtr = it->second;
		return shaderPtr.Get();
	}
	else
	{
		const auto process_shader = [&](const char *&ppTextOut, const char *ppTextIn, const string& fileNameIn, const string&& fileNameOut, int type) -> void
		{
			simplecpp::DUI dui;

			if (is_opengl())
				dui.defines.push_back("ENG_OPENGL");
			else
				dui.defines.push_back("ENG_DIRECTX11");

			if (type == 0)
				dui.defines.push_back("ENG_SHADER_VERTEX");
			else if (type == 1)
				dui.defines.push_back("ENG_SHADER_PIXEL");
			else if (type == 2)
				dui.defines.push_back("ENG_SHADER_GEOMETRY");

			if ((int)(req.attributes & INPUT_ATTRUBUTE::NORMAL)) dui.defines.push_back("ENG_INPUT_NORMAL");
			if ((int)(req.attributes & INPUT_ATTRUBUTE::TEX_COORD)) dui.defines.push_back("ENG_INPUT_TEXCOORD");
			if ((int)(req.attributes & INPUT_ATTRUBUTE::COLOR)) dui.defines.push_back("ENG_INPUT_COLOR");
			if (req.alphaTest) dui.defines.push_back("ENG_ALPHA_TEST");

			// TODO: move to Common.h or filesystem
			const char *pString;
			_pCore->GetInstalledDir(&pString);
			string installedDir = string(pString);
			string fullPath = installedDir + '\\' + SHADER_DIR + '\\' + fileNameIn;

			simplecpp::OutputList outputList;
			std::vector<std::string> files;
			string textIn = ppTextIn;
			std::stringstream f(textIn);
			std::map<std::string, simplecpp::TokenList*> included;

			simplecpp::TokenList rawtokens(f, files, fullPath, &outputList);

			simplecpp::TokenList outputTokens(files);

			simplecpp::preprocess(outputTokens, rawtokens, files, included, dui, &outputList);
			const string out = outputTokens.stringify();
			auto size = out.size();

			// Workaround for opengl because C preprocessor eats up #version 420
			if (is_opengl())
				size += 13;

			char *tmp = new char[size + 1];
			if (is_opengl())
			{
				strncpy(tmp + 0, "#version 420\n", 13);
				strncpy(tmp + 13, out.c_str(), size - 13);
			} else
				strncpy(tmp, out.c_str(), size);

			tmp[size] = '\0';

			ppTextOut = tmp;
		};

		const char *text;
		_standardShader->GetText(&text);

		const char *pFileIn;
		_standardShader->GetFile(&pFileIn);
		string fileIn = pFileIn;

		const char *textVertOut; 
		const char *textFragOut;

		process_shader(textVertOut, text, fileIn, "out_v.shader", 0);
		process_shader(textFragOut, text, fileIn, "out_f.shader", 1);

		bool compiled = SUCCEEDED(_pResMan->CreateShader(&pShader, textVertOut, nullptr, textFragOut)) && pShader != nullptr;

		if (!compiled)
			LOG_FATAL("Render::_get_shader(): can't compile standard shader\n");
		else
			_shaders_pool.emplace(req, WRL::ComPtr<IShader>(pShader));
	}

	return pShader;
}

bool Render::is_opengl()
{
	const char *gapi;
	_pCoreRender->GetName(&gapi);
	return (strcmp("GLCoreRender", gapi) == 0);	
}

void Render::_create_render_mesh_vec(vector<TRenderMesh>& meshes_vec)
{
	SceneManager *sm = (SceneManager*)_pSceneMan;	

	for (tree<IGameObject*>::iterator it = sm->_gameobjects.begin(); it != sm->_gameobjects.end(); ++it)
	{
		IGameObject *go = *it;		
		IModel *model = dynamic_cast<IModel*>(go);
		if (model)
		{
			uint meshes;
			model->GetNumberOfMesh(&meshes);

			for (auto j = 0u; j < meshes; j++)
			{
				ICoreMesh *mesh{ nullptr };
				model->GetCoreMesh(&mesh, j);

				mat4 mat;
				model->GetModelMatrix(&mat);

				meshes_vec.push_back({mesh, mat});
			}
		}
	}
}

void Render::_sort_meshes(vector<TRenderMesh>& meshes)
{
	// not impl
}

Render::Render(ICoreRender *pCoreRender) : _pCoreRender(pCoreRender)
{
	_pCore->GetSubSystem((ISubSystem**)&_pSceneMan, SUBSYSTEM_TYPE::SCENE_MANAGER);
	_pCore->GetSubSystem((ISubSystem**)&_pResMan, SUBSYSTEM_TYPE::RESOURCE_MANAGER);
	_pCore->GetSubSystem((ISubSystem**)&_fsystem, SUBSYSTEM_TYPE::FILESYSTEM);
}

Render::~Render()
{
}

void Render::Init()
{
	IShaderText *st;
	_pResMan->LoadShaderText(&st, "mesh.shader");
	_standardShader =  WRL::ComPtr<IShaderText>(st);

	IMesh *mesh;

	_pResMan->LoadMesh(&mesh, "std#axes");
	_axesMesh = WRL::ComPtr<IMesh>(mesh);

	_pResMan->LoadMesh(&mesh, "std#axes_arrows");
	_axesArrowMesh = WRL::ComPtr<IMesh>(mesh);

	_pResMan->LoadMesh(&mesh, "std#grid");
	_gridMesh = WRL::ComPtr<IMesh>(mesh);

	IConstantBuffer *cb;
	_pResMan->CreateConstantBuffer(&cb, sizeof(EveryFrameParameters));
	_everyFrameParameters = WRL::ComPtr<IConstantBuffer>(cb);

	LOG("Render initialized");
}

void Render::Free()
{
	_standardShader.Reset();
	_axesMesh.Reset();
	_axesArrowMesh.Reset();
	_gridMesh.Reset();
	_everyFrameParameters.Reset();

	_shaders_pool.clear();	
}

void Render::RenderFrame(const ICamera *pCamera)
{
	vector<TRenderMesh> meshes;
	
	_pCoreRender->Clear();

	uint w, h;
	_pCoreRender->GetViewport(&w, &h);

	float aspect = (float)w / h;

	mat4 VP;
	const_cast<ICamera*>(pCamera)->GetViewProjectionMatrix(&VP, aspect);

	_create_render_mesh_vec(meshes);
	_sort_meshes(meshes);

	for(TRenderMesh &renderMesh : meshes)
	{
		INPUT_ATTRUBUTE a;
		renderMesh.mesh->GetAttributes(&a);		
		IShader *shader = _get_shader({a, false});
		ICoreShader *coreShader;
		shader->GetCoreShader(&coreShader);

		if (!shader)
			continue;

		_pCoreRender->SetMesh(renderMesh.mesh);
		_pCoreRender->SetShader(coreShader);

		//
		// parameters
		params.MVP = VP * renderMesh.modelMat;
		params.main_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		if ((int)(a & INPUT_ATTRUBUTE::NORMAL))
		{
			params.NM = mat4();
			params.nL = vec3(1.0f, -2.0f, 3.0f).Normalized();
		}

		ICoreConstantBuffer *coreCB;
		_everyFrameParameters->GetCoreBuffer(&coreCB);

		_pCoreRender->SetConstantBufferData(coreCB, &params.main_color);
		_pCoreRender->SetConstantBuffer(coreCB, 0);

		_pCoreRender->Draw(renderMesh.mesh);

		// debug
		//{
		//	INPUT_ATTRUBUTE a;
		//	_pGridMesh->GetAttributes(&a);

		//	ICoreShader *shader{nullptr};
		//	ShaderRequirement req = {a, false};
		//	shader = _get_shader(req);
		//	if (!shader) return;
		//	_pCoreRender->SetShader(shader);

		//	params.MVP = VP;
		//	params.main_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);

		//	_pCoreRender->SetUniform(everyFrameParameters, &params.main_color);
		//	_pCoreRender->SetConstantBuffer(everyFrameParameters, 0);


		//	_pCoreRender->SetDepthState(true);

		//	_pCoreRender->SetMesh(_pGridMesh);
		//	_pCoreRender->Draw(_pGridMesh);

		//}
	}
}

API Render::ShadersReload()
{
	LOG("Shaders reloading...");
	_standardShader->Reload();
	_shaders_pool.clear();
	return S_OK;
}

API Render::PreprocessStandardShader(IShader** pShader, const ShaderRequirement* shaderReq)
{
	*pShader = _get_shader(*shaderReq);
	return S_OK;
}

API Render::GetName(const char** pName)
{
	*pName = "Render";
	return S_OK;
}
