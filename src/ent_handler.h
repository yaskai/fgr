#include <stdlib.h>
#include "entity.h"

#ifndef ENT_HANDLER_H
#define ENT_HANDLER_H

#define ENT_ARENA_CAP	1024	
#define ENT_PLAYER_ID	0

#define MAX_PLAYERS 	1
#define MAX_ASTEROIDS 	(ENT_ARENA_CAP/3)
#define MAX_FISH		(ENT_ARENA_CAP/3)
#define MAX_NPCS		0
#define MAX_ITEMS		((ENT_ARENA_CAP/3)-1)

#define SHOW_DEBUG	0x01

typedef struct {
	uint16_t count;
	uint16_t cap;

	float time_mod;

	uint16_t type_counts[ENT_TYPE_COUNT];

	Entity *ents;

	PlayerData *player_data;
	AsteroidData *asteroid_data;
	FishData *fish_data;
	NpcData *npc_data;
	ItemData *item_data;

	SpriteLoader *sprite_loader;
	Camera2D *camera;
} EntHandler;

void EntHandlerInit(EntHandler *handler, SpriteLoader *sl, Camera2D *camera);
void EntHandlerUpdate(EntHandler *handler, float dt);
void EntHandlerDraw(EntHandler *handler, uint8_t flags);

// Create an entity instance, returns entity's index, -1 if instance fails
int16_t EntMake(EntHandler *handler, uint8_t type);

void ReserveDataPlayer(EntHandler *handler, Entity *ent);
void ReserveDataFish(EntHandler *handler, Entity *ent);
void ReserveDataNpc(EntHandler *handler, Entity *ent);
void ReserveDataAsteroid(EntHandler *handler, Entity *ent);
void ReserveDataItem(EntHandler *handler, Entity *ent);

void AsteroidSpawn(EntHandler *handler, Vector2 position, float scale, float angle_vel);
void FishSpawn(EntHandler *handler, Vector2 position);

void PlayerSetHandler(EntHandler *handler);
void RopeSetHandler(EntHandler *handler);

#endif
