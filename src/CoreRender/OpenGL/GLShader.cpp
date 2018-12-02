#include "pch.h"
#include "GLShader.h"
#include "GLCoreRender.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

extern vector<UBO> allUBO;

GLShader::GLShader(GLuint programID, GLuint vertID, GLuint geomID, GLuint fragID) : 
	_programID(programID), _vertID(vertID), _geomID(geomID), _fragID(fragID)
{
	// Get all unoform variables from all Uniform Buffer Objects

	glUseProgram(_programID);

	GLint numUBO = 0;
	glGetProgramiv(_programID, GL_ACTIVE_UNIFORM_BLOCKS, &numUBO);

	for (int i = 0; i < numUBO; i++)
	{
		char nameUBO[1024];
		GLint bytesUBO;
		GLint numIndices, indicesArray[2048];

		glGetActiveUniformBlockName(_programID, i, sizeof(nameUBO), NULL, nameUBO);
		glGetActiveUniformBlockiv(_programID, i, GL_UNIFORM_BLOCK_DATA_SIZE, &bytesUBO);
		glGetActiveUniformBlockiv(_programID, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numIndices);

		if (numIndices == 0)
			continue;

		glGetActiveUniformBlockiv(_programID, i, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indicesArray);

		// check uniform block reference
		static const GLuint shader_types[] = {
			GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER,
			GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_CONTROL_SHADER,
			GL_UNIFORM_BLOCK_REFERENCED_BY_TESS_EVALUATION_SHADER,
			GL_UNIFORM_BLOCK_REFERENCED_BY_GEOMETRY_SHADER,
			GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER,
			GL_UNIFORM_BLOCK_REFERENCED_BY_COMPUTE_SHADER,
		};
		GLint referenced = 0;
		for (int j = 0; j < (int)(sizeof(shader_types) / sizeof(shader_types[0])); j++)
		{
			glGetActiveUniformBlockiv(_programID, i, shader_types[j], &referenced);
			if (referenced > 0)
				break;
		}

		if (referenced == 0)
			continue;

		vector<UBO::UBOParameter> paramsUBO;

		// UBO parameters
		for (int j = 0; j < numIndices; j++)
		{
			GLuint index = (GLuint)indicesArray[j];

			uint bytesVariable;

			GLint elementsNum;
			GLenum typeVariable;
			char nameVariable[1024];
			glGetActiveUniform(_programID, index, sizeof(nameVariable), NULL, &elementsNum, &typeVariable, nameVariable);
			
			GLint offsetVariable;						
			glGetActiveUniformsiv(_programID, 1, &index, GL_UNIFORM_OFFSET, &offsetVariable);

			char *s = strchr(nameVariable, '[');
			if (s != nullptr)
				*s = '\0';

			switch (typeVariable)
			{
				case GL_FLOAT:				bytesVariable = elementsNum * 4;	break;
				case GL_FLOAT_VEC2:			bytesVariable = elementsNum * 8;	break;
				case GL_FLOAT_VEC3:			bytesVariable = elementsNum * 12;	break;
				case GL_FLOAT_VEC4:			bytesVariable = elementsNum * 16;	break;
				case GL_DOUBLE:				bytesVariable = elementsNum * 8;	break;
				case GL_DOUBLE_VEC2:		bytesVariable = elementsNum * 16;	break;
				case GL_DOUBLE_VEC3:		bytesVariable = elementsNum * 24;	break;
				case GL_DOUBLE_VEC4:		bytesVariable = elementsNum * 32;	break;
				case GL_INT:				bytesVariable = elementsNum * 4;	break;
				case GL_INT_VEC2:			bytesVariable = elementsNum * 8;	break;
				case GL_INT_VEC3:			bytesVariable = elementsNum * 12;	break;
				case GL_INT_VEC4:			bytesVariable = elementsNum * 16;	break;
				case GL_UNSIGNED_INT:		bytesVariable = elementsNum * 4;	break;
				case GL_UNSIGNED_INT_VEC2:	bytesVariable = elementsNum * 8;	break;
				case GL_UNSIGNED_INT_VEC3:	bytesVariable = elementsNum * 12;	break;
				case GL_UNSIGNED_INT_VEC4:	bytesVariable = elementsNum * 16;	break;
				case GL_BOOL_VEC2:			bytesVariable = elementsNum * 8;	break;
				case GL_BOOL_VEC3:			bytesVariable = elementsNum * 12;	break;
				case GL_BOOL_VEC4:			bytesVariable = elementsNum * 16;	break;
				case GL_FLOAT_MAT3:			bytesVariable = elementsNum * 36;	break;
				case GL_FLOAT_MAT4:			bytesVariable = elementsNum * 64;	break;
				case GL_DOUBLE_MAT3:		bytesVariable = elementsNum * 72;	break;
				case GL_DOUBLE_MAT4:		bytesVariable = elementsNum * 128;	break;
				default: assert(0 && "GLShader::GLShader(): unknown parameter type");
			}


			UBO::UBOParameter p;
			p.name = nameVariable;
			p.bytes = bytesVariable;
			p.offset = offsetVariable;
			p.elements = elementsNum;

			paramsUBO.push_back(p);
		}

		// sort by offset for pretty view
		#ifdef _DEBUG
			std::sort(paramsUBO.begin(), paramsUBO.end(), [](const UBO::UBOParameter& a, const UBO::UBOParameter& b) -> bool
			{ 
				return a.offset < b.offset; 
			});
		#endif

		// find buffer
		int indexFound = -1;
		{
			for (int j = 0; indexFound == -1 && j < allUBO.size(); j++)
			{
				bool isEqual = true;

				if (allUBO[j].name != nameUBO)
					isEqual = false;

				if (allUBO[j].bytes != bytesUBO)
					isEqual = false;

				if (allUBO[j].parameters.size() != paramsUBO.size())
					isEqual = false;

				for (int k = 0; isEqual && k < paramsUBO.size(); k++)
				{
					if (allUBO[j].parameters[k].name != paramsUBO[k].name ||
						allUBO[j].parameters[k].bytes != paramsUBO[k].bytes ||
						allUBO[j].parameters[k].offset != paramsUBO[k].offset ||
						allUBO[j].parameters[k].elements != paramsUBO[k].elements)
					{
						isEqual = 0;
					}
				}

				if (isEqual)
					indexFound = j;
			}
		}		

		if (indexFound == -1)
		{
			GLuint id;

			glGenBuffers(1, &id);
			glBindBuffer(GL_UNIFORM_BUFFER, id);
			vector<char> data(bytesUBO, '\0');
			glBufferData(GL_UNIFORM_BUFFER, bytesUBO, &data[0], GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			for (int i = 0; i < paramsUBO.size(); i++)
			{
				_parameters[paramsUBO[i].name] = {(uint)allUBO.size(), (uint)i};
			}

			allUBO.emplace_back(std::move(UBO(id, bytesUBO, nameUBO, paramsUBO)));
		} else
		{
			for (int i = 0; i < paramsUBO.size(); i++)
			{
				_parameters[paramsUBO[i].name] = {(uint)indexFound, (uint)i};
			}
		}			
	}
}

GLShader::~GLShader()
{
	if (_vertID != 0) {	glDeleteShader(_vertID); _vertID = 0; }
	if (_fragID != 0) {	glDeleteShader(_fragID); _fragID = 0; }
	if (_geomID != 0) { glDeleteShader(_geomID); _geomID = 0; }
	if (_programID != 0) { glDeleteProgram(_programID); _programID = 0; }
}

API GLShader::SetFloatParameter(const char *name, float value)
{
	return S_OK;
}

API GLShader::SetVec4Parameter(const char *name, const vec4 * value)
{
	return S_OK;
}

API GLShader::SetMat4Parameter(const char *name, const mat4 * value)
{
	return S_OK;
}

API GLShader::SetUintParameter(const char *name, uint value)
{
	return S_OK;
}

API GLShader::FlushParameters()
{
	return S_OK;
}

