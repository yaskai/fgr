#include <stdio.h>
#include "raylib.h"
#include "entity.h"

void FishUpdate(Entity *fish, float dt) {
	FishData *f = fish->data;	

	EntUpdatePosition(fish, dt);
}

void FishDraw(Entity *fish, SpriteLoader *sl) {
	FishData *f = fish->data;	
	DrawSpritePro(&sl->spr_pool[2], 0, fish->position, fish->sprite_angle, fish->scale, 0);
}

