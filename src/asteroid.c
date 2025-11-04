#include "raylib.h"
#include "entity.h"
#include "sprites.h"

void AsteroidUpdate(Entity *asteroid, float dt) 
{

}

void AsteroidDraw(Entity *asteroid, SpriteLoader *sl) 
{
	DrawSpritePro(&sl->spr_pool[1], 0, asteroid->position, asteroid->sprite_angle, 0);

	/*
	Vector2 center = (Vector2){asteroid->position.x + asteroid->radius, asteroid->position.y + asteroid->radius};
	DrawCircleV(center, 10, RED);
	DrawCircleLinesV(center, asteroid->radius, RAYWHITE);
	DrawCircleLinesV(center, asteroid->radius * 3, RAYWHITE);
	*/	
}
