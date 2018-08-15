// Constant buffer
CONSTANT_BUFFER_BEGIN(mesh_veretx)
	CONSTANT(vec4, main_color)	
#ifdef ENG_INPUT_NORMAL
	CONSTANT(vec3, nL)
	CONSTANT(mat4, NM)
#endif
	CONSTANT(mat4, MVP)
CONSTANT_BUFFER_END


// Iterpolated Attributes
STRUCT(VS_OUTPUT)
	INIT_POSITION
	#ifdef ENG_INPUT_NORMAL
		ATTRIBUTE(0, vec3, Normal, TEXCOORD)
	#endif
	#ifdef ENG_INPUT_TEXCOORD
		ATTRIBUTE(1, vec2, TexCoord, TEXCOORD)
	#endif
	#ifdef ENG_INPUT_COLOR
		ATTRIBUTE(2, vec4, Color, TEXCOORD)
	#endif
END_STRUCT
