#include "raylib.h"
#include "entity.h"
#include "sprites.h"

void AsteroidUpdate(Entity *asteroid, float dt) 
{
	AsteroidData *d = asteroid->data;

	/*
	asteroid->angle += d->angle_vel * dt;
	if(asteroid->angle > 360 * DEG2RAD) asteroid->angle = 0 * DEG2RAD;
	else if(asteroid->angle < 0 * DEG2RAD) asteroid->angle = 360 * DEG2RAD;
	*/

	asteroid->sprite_angle = asteroid->angle * RAD2DEG;
}

void AsteroidDraw(Entity *asteroid, SpriteLoader *sl) 
{
	DrawSpritePro(&sl->spr_pool[1], 0, asteroid->position, asteroid->sprite_angle, asteroid->scale, 0);

	/*
	Vector2 center = (Vector2){asteroid->position.x + asteroid->radius, asteroid->position.y + asteroid->radius};
	DrawCircleV(center, 10, RED);
	DrawCircleLinesV(center, asteroid->radius, RAYWHITE);
	DrawCircleLinesV(center, asteroid->radius * 3, RAYWHITE);
	*/	

	AsteroidData *d = asteroid->data;

	//DrawText(TextFormat("%d", asteroid->type), asteroid->position.x, asteroid->position.y, 30, RAYWHITE);
	//DrawText(TextFormat("%f", d->angle_vel), asteroid->position.x, asteroid->position.y, 30, RAYWHITE);
}
