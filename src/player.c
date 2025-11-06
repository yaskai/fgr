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

float screenshake = 0;

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

	player->angle = -90 * DEG2RAD;
	player->center_offset = (Vector2){sl->spr_pool[0].frame_w * 0.5f, sl->spr_pool[0].frame_h * 0.5f};
	player->radius = player->center_offset.y;

	p->rope = &rope;
	RopeInit(p->rope, player->position);
}

void PlayerSpawn(Entity *player, Vector2 position) {
	
}

void PlayerUpdate(Entity *player, float dt) {
	PlayerData *p = player->data;

	p->cursor_pos = GetScreenToWorld2D(GetMousePosition(), *p->camera);

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

	//if(p->ex_flags & HARPOON_ACTIVE) RopeDraw(p->rope);

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

	if(p->ex_flags & HARPOON_ACTIVE) RopeDraw(p->rope);
}

void PlayerInput(Entity *player, float dt) {
	PlayerData *p = player->data;
	bool run_held = p->input->move_x != 0;

	if(run_held) {
		player->angle += p->input->move_x * dt;
		player->sprite_angle = player->angle * RAD2DEG + 90;
	}

	if(IsKeyDown(KEY_Z)) {
		p->jetpack_timer += dt;
		Vector2 dir = (Vector2){cosf(player->angle), sinf(player->angle)};
		player->velocity = Vector2Add(player->velocity, Vector2Scale(dir, p->jetpack_timer)); 
	} else {
		p->jetpack_timer -= dt;
		if(p->jetpack_timer < 0) p->jetpack_timer = 0;
	}
	
	p->jetpack_timer = Clamp(p->jetpack_timer, 0, 0.1f);

	/*
	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		Vector2 cursor_pos = GetScreenToWorld2D(GetMousePosition(), *p->camera);
		Vector2 dir = Vector2Normalize(Vector2Subtract(cursor_pos, EntCenter(player)));

		HarpoonShoot(player, dir);
	}
	*/

	// Aim harpoon
	/*
	if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		p->harpoon_state = HARPOON_AIM;
	}
	*/

	if(p->harpoon_state == HARPOON_AIM) {
		HarpoonAim(player, dt);
	} else if(p->harpoon_state == HARPOON_EXTEND) {
		p->camera->zoom = Lerp(p->camera->zoom, 0.9f, dt * 5);

		if(IsKeyPressed(KEY_R))
			p->harpoon_state = HARPOON_RETRACT;

	} else if(p->harpoon_state == HARPOON_RETRACT) {
		p->camera->zoom = Lerp(p->camera->zoom, 1.0f, dt);
	} else if(p->harpoon_state == 0) {
		p->camera->zoom = Lerp(p->camera->zoom, 1.0f, dt * 5);

		if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
			p->harpoon_state = HARPOON_AIM;
	}
}

void PlayerPhysicsFreeFloat(Entity *player, float dt) {
	PlayerData *p = player->data;

	Vector2 prev_pos = EntCenter(player);	
	Vector2 next_pos = Vector2Add(prev_pos, player->velocity);

	Vector2 dir = Vector2Normalize(Vector2Subtract(next_pos, prev_pos));

	Vector2 ray_start = Vector2Add(prev_pos, Vector2Scale(dir, player->radius));
	Vector2 ray_dest = Vector2Add(prev_pos, Vector2Scale(dir, 999));

	dir_ray = dir;

	for(uint16_t i = 0; i < ent_handler->count; i++) {
		Entity *ent = &ent_handler->ents[i];

		if(ent->type != ENT_ASTEROID)
			continue;

		if(!(CheckCollisionCircleLine(EntCenter(ent), ent->radius * 1.5f, ray_start, ray_dest)))
			continue;

		if(CheckCollisionCircles(EntCenter(ent), ent->radius, prev_pos, player->radius)) {
			/*
			if(Vector2Length(player->velocity) > 10) 
				screenshake = 0.5f;
			else 
				screenshake = 0.1f;
			*/
			screenshake = Vector2Length(player->velocity) * 0.05f;

			player->velocity = Vector2Scale(player->velocity, -0.75f);
		} 
	}

	player->position = Vector2Add(player->position, player->velocity);

	PlayerCameraControls(player, dt);
}

void PlayerCameraControls(Entity *player, float dt) {
	PlayerData *p = player->data;
	Camera2D *cam = p->camera;

	Vector2 player_center = EntCenter(player);
	cam->target = Vector2Lerp(cam->target, player_center, 5 * dt);

	float rot_target = -player->angle * RAD2DEG - 90;
	cam->rotation = Lerp(cam->rotation, rot_target, 5 * dt);

	if(screenshake > 0) {
		Vector2 offset = (Vector2) { GetRandomValue(-100, 100) * screenshake, GetRandomValue(-100, 100) * screenshake };
		cam->target = Vector2Add(cam->target, offset);
		
		screenshake -= dt;
	} 
}

