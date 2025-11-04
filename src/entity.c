#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"
#include "kmath.h"

void EntInit(Entity *ent, uint8_t type) {
	
}

void EntUpdatePosition(Entity *ent, float dt) {
	// Move entity by it's velocity scaled by delta time
	ent->position = Vector2Add(ent->position, Vector2Scale(ent->velocity, dt));	
}

Vector2 EntCenter(Entity *ent) {
	return Vector2Add(ent->position, ent->center_offset);
}
