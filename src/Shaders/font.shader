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

UNIFORM_BUFFER_BEGIN(viewport_parameters)
	UNIFORM(uint, width)
	UNIFORM(uint, height)
	UNIFORM(float, invWidth)
	UNIFORM(float, invHeight)
UNIFORM_BUFFER_END

STRUCT(Char)
	vec4 data;
	uint id;
	vec3 dummy;
END_STRUCT

STRUCTURED_BUFFER_IN(0, buffer, Char)

MAIN_VERTEX(VS_INPUT, VS_OUTPUT)

	vec2 pos = IN_ATTRIBUTE(PositionIn).xy;

	vec2 uv = pos * 0.5f + vec2(0.5f, 0.5f); // [0, 1] x [0, 1]

	float ww = 18.0f;
	float hh = 22.0f;

	vec2 pos_ = vec2(0.5f, 0.5f) + uv * vec2(2 *invWidth * (ww - 1), 2 *invHeight * (hh - 1));
	OUT_POSITION = vec4(pos_, 0.0f, 1.0f);

	uv.y = 1.0f - uv.y;

	uint id = buffer[0].id;
	id = id - 32;
	float x = float(id % 28u) * ww;
	float y = float(id / 28u) * hh;

	x += uv.x * (ww - 1.0f);
	y += uv.y * (hh - 1.0f);

	x = x / 512.0f;
	y = y / 256.0f;

	#ifdef ENG_INPUT_TEXCOORD
		OUT_ATTRIBUTE(TexCoord) = vec2(x, y);
		//OUT_ATTRIBUTE(TexCoord) = IN_ATTRIBUTE(TexCoordIn);
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

	float tex = TEXTURE(0, GET_ATRRIBUTE(TexCoord)).r;

	OUT_COLOR = vec4(1.0f, 1.0f, 1.0f, tex);

MAIN_FRAG_END

#endif
