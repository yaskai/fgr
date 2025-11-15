#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "raylib.h"
#include "raymath.h"
#include "ent_handler.h"
#include "entity.h"

// Maximum count of entity type array
uint16_t type_max[] = {
	MAX_PLAYERS,
	MAX_FISH,
	MAX_NPCS,
	MAX_ASTEROIDS,
	MAX_ITEMS
};

Vector2 ray_start;
Vector2 ray_end;
Vector2 ray_coll_point;

// Entity reservation function prototype and array 
typedef void(*ReserveDataFunc)(EntHandler *handler, Entity *ent);
ReserveDataFunc data_reserve_funcs[] = {
	&ReserveDataPlayer,
	&ReserveDataFish,
	&ReserveDataNpc,
	&ReserveDataAsteroid,  
	NULL
};

// Entity update function prototype and array 
typedef void(*EntUpdateFunc)(Entity *ent, float dt);
EntUpdateFunc ent_update_funcs[] = { 
	&PlayerUpdate,
	&FishUpdate,
	NULL,
	&AsteroidUpdate,
	NULL
};

// Entity draw function prototype and array 
typedef void(*EntDrawFunc)(Entity *ent, SpriteLoader *sl);
EntDrawFunc ent_draw_funcs[] = { 
	&PlayerDraw,
	&FishDraw,
	NULL,
	&AsteroidDraw,
	NULL
};

// Initialize entity handler 
void EntHandlerInit(EntHandler *handler, SpriteLoader *sprite_loader, Camera2D *camera) {
	handler->sprite_loader = sprite_loader;
	handler->camera = camera;

	handler->time_mod = 1.0f;

	handler->ents = calloc(ENT_ARENA_CAP, sizeof(Entity));

	handler->player_data 	= calloc(MAX_PLAYERS, sizeof(PlayerData));
	handler->fish_data 		= calloc(MAX_FISH, sizeof(FishData));
	handler->npc_data 		= calloc(MAX_NPCS, sizeof(NpcData));
	handler->asteroid_data 	= calloc(MAX_ASTEROIDS, sizeof(AsteroidData));
	handler->item_data 		= calloc(MAX_ITEMS, sizeof(ItemData));
}

// Update all entities
void EntHandlerUpdate(EntHandler *handler, float dt) {
	Entity *player_ent = &handler->ents[0];
	PlayerData *p = player_ent->data;

	for(uint16_t i = 0; i < handler->count; i++) {
		// Get entity pointer
		Entity *ent = &handler->ents[i];

		// Skip update if not active
		if(!(ent->flags & ENT_ACTIVE)) continue;

		// Call entity's update function
		if(ent->update) ent->update(ent, dt * handler->time_mod);
	}
}

// Draw all entities
void EntHandlerDraw(EntHandler *handler, uint8_t flags) {
	for(uint16_t i = 1; i < handler->count; i++) {
		// Get entity pointer
		Entity *ent = &handler->ents[i];
			
		// Skip draw call if entitiy inactive
		if(!(ent->flags & ENT_ACTIVE)) continue;
		
		// Call entity's draw function
		if(ent->draw) ent->draw(ent, handler->sprite_loader);
	}

	Entity *player_ent = &handler->ents[0];	
	PlayerData *p = player_ent->data;
	player_ent->draw(player_ent, handler->sprite_loader);
}

// Create a new entity and add to pool (corresponding to entity type)
int16_t EntMake(EntHandler *handler, uint8_t type) {
	// Get count for entity type
	uint16_t *type_count = &handler->type_counts[type];
	
	// Dont't add entity data if slots are full
	if(*type_count >= type_max[type]) return -1;
	
	// Get entity pointer
	Entity *ent = &handler->ents[handler->count]; 
	ent->type = type;

	// Initialize entity
	*ent = (Entity){0};
	ent->flags |= ENT_ACTIVE;

	// Set function pointers
	ent->update = ent_update_funcs[type];
	ent->draw = ent_draw_funcs[type];
	
	// Reserve data
	data_reserve_funcs[type](handler, &handler->ents[handler->count]);
	
	// Increment count for entity type
	(*type_count)++;

	// Return entity's index 
	return handler->count++;
}

// Reserve data for entity of type "player"
void ReserveDataPlayer(EntHandler *handler, Entity *ent) {
	// Get data index
	uint16_t data_id = handler->type_counts[ENT_PLAYER];

	// Init data
	PlayerData player_data = (PlayerData){0};
	handler->player_data[data_id] = player_data;

	// Set entity data pointer
	ent->data = &handler->player_data[data_id];
	PlayerInit(ent, handler->sprite_loader, handler->camera);

	PlayerData *p = ent->data;
	p->time_mod = &handler->time_mod;

	ent->scale = 1;
}

// Reserve data for entity of type "asteroid"
void ReserveDataAsteroid(EntHandler *handler, Entity *ent) {
	// Get data index
	uint16_t data_id = handler->type_counts[ENT_ASTEROID];

	// Init data
	AsteroidData data = (AsteroidData){0};
	handler->asteroid_data[data_id] = data;

	// Set entity data pointers
	ent->data = &handler->asteroid_data[data_id];

	printf("reserved data for asteroid entity %d\n", data_id);
}

// Reserve data for entity of type "fish"
void ReserveDataFish(EntHandler *handler, Entity *ent) {
	// Get data index
	uint16_t data_id = handler->type_counts[ENT_FISH];

	// Init data
	FishData data = (FishData){0};
	handler->fish_data[data_id] = data;

	// Set entity data pointer
	ent->data = &handler->fish_data[data_id];

	printf("reserved data for fish entity %d\n", data_id);
}

// Reserve data for entity of type "npc"
void ReserveDataNpc(EntHandler *handler, Entity *ent) {
	// Get data index
	uint16_t data_id = handler->type_counts[ENT_NPC];

	// Init data
	NpcData data = (NpcData){0};
	handler->npc_data[data_id] = data;

	// Set entity data pointer
	ent->data = &handler->fish_data[data_id];

	printf("reserved data for npc entity %d\n", data_id);
}

// Spawn an asteroid entity at provided position
void AsteroidSpawn(EntHandler *handler, Vector2 position, float scale, float angle_vel) {
	int16_t id = EntMake(handler, ENT_ASTEROID);
	if(id == -1) return;

	Entity *ast = &handler->ents[id];
	ast->position = position;
	ast->type = ENT_ASTEROID;
	ast->flags |= ENT_IS_BODY;

	AsteroidData *d = ast->data;
	d->angle_vel = angle_vel;

	ast->angle = GetRandomValue(0, 360) * DEG2RAD; 
	ast->scale = scale;

	ast->radius = handler->sprite_loader->spr_pool[1].frame_w * 0.5f * ast->scale;
	ast->center_offset = (Vector2){ast->radius / ast->scale, ast->radius / ast->scale};
}

void FishSpawn(EntHandler *handler, Vector2 position) {
	int16_t id = EntMake(handler, ENT_FISH);
	if(id == -1) return;
	
	Entity *fish = &handler->ents[id];

	fish->position = position;
	fish->scale = 1;

	fish->type = ENT_FISH;

	fish->radius = handler->sprite_loader->spr_pool[2].frame_w * 0.5f * fish->scale;
	fish->center_offset = (Vector2){fish->radius / fish->scale, fish->radius / fish->scale};
}

