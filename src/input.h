#include "raylib.h"

#ifndef INPUT_H_
#define INPUT_H_

typedef struct {
	KeyboardKey inc_key;
	KeyboardKey dec_key;
	
	GamepadAxis gp_axis;
	
	float value;
} DirectionInput;

typedef struct {
	short move_x, move_y;
	short look_x, look_y;
	
	bool jump;
	bool shoot;
	bool interact;
	bool pause;

	float move_timer;
	float look_timer;

	float jump_timer;
	float shoot_timer;
	float interact_timer;
	float pause_timer;
} InputState;

void ProcessInput(InputState *state, float delta_time);
void PollInputGamepad(InputState *state);
void PollInputKeyboard(InputState *state);

#endif
