#include <math.h>
#include <stdint.h>
#include <float.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"
#include "sprites.h"
#include "ent_handler.h"
#include "kmath.h"

#define PLAYER_MAX_VEL 		50.0f
#define CAM_ZOOM_DEFAULT 	0.9f
#define CAM_ZOOM_FOCUSED	1.2f
#define SCREENSHAKE_MAX		100

Rope rope = (Rope){0};

EntHandler *ent_handler = NULL;
void PlayerSetHandler(EntHandler *handler) { ent_handler = handler; }

Vector2 dir_ray;

float screenshake = 0;

float rope_t = 0;

Vector2 debug_ray_start, debug_ray_end;
Vector2 debug_ray_start1, debug_ray_end1;

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

	player->scale = 1;
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
	//DrawLine(center.x, center.y, ray_dest.x, ray_dest.y, RAYWHITE);
	//DrawLine(center.x, center.y, debug_ray_end.x, debug_ray_end.y, GREEN);
	DrawLine(center.x, center.y, debug_ray_end1.x, debug_ray_end1.y, RED);

	//if(p->ex_flags & HARPOON_ACTIVE) RopeDraw(p->rope);

	uint8_t draw_flags = 0;
	if(p->sprite_dir == -1) draw_flags |= SPR_FLIP_X;

	switch(p->state) {
		case PLR_IDLE:
			DrawSpritePro(&sl->spr_pool[player->sprite_id], 0, player->position, player->sprite_angle, player->scale, draw_flags);
			break;
		
		case PLR_RUN:
			AnimDrawPro(p->run_anim, player->position, player->sprite_angle, player->scale, draw_flags);
			break;

		case PLR_JUMP:
			DrawSpritePro(&sl->spr_pool[player->sprite_id], 2, player->position, player->sprite_angle, player->scale, draw_flags);
			break;

		case PLR_FALL:
			DrawSpritePro(&sl->spr_pool[player->sprite_id], 3, player->position, player->sprite_angle, player->scale, draw_flags);
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

	Vector2 mid = EntCenter(player);

	switch(p->harpoon_state) {
		case HARPOON_NONE:
			if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
				p->harpoon_state = HARPOON_AIM;

			break; 

		case HARPOON_AIM:
			HarpoonAim(player, dt);
			break;

		case HARPOON_EXTEND:
			if(IsKeyPressed(KEY_R))
				p->harpoon_state = HARPOON_RETRACT;

			break;

		case HARPOON_RETRACT:
			break;
			
		case HARPOON_STUCK:
			if(IsKeyPressed(KEY_R)) {
				switch(p->harpoon_hit_ent->type) {
					case ENT_ASTEROID: 
						p->harpoon_state = HARPOON_PULL;
						break;

					case ENT_FISH:
						p->harpoon_state = HARPOON_REEL;
						break;
				}
			}

			if(IsKeyPressed(KEY_X) && p->harpoon_hit_ent->type == ENT_ASTEROID) {
				p->harpoon_state = HARPOON_NONE;

				p->ex_flags &= ~HARPOON_ACTIVE;

				AsteroidData *a = p->harpoon_hit_ent->data;

				if(a->angle_vel != 0)
					p->ex_flags |= PLR_FLING;
			}

			break;

		case HARPOON_PULL:
			HarpoonPull(player, dt);
			break;

		case HARPOON_REEL:
			HarpoonReel(player, dt);
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

		if(CheckCollisionCircles(EntCenter(ent), ent->radius, EntCenter(player), player->radius)) {
			screenshake = Vector2Length(player->velocity) * 0.05f;

			player->velocity = Vector2Scale(player->velocity, -0.75f);
			player->position = Vector2Add(player->position, player->velocity);
		} 
	}

	if(p->harpoon_state != HARPOON_PULL)
		player->position = Vector2Add(player->position, player->velocity);

	Vector2 player_forward = Vector2Normalize((Vector2){cosf(player->angle), sinf(player->angle)});
	float dir_dot = Vector2DotProduct(Vector2Normalize(player->velocity), player_forward);

	if(Vector2Length(player->velocity) < 2)
		player->velocity = Vector2Subtract(player->velocity, Vector2Scale(player->velocity, 0.1f * dt));

	PlayerApplyFling(player, p->harpoon_hit_ent, dt);

	PlayerCameraControls(player, dt);

	player->velocity = Vector2ClampValue(player->velocity, -PLAYER_MAX_VEL, PLAYER_MAX_VEL);
}

