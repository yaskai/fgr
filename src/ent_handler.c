#include <math.h>
#include <stdint.h>
#include <stdio.h>
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
	MAX_ASTEROIDS
};

Vector2 ray_start;
Vector2 ray_end;
Vector2 ray_coll_point;

// Entity reservation function prototype and array 
typedef void(*ReserveDataFunc)(EntHandler *handler, Entity *ent);
ReserveDataFunc data_reserve_funcs[] = { &ReserveDataPlayer, &ReserveDataFish, &ReserveDataNpc, &ReserveDataAsteroid };

// Entity update function prototype and array 
typedef void(*EntUpdateFunc)(Entity *ent, float dt);
EntUpdateFunc ent_update_funcs[] = { &PlayerUpdate, &AsteroidUpdate, NULL, NULL };

// Entity draw function prototype and array 
typedef void(*EntDrawFunc)(Entity *ent, SpriteLoader *sl);
EntDrawFunc ent_draw_funcs[] = { &PlayerDraw, &AsteroidDraw, NULL, NULL };

// Initialize entity handler 
void EntHandlerInit(EntHandler *handler, SpriteLoader *sprite_loader, Camera2D *camera) {
	handler->sprite_loader = sprite_loader;
	handler->camera = camera;
}

// Update all entities
void EntHandlerUpdate(EntHandler *handler, float dt) {
	Entity *player_ent = &handler->ents[0];
	PlayerData *p = player_ent->data;

	if(p->anchor_id > -1)	
		EntOrbitUpdate(player_ent, &handler->ents[p->anchor_id], dt);

	for(uint16_t i = 0; i < handler->count; i++) {
		// Get entity pointer
		Entity *ent = &handler->ents[i];

		// Skip update if not active
		if(!(ent->flags & ENT_ACTIVE)) continue;

		// Call entity's update function
		if(ent->update) ent->update(ent, dt);
	}
	
	FindPlayerOrbit(handler, dt);
}

// Draw all entities
void EntHandlerDraw(EntHandler *handler, uint8_t flags) {
	//if(flags & SHOW_DEBUG)
		//DrawLine(ray_start.x, ray_start.y, ray_end.x, ray_end.y, WHITE);

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
	if(p->raycast_id > -1) {
		Entity *cast_hit_body = &handler->ents[p->raycast_id];
		//DrawCircleLinesV(EntCenter(cast_hit_body), cast_hit_body->radius * 3, BLUE);
		
		//DrawText(TextFormat("%d", p->raycast_id), player_ent->position.x, player_ent->position.y, 16, BLUE);	
	}
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
}

// Spawn an asteroid entity at provided position
void AsteroidSpawn(EntHandler *handler, Vector2 position) {
	int16_t id = EntMake(handler, ENT_ASTEROID);
	if(id == -1) return;

	Entity *ast = &handler->ents[id];
	ast->position = position;
	ast->radius = handler->sprite_loader->spr_pool[1].frame_w * 0.5f;
	ast->center_offset = (Vector2){ast->radius, ast->radius};
	ast->flags |= ENT_IS_BODY;
}

void FindPlayerOrbit(EntHandler *handler, float dt) {
	Entity *player_ent = &handler->ents[0];
	PlayerData *p = player_ent->data;

	Entity *orbit_body = NULL;

	int16_t nearest_body_id = -1;
	int16_t raycast_body_id = -1;

	float shortest_dist = FLT_MAX, shortest_cast_dist = FLT_MAX;

	ray_start = EntCenter(player_ent);
	ray_end = Vector2Add(EntCenter(player_ent), Vector2Scale(p->orbit_dir, 2000)); 
 
	for(uint16_t i = 0; i < handler->count; i++) {
		Entity *body = &handler->ents[i];
		if(!(body->flags & ENT_IS_BODY)) continue;
	
		float dist = Vector2Distance(EntCenter(player_ent), EntCenter(body));		
		if(dist < shortest_dist) {
			nearest_body_id = i;	
			shortest_dist = dist;
		}

		if(CheckCollisionCircleLine(EntCenter(body), body->radius * 3, ray_start, ray_end)) {
			if(p->anchor_id != i) {
				raycast_body_id = i;
			}
		}
	}

	if(nearest_body_id > -1) {
		orbit_body = &handler->ents[nearest_body_id];
		
		if(player_ent->flags & ENT_ORBIT)
			p->prev_anchor_id = p->anchor_id;

		if(CheckCollisionCircles(EntCenter(player_ent), player_ent->radius, EntCenter(orbit_body), orbit_body->radius * 2)) {
			if(((player_ent->flags & ENT_ORBIT) == 0) || p->anchor_id != nearest_body_id) {
				EntOrbitStart(player_ent, orbit_body);
				p->anchor_id = nearest_body_id;
			}
		}
	}

	p->raycast_id = raycast_body_id;
}

