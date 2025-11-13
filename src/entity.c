#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"
#include "kmath.h"

void EntInit(Entity *ent, uint8_t type) {
	
}

// Move entity by it's velocity scaled by delta time
void EntUpdatePosition(Entity *ent, float dt) {
	ent->position = Vector2Add(ent->position, Vector2Scale(ent->velocity, dt));	
}

Vector2 EntCenter(Entity *ent) {
	return Vector2Add(ent->position, ent->center_offset);
}