void PlayerCameraControls(Entity *player, float dt) {
	PlayerData *p = player->data;
	Camera2D *cam = p->camera;

	Vector2 player_center = EntCenter(player);
	cam->target = Vector2Lerp(cam->target, player_center, 5 * dt);

	Vector2 mid = player_center;

	float rot_target = -player->angle * RAD2DEG - 90;
	cam->rotation = Lerp(cam->rotation, rot_target, 5 * dt);

	if(screenshake > 0) {
		short shake_x = GetRandomValue(-SCREENSHAKE_MAX, SCREENSHAKE_MAX) * screenshake;
		short shake_y = GetRandomValue(-SCREENSHAKE_MAX, SCREENSHAKE_MAX) * screenshake;

		Vector2 offset = (Vector2) { shake_x, shake_y };
		cam->target = Vector2Add(cam->target, offset);
		
		screenshake -= dt;
	} 

	switch(p->harpoon_state) {
		case HARPOON_NONE:
			p->camera->zoom = 
				Lerp(p->camera->zoom, CAM_ZOOM_DEFAULT - (Vector2Length(Vector2Normalize(player->velocity)) * 0.01f), dt * 10);

			break; 

		case HARPOON_AIM:
			p->camera->zoom = 
				Lerp(p->camera->zoom, CAM_ZOOM_FOCUSED, dt * 5);

			break;

		case HARPOON_EXTEND:
			p->camera->zoom 
				= Lerp(p->camera->zoom, 0.9f, dt * 5);

			break;

		case HARPOON_RETRACT:
			p->camera->zoom = Lerp(p->camera->zoom, CAM_ZOOM_DEFAULT, dt);
			p->camera->target = Vector2Lerp(p->camera->target, EntCenter(player), dt);
			break;
			
		case HARPOON_STUCK:
			//p->camera->zoom = Lerp(p->camera->zoom, 1 - (p->rope->stretch * 0.0001f), dt * 10);
			mid = Vector2Subtract(p->camera->target, EntCenter(player));
			p->camera->target = Vector2Lerp(p->camera->target, mid, dt);
			break;

		case HARPOON_PULL:
			break;

		case HARPOON_REEL:
			mid = Vector2Subtract(p->camera->target, EntCenter(p->harpoon_hit_ent));
			p->camera->target = Vector2Lerp(p->camera->target, mid, dt);
			break;
	}
}

