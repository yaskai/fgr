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

float rope_t = 0;

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

	if(p->ex_flags & HARPOON_ACTIVE) 
		RopeDraw(p->rope);

	if(p->harpoon_state == HARPOON_AIM)
		DrawCircleLinesV(p->cursor_pos, 15, RAYWHITE);
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

	switch(p->harpoon_state) {
		case HARPOON_NONE:
			p->camera->zoom = 
				Lerp(p->camera->zoom, 1.0f, dt * 5);

			if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
				p->harpoon_state = HARPOON_AIM;

			break; 

		case HARPOON_AIM:
			HarpoonAim(player, dt);
			break;

		case HARPOON_EXTEND:
			p->camera->zoom 
				= Lerp(p->camera->zoom, 0.9f, dt * 5);

			if(IsKeyPressed(KEY_R))
				p->harpoon_state = HARPOON_RETRACT;

			break;

		case HARPOON_RETRACT:
			p->camera->zoom = Lerp(p->camera->zoom, 1.0f, dt);
			break;
			
		case HARPOON_STUCK:
			if(IsKeyPressed(KEY_R))
				p->harpoon_state = HARPOON_PULL;

			//p->camera->zoom = Lerp(p->camera->zoom, 1 - (p->rope->stretch * 0.0001f), dt * 10);
			Vector2 mid = Vector2Subtract(p->camera->target, EntCenter(player));
			p->camera->target = Vector2Lerp(p->camera->target, mid, dt * 3);

			break;

		case HARPOON_PULL:
			HarpoonPull(player, dt);
			break;
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

		/*
		if(!(CheckCollisionCircleLine(EntCenter(ent), ent->radius * 1.5f, ray_start, ray_dest)))
			continue;
		*/

		if(CheckCollisionCircles(EntCenter(ent), ent->radius, EntCenter(player), player->radius)) {
			screenshake = Vector2Length(player->velocity) * 0.05f;

			if(Vector2Length(player->velocity) < 0.1f ) {
				player->velocity = Vector2Scale(dir, -10);
			}

			player->velocity = Vector2Scale(player->velocity, -0.75f);

			if(p->harpoon_state == HARPOON_PULL) {
				p->harpoon_state = HARPOON_NONE;
				p->ex_flags &= ~HARPOON_ACTIVE;

				Vector2 to_ent = Vector2Normalize(Vector2Subtract(EntCenter(ent), EntCenter(player)));
				float dist = Vector2Distance(EntCenter(ent), EntCenter(player));

				player->position = Vector2Subtract(player->position, Vector2Scale(to_ent, (player->radius + ent->radius) * 0.25f));
			}
		} 
	}

	if(p->harpoon_state != HARPOON_PULL)
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

	if(p->harpoon_state == HARPOON_RETRACT) {
		HarpoonRetract(player, dt);
		return;
	} else if(p->harpoon_state == HARPOON_EXTEND) {
		p->rope->segment_dist = Lerp(p->rope->segment_dist, 4.0f, dt * 20);

		rope_t -= dt;
		if(rope_t < 0) {
			rope_t = 0.5f;
		}

		HarpoonCollision(player, dt);
	} else if(p->harpoon_state == HARPOON_PULL) {
		HarpoonPull(player, dt);
 	} else if(p->harpoon_state == HARPOON_STUCK) {
		Vector2 pull_dir = Vector2Normalize(Vector2Subtract(p->rope->nodes[1].pos_curr, EntCenter(player)));
		//Vector2 pull_dir = Vector2Normalize(Vector2Subtract(p->harpoon_pos, EntCenter(player)));
		float pull_amount = fabs(p->rope->stretch - p->rope->max_stretch) * 1.5f;

		float dir_dot = Vector2DotProduct(pull_dir, Vector2Normalize(player->velocity));

		float harpoon_dist = Vector2Distance(EntCenter(player), p->harpoon_pos);

		bool pull = (
			//Vector2DotProduct(pull_dir, Vector2Normalize(player->velocity)) < -0.7f &&
			//dir_dot < -0.6f &&
			dir_dot < -0.7f &&
			p->rope->stretch >= p->rope->max_stretch 
		);

		if(pull) {
			//player->velocity = Vector2Lerp(player->velocity, pull_dir, pull_amount * dt);
			player->velocity = Vector2Add(player->velocity, Vector2Scale(pull_dir, pull_amount * dt));
		}
	}

	p->harpoon_pos = Vector2Add(p->harpoon_pos, Vector2Scale(p->harpoon_vel, dt));

	RopeNodeSetPos(&p->rope->nodes[0], EntCenter(player));
	RopeNodeSetPos(&p->rope->nodes[ROPE_TAIL], p->harpoon_pos);

	p->rope->gravity = Vector2Scale((Vector2){cosf(player->angle), sinf(player->angle)}, -p->grav_force * 0.001f);

	RopeUpdate(p->rope, dt);
}

