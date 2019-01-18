#include "common/language.h"

// Iterpolated Attributes
STRUCT(VS_OUTPUT)
	INIT_POSITION
	#ifdef ENG_INPUT_TEXCOORD
		ATTRIBUTE(vec2, TexCoord, TEXCOORD2)
	#endif
END_STRUCT


#ifdef ENG_SHADER_VERTEX

// Input Attributes
STRUCT(VS_INPUT)
	ATTRIBUTE_VERETX_IN(0, vec4, PositionIn, POSITION0)
	#ifdef ENG_INPUT_TEXCOORD
		ATTRIBUTE_VERETX_IN(2, vec2, TexCoordIn, TEXCOORD2)
	#endif
END_STRUCT


MAIN_VERTEX(VS_INPUT, VS_OUTPUT)

	OUT_POSITION = vec4(IN_ATTRIBUTE(PositionIn).xy, 0.0f, 1.0f);

	#ifdef ENG_INPUT_TEXCOORD
		OUT_ATTRIBUTE(TexCoord) = IN_ATTRIBUTE(TexCoordIn);
	#endif

MAIN_VERTEX_END

#else // ENG_SHADER_PIXEL

///////////////////////
// PIXEL SHADER
///////////////////////

#ifdef ENG_INPUT_TEXCOORD
	TEXTURE2D_IN(0, TEX_ALBEDO)
#endif

MAIN_FRAG(VS_OUTPUT)

	float tex = TEXTURE(0, GET_ATRRIBUTE(TexCoord));

	OUT_COLOR = vec4(tex, tex, tex, 1.0f);

MAIN_FRAG_END

#endif