void HarpoonUpdate(Entity *player, float dt) {
	PlayerData *p = player->data;

	Vector2 harpoon_pos_prev = p->harpoon_pos;
	Vector2 grav_targ = Vector2Zero();

	//Vector2 fling_dir = Vector2Zero();
	//float fling_force = 0;

	if(!(p->ex_flags & HARPOON_ACTIVE)) return;

	Vector2 to_player = Vector2Normalize(Vector2Subtract(p->harpoon_pos, EntCenter(player)));
	Vector2 player_forward = (Vector2){cosf(player->angle), sinf(player->angle)};

	//Vector2 pull_dir = Vector2Normalize(Vector2Subtract(harpoon_pos_prev, EntCenter(player)));
	Vector2 pull_dir = Vector2Normalize(Vector2Subtract(p->rope->nodes[1].pos_prev, EntCenter(player)));

	p->rope->dampening = 0.825f;
	
	if(p->harpoon_state == HARPOON_RETRACT) {
		HarpoonRetract(player, dt);
		return;
	} else if(p->harpoon_state == HARPOON_EXTEND) {
		p->rope->segment_dist = Lerp(p->rope->segment_dist, 4.0f, dt * 20);

		if(p->rope->stretch >= p->rope->max_stretch && p->rope->segment_dist && rope.segment_dist >= 3.9f) {
			rope_t -= dt;
			if(rope_t < -2) {
				p->harpoon_state = HARPOON_RETRACT;
			}
		}

		HarpoonCollision(player, dt);
	} else if(p->harpoon_state == HARPOON_PULL) {
		HarpoonPull(player, dt);
 	} else if(p->harpoon_state == HARPOON_STUCK || p->harpoon_state) {
		AsteroidData *a = p->harpoon_hit_ent->data;

		grav_targ = Vector2Normalize(
			Vector2Subtract(EntCenter(player), EntCenter(p->harpoon_hit_ent)));

		if(p->harpoon_hit_angle != p->harpoon_hit_ent->angle) {
			Vector2 offset = Vector2Subtract(p->harpoon_hit_pos, EntCenter(p->harpoon_hit_ent));
			float angle_diff = p->harpoon_hit_ent->angle - p->harpoon_hit_angle;

			Vector2 rot_offset = (Vector2) {
				offset.x * cosf(angle_diff) - offset.y * sinf(angle_diff),
				offset.x * sinf(angle_diff) + offset.y * cosf(angle_diff)
			};

			Vector2 new_harpoon_pos =
				Vector2Add(EntCenter(p->harpoon_hit_ent), rot_offset);

			p->harpoon_pos = 
				Vector2Add(EntCenter(p->harpoon_hit_ent), rot_offset);

			Vector2 harpoon_dir = Vector2Subtract(p->harpoon_pos, harpoon_pos_prev);

			Vector2 mid = Vector2Normalize(Vector2Subtract(harpoon_dir, (Vector2){-harpoon_dir.y, harpoon_dir.x}));

			//pull_dir = Vector2Add(pull_dir, Vector2Scale(mid, dt));
			//pull_dir = Vector2Lerp(pull_dir, mid, dt);
			///pull_dir = Vector2Normalize(pull_dir);
			//p->rope->gravity = Vector2Negate(Vector2Scale(mid, 10));

			//debug_ray_start = EntCenter(player);
			//debug_ray_end = Vector2Add(debug_ray_start, Vector2Scale((Vector2){-harpoon_dir.y, harpoon_dir.x}, 1000));
			//debug_ray_end = Vector2Add(debug_ray_start, Vector2Scale(mid, 1000));

			Vector2 radial = Vector2Subtract(EntCenter(p->harpoon_hit_ent), EntCenter(player));

			Vector2 tangent = (Vector2){-radial.y, radial.x};

			float angle_speed = a->angle_vel * Vector2Length(radial);
			Vector2 tan_vel = Vector2Scale(Vector2Normalize(tangent), angle_speed * 0.1f * dt);

			//pull_dir = Vector2Normalize(Vector2Add(radial, Vector2Scale(tangent, 0.5f)));

			float strength = 0.f;

			//p->rope->gravity = Vector2Scale(radial, -1.5f);
			grav_targ = Vector2Scale(radial, fabsf(a->angle_vel));
			grav_targ = Vector2Lerp(grav_targ, Vector2Add(grav_targ, tan_vel), dt);

			for(uint8_t i = 0; i < ROPE_TAIL; i++) {
				RopeNode *node = &p->rope->nodes[i];

				Vector2 r  = Vector2Subtract(node->pos_curr, EntCenter(p->harpoon_hit_ent));
				Vector2 rn = Vector2Normalize(r); 
				Vector2 vp  = (Vector2){-rn.y, rn.x};

				Vector2 node_vel = Vector2Scale(vp, (a->angle_vel * Vector2Length(r)) * dt);
				node_vel = Vector2Add(node_vel, player->velocity);
				node_vel = Vector2Scale(node_vel, 10 / Vector2Distance(EntCenter(player), node->pos_curr));
					
				node->pos_curr = Vector2Add(node->pos_curr, node_vel);
				//node->pos_curr = Vector2Lerp(node->pos_curr, Vector2Add(node->pos_curr, node_vel), dt * 10);
			}

			RopeNode *anchor_node = &p->rope->nodes[1];
			anchor_node->pos_curr = Vector2Add(
				anchor_node->pos_curr,
				Vector2Scale(tan_vel, -10)
			);

			debug_ray_start1 = EntCenter(player);
			debug_ray_end1 = Vector2Add(debug_ray_start1, Vector2Scale(pull_dir, 1000));

			player->velocity = Vector2Lerp(player->velocity, Vector2Zero(), dt * 0.01f);
			player->velocity = Vector2Subtract(player->velocity, Vector2Scale(tan_vel, dt * 1.0f));

			p->fling_vel = Vector2Scale(player->velocity, -1.0f);

			if(Vector2Length(player->velocity) > 5)
				p->fling_timer += (Vector2Length(tan_vel) * 0.1f) * dt;

			p->fling_timer = Clamp(p->fling_timer, 0, 2);

			debug_ray_start1 = EntCenter(player);
			debug_ray_end1 = Vector2Add(debug_ray_start1, Vector2Scale(Vector2Normalize(p->fling_vel), 999));
		}

		float pull_amount = fabs(p->rope->stretch - p->rope->max_stretch) * 1.0f;

		float dir_dot = Vector2DotProduct(pull_dir, Vector2Normalize(player->velocity));

			float harpoon_dist = Vector2Distance(EntCenter(player), p->harpoon_pos);

		bool pull = (
			dir_dot < 0.0f &&
			p->rope->stretch >= p->rope->max_stretch &&
			p->harpoon_state != HARPOON_REEL  
			//p->harpoon_hit_ent->type != ENT_FISH
		);

		if(pull) {
			if(p->harpoon_hit_ent->type == ENT_FISH) {
				pull_dir = Vector2Normalize(Vector2Subtract(EntCenter(p->harpoon_hit_ent), EntCenter(player)));
				grav_targ = pull_dir;

				p->harpoon_hit_ent->velocity = Vector2Subtract(
					p->harpoon_hit_ent->velocity,
					Vector2Scale(pull_dir, pull_amount * dt)
					);
			} else
				player->velocity = Vector2Add(player->velocity, Vector2Scale(pull_dir, pull_amount * dt));
		}

		Vector2 dir = Vector2Normalize(Vector2Subtract(EntCenter(p->harpoon_hit_ent), EntCenter(player)));
		//p->rope->gravity = Vector2Scale(dir, -p->grav_force * 0.1f);
	} 

	p->harpoon_pos = Vector2Add(p->harpoon_pos, Vector2Scale(p->harpoon_vel, dt));

	RopeNodeSetPos(&p->rope->nodes[0], EntCenter(player));
	RopeNodeSetPos(&p->rope->nodes[ROPE_TAIL], p->harpoon_pos);

	if(p->harpoon_state == HARPOON_STUCK) {
		if(p->harpoon_hit_ent->type == ENT_FISH)
			RopeNodeSetPos(&p->rope->nodes[ROPE_TAIL], Vector2Add(EntCenter(p->harpoon_hit_ent), p->harpoon_offset));
	}

	p->rope->gravity = Vector2Lerp(p->rope->gravity, grav_targ, dt * 5);

	RopeUpdate(p->rope, dt);
}

