#include "../common/language.h"
#include "../common/common_post.h"

#ifdef ENG_SHADER_PIXEL

	///////////////////////
	// PIXEL SHADER
	///////////////////////	
	
	TEXTURE2D_IN(0, OLD_COLOR)
	TEXTURE2D_IN(1, CURRENT_COLOR)
	
	MAIN_FRAG(VS_OUTPUT)

		vec2 uv = GET_ATRRIBUTE(TexCoord);
		
		vec4 old_color = TEXTURE(0, uv);
		vec4 current_color = TEXTURE(1, uv);

		OUT_COLOR = lerp(old_color, current_color, 0.1f);
		
	MAIN_FRAG_END

#endif