void HarpoonCollision(Entity *player, float dt) {
	PlayerData *p = player->data;

	if(p->harpoon_state == HARPOON_STUCK) return;

	for(uint16_t i = 0; i < ent_handler->count; i++) {
		Entity *ent = &ent_handler->ents[i];

		if(ent->type == ENT_PLAYER) continue;	

		Vector2 forward = Vector2Normalize(p->harpoon_vel);
		Vector2 ray_start = p->harpoon_pos;
		Vector2 ray_end = Vector2Add(ray_start, Vector2Scale(forward, 999));
		
		if(!(CheckCollisionCircleLine(EntCenter(ent), ent->radius * 1.5f, ray_start, ray_end))) 
			continue;

  		if(CheckCollisionCircles(EntCenter(ent), ent->radius * 1.25f, p->harpoon_pos, 1.0f)) {
			p->harpoon_state = HARPOON_STUCK;
			p->harpoon_vel = Vector2Zero();
		}
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

	p->rope->start_id = 0;

	for(uint8_t i = p->rope->start_id; i < ROPE_TAIL; i++) {
		Vector2 tangent = (Vector2){-dir.y, dir.x};
		Vector2 tan_offset = (Vector2Scale)(tangent, GetRandomValue(-100, 100));
		Vector2 offset = Vector2Add(tan_offset, Vector2Scale(dir, GetRandomValue(-100, 0)));
		RopeNodeSetPos(&p->rope->nodes[i], Vector2Add(EntCenter(player), offset));
	}

	p->rope->segment_dist = 0.0f;

	p->rope->nodes[0].flags |= NODE_PINNED;
	p->rope->nodes[ROPE_TAIL].flags |= NODE_PINNED;

	p->harpoon_pos = EntCenter(player);
	p->harpoon_vel = Vector2Scale(dir, 999);

	p->harpoon_state = HARPOON_EXTEND;

	p->rope->iterations = 16;

	*p->time_mod = 1.0f;

	player->velocity = Vector2Add(player->velocity, Vector2Scale(dir, -2.5f));
}

void HarpoonRetract(Entity *player, float dt) {
	PlayerData *p = player->data;

	p->rope->nodes[ROPE_TAIL].flags &= ~NODE_PINNED;

	p->harpoon_vel = Vector2Zero();
	RopeNodeSetPos(&p->rope->nodes[0], EntCenter(player));

	//p->rope->segment_dist = Lerp(p->rope->segment_dist, p->rope->segment_dist * 0.25f, dt * 20);
	p->rope->segment_dist *= (0.9999f * dt);

	for(uint8_t i = 0; i < ROPE_LENGTH; i++) {
		RopeNode *node = &p->rope->nodes[i];
		
		Vector2 to_player = Vector2Normalize(Vector2Subtract(node->pos_curr, EntCenter(player))); 
		node->pos_prev = Vector2Add(node->pos_prev, Vector2Scale(to_player, dt));
	}

	if(p->rope->segment_dist < 1) 
		p->rope->iterations = 128;

	RopeUpdate(p->rope, dt);

	if(Vector2Distance(p->rope->nodes[ROPE_TAIL].pos_curr, EntCenter(player)) <= player->radius * 2) {
		p->ex_flags &= ~HARPOON_ACTIVE;
		p->harpoon_state = 0;
		p->rope->iterations = 16;
	}
}

void HarpoonPull(Entity *player, float dt) {
	PlayerData *p = player->data;

	//p->rope->segment_dist = Lerp(p->rope->segment_dist, 0, dt * 20);
	//p->rope->segment_dist -= (p->rope->segment_dist * 0.1f) * dt;
	p->rope->segment_dist *= (0.9999f * dt);

	rope_t -= dt;
	if(rope_t < 0) {
		p->rope->start_id++;
		rope_t = 0.5f;
	}

	//player->velocity = (Vector2){0, 0};

	p->rope->nodes[p->rope->start_id].flags &= ~NODE_PINNED;
	//p->rope->nodes[ROPE_TAIL].flags |= NODE_PINNED;
	RopeNodeSetPos(&p->rope->nodes[ROPE_TAIL], p->harpoon_pos);

	Vector2 new_center = p->rope->nodes[p->rope->start_id].pos_curr; 
	player->position = (Vector2) {
		new_center.x - player->center_offset.x,
		new_center.y - player->center_offset.y
	};

	Vector2 to_node = Vector2Normalize(Vector2Subtract(p->rope->nodes[p->rope->start_id].pos_curr, EntCenter(player)));
	//Vector2 to_node = Vector2Normalize(Vector2Subtract(p->rope->nodes[ROPE_TAIL].pos_curr, EntCenter(player)));
	Vector2 wish_vel = Vector2Scale(to_node, 4.0f);
	//player->velocity = Vector2Lerp(player->velocity, wish_vel, dt * 10);
	player->velocity = wish_vel;
	//player->velocity = Vector2Add(player->velocity, wish_vel);

	//player->angle = atan2f(-to_node.y, to_node.x) * DEG2RAD - (0 * DEG2RAD);
	//player->sprite_angle = atan2f(-to_node.y, to_node.x);

	//player->velocity = wish_vel;

	RopeUpdate(p->rope, dt);

	if(Vector2Distance(p->rope->nodes[ROPE_TAIL].pos_curr, EntCenter(player)) <= player->radius * 2) {
		p->ex_flags &= ~HARPOON_ACTIVE;
		p->harpoon_state = 0;
		//player->velocity = Vector2Zero();
		//player->velocity = Vector2Scale(player->velocity, 0.8f);
		to_node = Vector2Normalize(Vector2Subtract(p->harpoon_pos, EntCenter(player)));
		wish_vel = Vector2Scale(to_node, 4.0f);
		player->velocity = wish_vel;
	}
}

/*
void HarpoonPull(Entity *player, float dt) {
	PlayerData *p = player->data;
	Rope *rope = p->rope;

	Vector2 rope_dir = Vector2Normalize(Vector2Subtract(rope->nodes[ROPE_TAIL].pos_curr, EntCenter(player)));
	
	//float f_rope_len = 
}
*/