void HarpoonCollision(Entity *player, float dt) {
	PlayerData *p = player->data;

	if(p->harpoon_state == HARPOON_STUCK) return;

	for(uint16_t i = 0; i < ent_handler->count; i++) {
		Entity *ent = &ent_handler->ents[i];

		if(!(ent->flags & ENT_ACTIVE)) 	continue;
		if(ent->type == ENT_PLAYER) 	continue;	

		Vector2 forward = Vector2Normalize(p->harpoon_vel);
		Vector2 ray_start = p->harpoon_pos;
		Vector2 ray_end = Vector2Add(ray_start, Vector2Scale(forward, 999));
		
		if(!(CheckCollisionCircleLine(EntCenter(ent), ent->radius * 1.5f, ray_start, ray_end))) 
			continue;

  		if(CheckCollisionCircles(EntCenter(ent), ent->radius * 1.25f, p->harpoon_pos, 1.0f)) {
			p->harpoon_state = HARPOON_STUCK;
			p->harpoon_vel = Vector2Zero();

			p->harpoon_hit_ent = ent;
			p->harpoon_hit_angle = ent->angle;
			p->harpoon_hit_pos = p->harpoon_pos;

			p->harpoon_offset = Vector2Subtract(p->harpoon_pos, p->harpoon_hit_ent->position);
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
	p->rope->stretch = 0;

	rope_t = 0;

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
	Rope *rope = p->rope;

	rope->nodes[ROPE_TAIL].flags &= ~NODE_PINNED;

	//p->harpoon_vel = Vector2Zero();
	RopeNodeSetPos(&rope->nodes[0], EntCenter(player));

	//p->rope->segment_dist = Lerp(p->rope->segment_dist, p->rope->segment_dist * 0.25f, dt * 20);
	p->rope->segment_dist *= (0.9999f * dt);

	for(uint8_t i = 1; i < ROPE_LENGTH; i++) {
		RopeNode *node = &rope->nodes[i];
		
		Vector2 to_player = Vector2Normalize(Vector2Subtract(node->pos_curr, EntCenter(player))); 
		node->pos_prev = Vector2Add(node->pos_prev, Vector2Scale(to_player, dt));
	}

	if(rope->segment_dist < 1) 
		rope->iterations = 128;

	if(Vector2Distance(rope->nodes[ROPE_TAIL].pos_curr, EntCenter(player)) <= player->radius * 2) {
		p->ex_flags &= ~HARPOON_ACTIVE;
		p->harpoon_state = 0;
		rope->iterations = 16;
	}

	RopeUpdate(rope, dt);
}

void HarpoonPull(Entity *player, float dt) {
	PlayerData *p = player->data;

	//p->rope->segment_dist = Lerp(p->rope->segment_dist, 0, dt * 20);
	//p->rope->segment_dist -= (p->rope->segment_dist * 0.1f) * dt;
	p->rope->segment_dist *= (0.99f * dt);

	rope_t -= dt;
	if(rope_t < 0) {
		p->rope->start_id++;
		rope_t = 1.0f;
	}

	p->rope->nodes[p->rope->start_id].flags &= ~NODE_PINNED;
	//p->rope->nodes[ROPE_TAIL].flags |= NODE_PINNED;
	RopeNodeSetPos(&p->rope->nodes[ROPE_TAIL], p->harpoon_pos);

	Vector2 new_center = p->rope->nodes[p->rope->start_id].pos_curr; 
	Vector2 new_pos = Vector2Subtract(new_center, player->center_offset);
	player->position = Vector2Lerp(player->position, new_pos, dt * 10);

	Vector2 to_node = Vector2Normalize(Vector2Subtract(p->rope->nodes[p->rope->start_id].pos_curr, EntCenter(player)));
	Vector2 wish_vel = Vector2Scale(to_node, 4.0f);
	//player->velocity = Vector2Add(player->velocity, wish_vel);

	//player->velocity = wish_vel;

	//RopeUpdate(p->rope, dt);

	// Check for collisions with asteroids 
	// NOTE:
	// Change later when spatial partioning is added!
	for(uint8_t i = 0; i < ent_handler->count; i++) {
		Entity *ent = &ent_handler->ents[i];

		if(ent->type != ENT_ASTEROID) continue;

		if(CheckCollisionCircles(EntCenter(player), player->radius * 1.25f, EntCenter(ent), ent->radius)) {
			p->ex_flags &= ~HARPOON_ACTIVE;
			p->harpoon_state = 0;
			to_node = Vector2Normalize(Vector2Subtract(p->harpoon_pos, EntCenter(player)));
			wish_vel = Vector2Scale(to_node, -10.0f);

			Vector2 to_ent = Vector2Normalize(Vector2Subtract(EntCenter(ent), EntCenter(player)));
			float dist = Vector2Distance(EntCenter(player), EntCenter(ent));
			float depth = ent->radius - dist;

			Vector2 prev_node = p->rope->nodes[0].pos_prev;
			Vector2 to_prev = Vector2Normalize(Vector2Subtract(prev_node, EntCenter(player)));

			player->position = Vector2Add(player->position, Vector2Scale(to_prev, 10));

			screenshake = Vector2Length(player->velocity) * 0.05f;
			break;
		}
	}

	if(Vector2Distance(p->rope->nodes[ROPE_TAIL].pos_curr, EntCenter(player)) <= player->radius * 2) {
		p->ex_flags &= ~HARPOON_ACTIVE;
		p->harpoon_state = 0;
		to_node = Vector2Normalize(Vector2Subtract(p->harpoon_pos, EntCenter(player)));
		wish_vel = Vector2Scale(to_node, 4.0f);
		player->velocity = wish_vel;

		screenshake = 0.1f;
	}

	RopeUpdate(p->rope, dt);
}

void HarpoonReel(Entity *player, float dt) {
	PlayerData *p = player->data;
	Entity *fish = p->harpoon_hit_ent;

	p->rope->segment_dist *= 0.99f;
	//p->rope->nodes[ROPE_TAIL].flags &= ~NODE_PINNED;
	//p->harpoon_pos = EntCenter(fish);
	
	//p->harpoon_pos = Vector2Add(fish->position, p->harpoon_offset);
	//RopeNodeSetPos(&p->rope->nodes[ROPE_TAIL], p->harpoon_pos);

	if(p->rope->segment_dist < 1) 
		p->rope->iterations = 128;

	Vector2 ftop = Vector2Subtract(Vector2Add(EntCenter(player), Vector2Scale(player->velocity, dt)), EntCenter(fish));
	ftop = Vector2Normalize(ftop);

	for(uint8_t i = 1; i < ROPE_TAIL - 1; i++) {
		RopeNode *node = &p->rope->nodes[i];
		
		Vector2 to_player = Vector2Normalize(Vector2Subtract(node->pos_curr, EntCenter(player))); 

		node->pos_curr = Vector2Add(node->pos_curr, Vector2Scale(to_player, 200 * dt));
	}

	Vector2 new_center = p->rope->nodes[ROPE_TAIL].pos_curr; 
	Vector2 new_pos = Vector2Subtract(new_center, fish->center_offset);
	fish->position = new_pos;

	//fish->velocity = Vector2Add(fish->velocity, Vector2Scale(ftop, 1000 * dt));

	if(Vector2Distance(EntCenter(fish), EntCenter(player)) <= player->radius * 1.6f + (Vector2Length(fish->velocity) * dt)) {
		p->ex_flags &= ~HARPOON_ACTIVE;
		p->harpoon_state = 0;

		screenshake = 0.1f;
		fish->flags &= ~ENT_ACTIVE;

		ent_handler->fish_collected++;
	}

	RopeUpdate(p->rope, dt);
}

void PlayerApplyFling(Entity *player, Entity *ent, float dt) {
	if(!ent) return;

	PlayerData *p = player->data;

	if(p->ex_flags & HARPOON_ACTIVE) return;

	p->fling_timer -= dt;	
	if(p->fling_timer <= 0) {
		p->ex_flags &= ~PLR_FLING;
		return;
	}

	Vector2 to_ent = Vector2Subtract(EntCenter(ent), EntCenter(player));
	to_ent = Vector2Normalize(to_ent);

	player->velocity = Vector2Add(player->velocity, Vector2Scale(to_ent, (55 - p->fling_timer * 2) * dt));	
	player->velocity = Vector2Add(player->velocity, Vector2Scale(p->fling_vel, (1.0f - (p->fling_timer * 1.75f)) * dt));
}

