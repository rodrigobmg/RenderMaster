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
	INSTANCE_IN
END_STRUCT

UNIFORM_BUFFER_BEGIN(viewport_parameters)
	UNIFORM(float, invWidth2)
	UNIFORM(float, invHeight2)
UNIFORM_BUFFER_END

STRUCT(Char)
	vec4 data;
	uint id;
	vec3 __dummy;
	END_STRUCT

STRUCTURED_BUFFER_IN(0, buffer, Char)

MAIN_VERTEX(VS_INPUT, VS_OUTPUT)

	uint left_padding = 10;
	uint top_padding = 32;
	float ww = 32.0f;
	float hh = 32.0f;
	uint chars_in_row = 16u;
	float tex_size = 512.0f;
	
	float w = buffer[INSTANCE].data.x;
	float h = buffer[INSTANCE].data.y;

	vec2 pos = IN_ATTRIBUTE(PositionIn).xy;

	vec2 vtx = pos * 0.5f + vec2(0.5f, 0.5f); // [-1, 1] -> [0, 1]

	vec2 ndc = 
		vec2(-1, 1) +
		vec2(invWidth2 * left_padding, -invHeight2 * top_padding) +
		vec2(invWidth2 * h, 0.0f) + 
		vec2(invWidth2 * w, invHeight2 * hh) * vtx;
	
	OUT_POSITION = vec4(ndc, 0.0f, 1.0f);

	vtx.y = 1.0f - vtx.y;

	uint id = buffer[INSTANCE].id - 32;
	float uv_x = float(id % chars_in_row) * ww;
	float uv_y = float(id / chars_in_row) * hh;

	uv_x += vtx.x * w;
	uv_y += vtx.y * hh;

	uv_x = uv_x / tex_size;
	uv_y = uv_y / tex_size;

	#ifdef ENG_INPUT_TEXCOORD
		OUT_ATTRIBUTE(TexCoord) = vec2(uv_x, uv_y);
	#endif

MAIN_VERTEX_END

#else // ENG_SHADER_PIXEL

///////////////////////
// PIXEL SHADER
///////////////////////

float luma(vec3 col)
{
	return 0.299*col.r + 0.587*col.g + 0.114*col.b;
}

#ifdef ENG_INPUT_TEXCOORD
	TEXTURE2D_IN(0, TEX_ALBEDO)
#endif

MAIN_FRAG(VS_OUTPUT)

	vec3 tex = TEXTURE(0, GET_ATRRIBUTE(TexCoord)).rgb;

	OUT_COLOR = vec4(vec3(1, 1, 1) * tex, luma(tex));
	//OUT_COLOR = vec4(1,1,0,1);

MAIN_FRAG_END

#endif
