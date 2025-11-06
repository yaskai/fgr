#include <math.h>
#include <stdint.h>
#include <float.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"
#include "sprites.h"
#include "ent_handler.h"

Rope rope = (Rope){0};

EntHandler *ent_handler = NULL;
void PlayerSetHandler(EntHandler *handler) { ent_handler = handler; }

Vector2 dir_ray;

// Initialize player, set data, pointers, references, etc.
void PlayerInit(Entity *player, SpriteLoader *sl, Camera2D *camera) {
	PlayerData *p = player->data;
	*p = (PlayerData){0};

	p->anchor_id = -1;
	p->active_anim = 0;
	p->grav_force = PLR_FALL_GRAV;
	p->run_anim = &sl->anims[0];
	p->camera = camera;
	p->harpoon_pos = EntCenter(player);

	player->orbit_angle = -90 * DEG2RAD;
	player->center_offset = (Vector2){sl->spr_pool[0].frame_w * 0.5f, sl->spr_pool[0].frame_h * 0.5f};
	player->radius = player->center_offset.y;

	p->rope = &rope;
	RopeInit(p->rope, player->position);
}

void PlayerSpawn(Entity *player, Vector2 position) {
	
}

void PlayerUpdate(Entity *player, float dt) {
	PlayerData *p = player->data;

	//EntUpdatePosition(player, dt);
	PlayerInput(player, dt);

	switch(p->state) {
		case PLR_IDLE:
			break;
		
		case PLR_RUN:
			AnimPlay(p->run_anim, dt);
			break;

		case PLR_JUMP:
			break;

		case PLR_FALL:
			break;
		
		case PLR_CHARGE_SHOT:
			break;

		case PLR_SHOOT:
			break;

		case PLR_DEAD:
			break;
	}

	PlayerPhysicsFreeFloat(player, dt);
	HarpoonUpdate(player, dt);
}

void PlayerDraw(Entity *player, SpriteLoader *sl) {
	PlayerData *p = player->data;

	Vector2 center = EntCenter(player);
	Vector2 ray_dest = Vector2Add(center, Vector2Scale(dir_ray, 999));
	DrawLine(center.x, center.y, ray_dest.x, ray_dest.y, RAYWHITE);

	RopeDraw(p->rope);
	//if(player->flags & ENT_ORBIT) OrbitDataDrawDebug(&player->orbit_data);

	uint8_t draw_flags = 0;
	if(p->sprite_dir == -1) draw_flags |= SPR_FLIP_X;

	switch(p->state) {
		case PLR_IDLE:
			DrawSpritePro(&sl->spr_pool[player->sprite_id], 0, player->position, player->sprite_angle, draw_flags);
			break;
		
		case PLR_RUN:
			AnimDrawPro(p->run_anim, player->position, player->sprite_angle, draw_flags);
			break;

		case PLR_JUMP:
			DrawSpritePro(&sl->spr_pool[player->sprite_id], 2, player->position, player->sprite_angle, draw_flags);
			break;

		case PLR_FALL:
			DrawSpritePro(&sl->spr_pool[player->sprite_id], 3, player->position, player->sprite_angle, draw_flags);
			break;
		
		case PLR_CHARGE_SHOT:
			break;

		case PLR_SHOOT:
			break;

		case PLR_DEAD:
			break;
	}

	//DrawText(TextFormat("%d", p->anchor_id), 0, 0, 16, GREEN);
	//DrawText(TextFormat("%f", player->orbit_height), 0, 16, 16, GREEN);

	//DrawCircleLinesV(EntCenter(player), player->radius, RAYWHITE);

	//bool grounded = (player->flags & ENT_GROUNDED);
	//DrawText(TextFormat("grounded: %d", grounded), player->position.x, player->position.y, 16, RAYWHITE);
}

void PlayerInput(Entity *player, float dt) {
	PlayerData *p = player->data;
	bool run_held = p->input->move_x != 0;

	if(run_held) {
		player->orbit_angle += p->input->move_x * dt;
		player->sprite_angle = player->orbit_angle * RAD2DEG + 90;
	}

	if(IsKeyDown(KEY_Z)) {
		p->jetpack_timer += dt;
		Vector2 dir = (Vector2){cosf(player->orbit_angle), sinf(player->orbit_angle)};
		player->velocity = Vector2Add(player->velocity, Vector2Scale(dir, p->jetpack_timer)); 
	} else {
		p->jetpack_timer -= dt;
		if(p->jetpack_timer < 0) p->jetpack_timer = 0;
	}
	
	p->jetpack_timer = Clamp(p->jetpack_timer, 0, 0.1f);
}

void PlayerPhysicsFreeFloat(Entity *player, float dt) {
	PlayerData *p = player->data;

	Vector2 prev_pos = EntCenter(player);	
	Vector2 next_pos = Vector2Add(prev_pos, player->velocity);

	Vector2 dir = Vector2Normalize(Vector2Subtract(next_pos, prev_pos));
	Vector2 ray_dest = Vector2Add(prev_pos, Vector2Scale(dir, 999));

	dir_ray = dir;

	for(uint16_t i = 0; i < ent_handler->count; i++) {
		Entity *ent = &ent_handler->ents[i];

		if(ent->type != ENT_ASTEROID)
			continue;

		if(!(CheckCollisionCircleLine(EntCenter(ent), ent->radius * 1.25f, prev_pos, ray_dest)))
			continue;

		if(CheckCollisionCircles(EntCenter(ent), ent->radius, prev_pos, player->radius)) 
			player->velocity = Vector2Scale(player->velocity, -0.75f);
	}

	player->position = Vector2Add(player->position, player->velocity);

	PlayerCameraControls(player, dt);
}

void PlayerCameraControls(Entity *player, float dt) {
	PlayerData *p = player->data;
	Camera2D *cam = p->camera;

	Vector2 player_center = EntCenter(player);
	cam->target = Vector2Lerp(cam->target, player_center, 5 * dt);

	float rot_target = -player->orbit_angle * RAD2DEG - 90;
	cam->rotation = Lerp(cam->rotation, rot_target, 5 * dt);
}

void HarpoonUpdate(Entity *player, float dt) {
	PlayerData *p = player->data;

	p->harpoon_pos = Vector2Add(p->harpoon_pos, Vector2Scale(p->harpoon_vel, dt));

	RopeNodeSetPos(&p->rope->nodes[0], EntCenter(player));
	RopeNodeSetPos(&p->rope->nodes[ROPE_TAIL], p->harpoon_pos);

	p->rope->gravity = Vector2Scale(p->orbit_dir, -p->grav_force * 0.001f);

	RopeUpdate(p->rope, dt);
}

