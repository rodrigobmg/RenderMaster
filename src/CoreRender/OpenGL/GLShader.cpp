#include "pch.h"
#include "GLShader.h"
#include "Core.h"

extern Core *_pCore;
DEFINE_DEBUG_LOG_HELPERS(_pCore)
DEFINE_LOG_HELPERS(_pCore)

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

		//// create buffer
		//buffer.data = NULL;
		//buffer.size = size >> 2;
		//buffer.begin = 0;
		//buffer.end = size >> 2;
		//buffer.ubo_id = 0;
		//buffer.name = name;
		//buffer.parameters.clear();

		//// buffer parameters
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

		//	// append parameter
		//	BufferParameter &parameter = buffer.parameters.append();
		//	parameter.name = name;
		//	parameter.type = type;
		//	parameter.size = size;
		//	parameter.offset = offset >> 2;
		}

		//// find buffer
		//int index = -1;
		//for (int j = 0; index == -1 && j < buffers.size(); j++)
		//{
		//	int is_equal = 1;
		//	if (buffers[j].name != buffer.name)
		//		is_equal = 0;
		//	if (buffers[j].size != buffer.size)
		//		is_equal = 0;
		//	if (buffers[j].parameters.size() != buffer.parameters.size())
		//		is_equal = 0;
		//	for (int k = 0; is_equal && k < buffer.parameters.size(); k++)
		//	{
		//		if (buffers[j].parameters[k].name != buffer.parameters[k].name)
		//			is_equal = 0;
		//		if (buffers[j].parameters[k].type != buffer.parameters[k].type)
		//			is_equal = 0;
		//		if (buffers[j].parameters[k].size != buffer.parameters[k].size)
		//			is_equal = 0;
		//		if (buffers[j].parameters[k].offset != buffer.parameters[k].offset)
		//			is_equal = 0;
		//	}
		//	if (is_equal)
		//		index = j;
		//}

		//// create buffer
		//if (index == -1)
		//{
		//	index = buffers.size();
		//	buffer.data = new int[buffer.size];
		//	memset(buffer.data, 0, buffer.size * sizeof(int));
		//	glGenBuffers(1, &buffer.ubo_id);
		//	glBindBuffer(GL_UNIFORM_BUFFER, buffer.ubo_id);
		//	glBufferData(GL_UNIFORM_BUFFER, buffer.size << 2, NULL, GL_DYNAMIC_DRAW);
		//	glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer.ubo_id);
		//	glUniformBlockBinding(program_id, i, index);
		//	glBindBuffer(GL_UNIFORM_BUFFER, 0);
		//	buffers.append(buffer);
		//} else
		//	glUniformBlockBinding(program_id, i, index);

		//// append parameter
		//for (int j = 0; j < buffer.parameters.size(); j++)
		//{
		//	const String &name = buffer.parameters[j].name;
		//	Map<String, int>::Iterator it = parameter_names.find(name);
		//	if (it == parameter_names.end())
		//	{
		//		Parameter parameter;
		//		parameter.location = -1;
		//		parameter.index = index;
		//		parameter.size = buffer.parameters[j].size;
		//		parameter.offset = buffer.parameters[j].offset;
		//		parameter.cache_valid = 0;
		//		int id = parameters.append(parameter);
		//		parameter_names.append(name, id);
		//	} else
		//		Log::error("GLShader::compile(): collision of \"%s\" parameter\n", name.get());
		//}

		// program buffer
		//program_buffers.append(index);
	}
}

GLShader::~GLShader()
{
	if (_vertID != 0) {	glDeleteShader(_vertID); _vertID = 0; }
	if (_fragID != 0) {	glDeleteShader(_fragID); _fragID = 0; }
	if (_geomID != 0) { glDeleteShader(_geomID); _geomID = 0; }
	if (_programID != 0) { glDeleteProgram(_programID); _programID = 0; }
}

API GLShader::SetFloatParameter(const char * name, float value)
{
	return S_OK;
}

API GLShader::SetVec4Parameter(const char * name, const vec4 * value)
{
	return S_OK;
}

API GLShader::SetMat4Parameter(const char * name, const mat4 * value)
{
	return S_OK;
}

API GLShader::SetUintParameter(const char * name, uint value)
{
	return S_OK;
}

API GLShader::FlushParameters()
{
	return S_OK;
}

