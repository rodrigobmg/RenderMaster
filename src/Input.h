#pragma once
#include "Common.h"

class Input : public IInput
{
	static Input* instance;

	uint8 _keys[256]{};
	uint8 _mouse[3]{};
	int _cursorX{}, _cursorY{};

	vec2 _oldPos;
	vec2 _mouseDeltaPos;

	void update();
	void clear_mouse();
	void messageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);
	static void s_messageCallback(WINDOW_MESSAGE type, uint32 param1, uint32 param2, void *pData);

public:

	Input();
	~Input();

	API IsKeyPressed(OUT int *isPressed, KEYBOARD_KEY_CODES key) override;
	API IsMoisePressed(OUT int *isPressed, MOUSE_BUTTON type) override;
	API GetMouseDeltaPos(OUT vec2 *dPos) override;
	API GetMousePos(OUT uint *x, OUT uint *y) override;
	API GetName(OUT const char **pName) override;
};
