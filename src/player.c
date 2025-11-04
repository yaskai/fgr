#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"
#include "sprites.h"

// Initialize player, set data, pointers, references, etc.
void PlayerInit(Entity *player, SpriteLoader *sl, Camera2D *camera) {
	PlayerData *p = player->data;
	*p = (PlayerData){0};

	p->anchor_id = -1;
	p->active_anim = 0;
	p->grav_force = PLR_FALL_GRAV;
	p->run_anim = &sl->anims[0];
	p->camera = camera;

	player->center_offset = (Vector2){sl->spr_pool[0].frame_w * 0.5f, sl->spr_pool[0].frame_h * 0.5f};
	player->radius = player->center_offset.y;

	RopeInit(&p->rope, player->position);
}

void PlayerSpawn(Entity *player, Vector2 position) {
	
}

void PlayerUpdate(Entity *player, float dt) {
	PlayerData *p = player->data;

	//EntUpdatePosition(player, dt);
	EntUpdatePosition(player, dt);
	PlayerInput(player, dt);

	switch(p->state) {
		case PLR_IDLE:
			break;
		
		case PLR_RUN:
			AnimPlay(p->run_anim, dt);
			break;

		case PLR_JUMP:
			p->jump_timer -= dt;
			if(p->jump_timer <= 0) PlayerEndJump(player, false);
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

	if(p->anchor_id > -1) { 
		PlayerPhysicsOrbit(player, dt);

		if(player->flags & ENT_GROUNDED) 
			p->state = (p->input->move_x != 0) ? PLR_RUN : PLR_IDLE;
	} else { 
		PlayerPhysicsFreeFloat(player, dt);
	}

	p->rope.nodes[0].pos_prev = EntCenter(player), p->rope.nodes[0].pos_curr = EntCenter(player);
	p->rope.gravity = Vector2Scale(p->orbit_dir, -p->grav_force * 0.001f);
	//p->rope.gravity = Vector2Zero();
	RopeUpdate(&p->rope, dt);
}

void PlayerDraw(Entity *player, SpriteLoader *sl) {
	PlayerData *p = player->data;

	RopeDraw(&p->rope);
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

	if(player->flags & ENT_ORBIT) {
		if(run_held) {
			p->orbit_vel.x += (p->input->move_x * 2.5f) * dt;
			p->sprite_dir = p->input->move_x;
		}

		if(player->flags & ENT_GROUNDED) {
			if(p->input->jump) PlayerStartJump(player);
		} else {
			if(p->jump_timer > 0 && !p->input->jump) PlayerEndJump(player, true);
		}

		if(!run_held) p->orbit_vel.x += (-p->orbit_vel.x * 10.0f) * dt;
		p->orbit_vel.x = Clamp(p->orbit_vel.x, -2.0f, 2.0f);

	} else if(!(player->flags & ENT_ORBIT)) {
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

		PlayerPhysicsFreeFloat(player, dt);
	}
}

void PlayerPhysicsFreeFloat(Entity *player, float dt) {
	PlayerData *p = player->data;
	
	player->position = Vector2Add(player->position, player->velocity);

	PlayerCameraControls(player, dt);
}

void PlayerPhysicsOrbit(Entity *player, float dt) {
	PlayerData *p = player->data; 

	float rad_curr = player->orbit_data.body_radius + player->orbit_height;

	// Move around body (angular)
	player->orbit_angle += p->orbit_vel.x * dt;	

	// Move towards and away from body (radial)
	player->orbit_height += p->orbit_vel.y * dt;

	if(!(player->flags & ENT_GROUNDED)) {
		p->orbit_vel.y -= p->grav_force * dt;
	} else {
		p->orbit_vel.y = 0;
	}

	if(p->input->move_x == 0)
		p->orbit_vel.x += (-p->orbit_vel.x * 10.0f) * dt;

	p->orbit_vel.x = Clamp(p->orbit_vel.x, -2.0f, 2.0f);

	PlayerCameraControls(player, dt);
}

void PlayerStartJump(Entity *player) {
	PlayerData *p = player->data;

	p->jump_timer = 1.0f;
	p->orbit_vel.y = 400.0f;
	p->grav_force = PLR_JUMP_GRAV; 
	p->state = PLR_JUMP;

	// Request orbit raycast player->flags |= ENT_CAST_ORBIT;
	// Unground player
	player->flags &= ~ENT_GROUNDED;
}

void PlayerEndJump(Entity *player, bool cut) {
	PlayerData *p = player->data;

	p->grav_force = (cut) ? PLR_CUT_GRAV : PLR_FALL_GRAV;	
	p->state = PLR_FALL;
}

void PlayerCameraControls(Entity *player, float dt) {
	PlayerData *p = player->data;
	Camera2D *cam = p->camera;

	Vector2 player_center = EntCenter(player);
	cam->target = player_center;
	cam->rotation = -player->orbit_angle * RAD2DEG - 90;
}

