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

	uint left_padding = 10;
	uint top_padding = 32;
	float ww = 32.0f;
	float hh = 32.0f;
	uint chars_in_row = 16u;
	float tex_size = 512.0f;
	
	float widthPixels = buffer[INSTANCE].data.x;
	float offsetPixels = buffer[INSTANCE].data.y;

	vec2 pos = IN_ATTRIBUTE(PositionIn).xy;

	vec2 uv = pos * 0.5f + vec2(0.5f, 0.5f); // [-1, 1] -> [0, 1]

	vec2 pos_ = vec2(-1, 1) + 
		vec2(2 * invWidth * left_padding, -2 * invHeight * top_padding) +
		vec2(2 * invWidth * offsetPixels, 0.0f) + 
		uv * vec2(2 * invWidth * (widthPixels + 0.0f), 2 * invHeight * hh);
	
	OUT_POSITION = vec4(pos_, 0.0f, 1.0f);

	uv.y = 1.0f - uv.y;

	uint id = buffer[INSTANCE].id;
	id = id - 32;
	float uv_x = float(id % chars_in_row) * ww;
	float uv_y = float(id / chars_in_row) * hh;

	uv_x += uv.x * (widthPixels + 0.0); // 0.5f need???
	uv_y += uv.y * hh;

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

	OUT_COLOR = vec4(tex, luma(tex));
	//OUT_COLOR = vec4(1,1,0,1);

MAIN_FRAG_END

#endif
