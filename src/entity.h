#include <stdint.h>
#include "raylib.h"
#include "sprites.h"
#include "input.h"
#include "rope.h"

#ifndef ENTITY_H_
#define ENTITY_H_

#define ENT_ACTIVE			0x01
#define ENT_ORBIT			0x02
#define ENT_IS_BODY			0x04
#define ENT_GROUNDED		0x08
#define ENT_CAST_ORBIT		0x10
#define ENT_SWITCH_ORBIT	0x20

#define ENT_TYPE_COUNT	4

enum ENT_TYPE {
	ENT_PLAYER,		
	ENT_ASTEROID,
	ENT_FISH,
	ENT_NPC
};

// *** BASE ENTITY STRUCT ***	
//
typedef struct Entity {
	uint8_t flags;			// Bit flags: active, anchored, alive, etc...
	uint8_t type;			// Entity type
	uint8_t sprite_id;		// Spritesheet index

	float radius;
	float angle;
	float orbit_height;
	float orbit_angle;		// Angle used for physics calculations in radians
	float sprite_angle;		// Angle used for sprite rotation in degrees

	float orbit_switch_t;

	Vector2 position;
	Vector2 velocity;
	Vector2 center_offset;
	
	Vector2 pull_point_prev;
	Vector2 pull_point_next;
	Vector2 pull_point;
	
	// Primary entity function pointers:
	// functions specified on EntInit() 
	void (*update)(struct Entity *self, float dt);
	void (*draw)(struct Entity *self, SpriteLoader *sl);

	// Pointer to entity type specific data
	void *data;
} Entity;

// *** SHARED ENTITY FUNCTIONS ***
//
// Initialize entity, set function pointers, data, etc.
void EntInit(Entity *ent, uint8_t type);

// Add current velocity of entity to entity's position
void EntUpdatePosition(Entity *ent, float dt);

Vector2 EntCenter(Entity *ent);

// *** PLAYER ***
//
typedef struct {
	uint8_t ex_flags;			// Extra flags
	uint8_t state;				// State used for animations some inputs
	uint8_t harpoon_state;

	short sprite_dir;			// Sprite direction
	short active_anim;		    // Index of current animation, -1 for none
	
	int16_t anchor_id;			// Index of anchored body, -1 for none
	int16_t prev_anchor_id;		// Index of previous anchored body

	int16_t raycast_id;	

	float orbit_height;			// How far away entity should be from orbited body
	float grav_force;

	float jump_timer;
	float jetpack_timer;

	Vector2 orbit_dir;
	Vector2 orbit_vel;			// X for circular movement and Y for height/distance 

	Vector2 harpoon_pos;
	Vector2 harpoon_vel;

	Vector2 cursor_pos;

	float *time_mod;

	Camera2D *camera;			// Pointer to camera instance
	InputState *input;			// Pointer to input state instance
	
	SpriteAnimation *run_anim;

	Rope *rope;
} PlayerData;

enum PLAYER_STATES {
	PLR_IDLE,
	PLR_RUN,
	PLR_JUMP,
	PLR_FALL,
	PLR_CHARGE_SHOT,
	PLR_SHOOT,
	PLR_DEAD
};

#define PLR_MAX_RUN_VEL		100.0f;
#define PLR_MAX_FALL_VEL	200.0f;

#define PLR_JUMP_GRAV	   1000.0f		
#define PLR_FALL_GRAV	    900.0f
#define PLR_CUT_GRAV	   1850.0f

#define HARPOON_ACTIVE		0x01

enum harpoon_state {
	HARPOON_NONE,
	HARPOON_AIM,
	HARPOON_EXTEND,
	HARPOON_RETRACT
};

void PlayerInit(Entity *player, SpriteLoader *sl, Camera2D *camera);
void PlayerSpawn(Entity *player, Vector2 position);
void PlayerUpdate(Entity *player, float dt);
void PlayerDraw(Entity *player, SpriteLoader *sl);
void PlayerInput(Entity *player, float dt);

void PlayerPhysicsFreeFloat(Entity *player, float dt);

void PlayerCameraControls(Entity *player, float dt);

void HarpoonUpdate(Entity *player, float dt);
void HarpoonCollision(Entity *player, float dt);

void HarpoonAim(Entity *player, float dt);
void HarpoonShoot(Entity *player, Vector2 dir);
void HarpoonRetract(Entity *player, float dt);

// *** ASTEROID ***
//
typedef struct {
	uint8_t ex_flags;
	uint8_t state;
} AsteroidData;

void AsteroidUpdate(Entity *asteroid, float dt);
void AsteroidDraw(Entity *asteroid, SpriteLoader *sl);

// *** FISH ***
//
typedef struct {
	uint8_t ex_flags;
	uint8_t state;
} FishData;

void FishUpdate(Entity *fish, float dt);
void FishDraw(Entity *fish, SpriteLoader *sl);

// *** NPC ***
//
typedef struct {
	uint8_t ex_flags;
	uint8_t state;
} NpcData;

void NpcUpdate(Entity *npc, float dt);
void NpcDraw(Entity *npc, SpriteLoader *sl);

#endif // !ENTITY_H_