void HarpoonUpdate(Entity *player, float dt) {
	PlayerData *p = player->data;

	if(!(p->ex_flags & HARPOON_ACTIVE)) return;

	Vector2 to_player = Vector2Normalize(Vector2Subtract(p->harpoon_pos, EntCenter(player)));
	Vector2 player_forward = (Vector2){cosf(player->angle), sinf(player->angle)};

	/*
	if(Vector2DotProduct(to_player, player_forward) < 0)
		p->harpoon_vel = Vector2Subtract(p->harpoon_vel, Vector2Scale(to_player, Vector2Distance(EntCenter(player), p->harpoon_pos) * dt));
	*/

	if(p->harpoon_state == HARPOON_RETRACT) {
		HarpoonRetract(player, dt);
		return;
	} else if(p->harpoon_state == HARPOON_EXTEND) {
		p->rope->segment_dist = Lerp(p->rope->segment_dist, 4.0f, dt * 20);
	}

	p->harpoon_pos = Vector2Add(p->harpoon_pos, Vector2Scale(p->harpoon_vel, dt));

	RopeNodeSetPos(&p->rope->nodes[0], EntCenter(player));
	RopeNodeSetPos(&p->rope->nodes[ROPE_TAIL], p->harpoon_pos);

	p->rope->gravity = Vector2Scale((Vector2){cosf(player->angle), sinf(player->angle)}, -p->grav_force * 0.001f);

	RopeUpdate(p->rope, dt);
}

void HarpoonCollision(Entity *player, float dt) {
	PlayerData *p = player->data;

	for(uint16_t i = 0; i < ent_handler->count; i++) {
		Entity *ent = &ent_handler->ents[i];

		if(ent->type == ENT_PLAYER) continue;	
	}
}

void HarpoonAim(Entity *player, float dt) {
	PlayerData *p = player->data;

	*p->time_mod = Lerp(*p->time_mod, 0.25f, dt * 5);
	p->camera->zoom = Lerp(p->camera->zoom, 1.15f, GetFrameTime() * 5);

	player->velocity = Vector2Lerp(player->velocity, Vector2Scale(player->velocity, 0.85f), GetFrameTime() * 2.5f);

	if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		Vector2 dir = Vector2Normalize(Vector2Subtract(p->cursor_pos, EntCenter(player)));
		HarpoonShoot(player, dir);
	} 
}

void HarpoonShoot(Entity *player, Vector2 dir) {
	PlayerData *p = player->data;

	if(p->ex_flags & HARPOON_ACTIVE) return;
	p->ex_flags |= HARPOON_ACTIVE;

	for(uint8_t i = 0; i < ROPE_TAIL; i++) {
		Vector2 tangent = (Vector2){-dir.y, dir.x};
		Vector2 tan_offset = (Vector2Scale)(tangent, GetRandomValue(-100, 100));
		Vector2 offset = Vector2Add(tan_offset, Vector2Scale(dir, GetRandomValue(-100, 0)));
		RopeNodeSetPos(&p->rope->nodes[i], Vector2Add(EntCenter(player), offset));
	}

	//p->rope->segment_dist = 3.25f;
	p->rope->segment_dist = 0.0f;

	p->rope->nodes[ROPE_TAIL].flags |= NODE_PINNED;

	p->harpoon_pos = EntCenter(player);
	p->harpoon_vel = Vector2Scale(dir, 999);

	p->harpoon_state = HARPOON_EXTEND;

	*p->time_mod = 1.0f;

	player->velocity = Vector2Add(player->velocity, Vector2Scale(dir, -2.5f));
}

void HarpoonRetract(Entity *player, float dt) {
	PlayerData *p = player->data;

	p->rope->nodes[ROPE_TAIL].flags &= ~NODE_PINNED;

	p->harpoon_vel = Vector2Zero();
	RopeNodeSetPos(&p->rope->nodes[0], EntCenter(player));

	p->rope->segment_dist = Lerp(p->rope->segment_dist, p->rope->segment_dist * 0.25f, dt * 20);

	for(uint8_t i = 0; i < ROPE_LENGTH; i++) {
		RopeNode *node = &p->rope->nodes[i];
		
		Vector2 to_player = Vector2Normalize(Vector2Subtract(node->pos_curr, EntCenter(player))); 
		node->pos_prev = Vector2Add(node->pos_prev, Vector2Scale(to_player, dt));
	}

	RopeUpdate(p->rope, dt);

	if(Vector2Distance(p->rope->nodes[ROPE_TAIL].pos_curr, EntCenter(player)) <= player->radius * 2) {
		p->ex_flags &= ~HARPOON_ACTIVE;
		p->harpoon_state = 0;
	}
}

