#include "raylib.h"
#include "input.h"

void ProcessInput(InputState *state, float delta_time) {
	// Poll input
	if(IsGamepadAvailable(0))
		PollInputGamepad(state);
	else
		PollInputKeyboard(state);

	// TODO: Manage timers
}

void PollInputKeyboard(InputState *state) {
	state->move_x = 0;
	state->move_y = 0;

	if(IsKeyDown(KEY_A)) state->move_x = -1;
	if(IsKeyDown(KEY_D)) state->move_x =  1;

	if(IsKeyDown(KEY_W)) state->move_y = -1;
	if(IsKeyDown(KEY_S)) state->move_y =  1;

	state->jump = IsKeyDown(KEY_SPACE);
}

void PollInputGamepad(InputState *state) {
}
