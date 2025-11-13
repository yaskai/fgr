#include "entity.h"
#include <raylib.h>

void FishUpdate(Entity *fish, float dt) {
	FishData *f = fish->data;	

	EntUpdatePosition(fish, dt);
}

void FishDraw(Entity *fish, SpriteLoader *sl) {
	FishData *f = fish->data;	

	DrawCircleV(EntCenter(fish), fish->radius, GOLD);
}

