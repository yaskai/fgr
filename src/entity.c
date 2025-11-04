#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "entity.h"
#include "kmath.h"

void EntInit(Entity *ent, uint8_t type) {
	
}

void EntUpdatePosition(Entity *ent, float dt) {
	// Move entity by it's velocity scaled by delta time
	ent->position = Vector2Add(ent->position, Vector2Scale(ent->velocity, dt));	
}

Vector2 EntCenter(Entity *ent) {
	return Vector2Add(ent->position, ent->center_offset);
}

void EntOrbitStart(Entity *ent, Entity *orbit_body) {
	ent->orbit_data = (OrbitData){0};

	Vector2 ent_center = EntCenter(ent);
	Vector2 orb_center = EntCenter(orbit_body);
	//ent->pull_point = Vector2Subtract(orb_center, orbit_body->center_offset);
	ent->pull_point = orb_center;

	Vector2 d = Vector2Subtract(ent_center, orb_center); 	
	float r = Vector2Length(d);
	float h = r - (orbit_body->radius + ent->radius);
	if(h < 0) h = 0;

	Vector2 dir = Vector2Normalize(d);
	Vector2 edge = Vector2Add(orb_center, Vector2Scale(dir, orbit_body->radius));

	Vector2 tangent = (Vector2){-dir.y, dir.x};
	ent->orbit_angle = atan2f(dir.y, dir.x);

	if(ent->type == ENT_PLAYER) {
		PlayerData *p = ent->data;
		//p->orbit_vel.y = 10;

		float orbital_vel = sqrtf(p->grav_force * orbit_body->radius) * 0.1f;
		p->orbit_vel.x = orbital_vel;
		p->orbit_vel.y = 0;
	}

	ent->flags |= ENT_ORBIT;

	ent->orbit_data.initial_pos = ent_center;
	ent->orbit_height = h;
	ent->orbit_data.body_radius = orbit_body->radius;
	ent->orbit_data.edge = edge;
	ent->orbit_data.orbit_center = orb_center;
}

void EntOrbitUpdate(Entity *ent, Entity *orbit_body, float dt) {
	Vector2 ent_center = EntCenter(ent);
	//Vector2 orb_center = EntCenter(orbit_body);
	Vector2 orb_center = ent->pull_point;

	Vector2 dir = (Vector2){cosf(ent->orbit_angle), sinf(ent->orbit_angle)};
	ent_center = Vector2Add(orb_center, Vector2Scale(dir, orbit_body->radius + ent->orbit_height));	

	ent->position = Vector2Add(ent_center, Vector2Scale(ent->center_offset, -1));

	Vector2 tangent = {-dir.y, dir.x};

	float targ_spr_angle = atan2f(tangent.y, tangent.x) * RAD2DEG; 
	ent->sprite_angle = AngleLerp(ent->sprite_angle, targ_spr_angle, 0.1f * dt);

	if(ent->orbit_height <= ent->radius) {
		ent->orbit_height = ent->radius;
		ent->flags |= ENT_GROUNDED;
	}

	if(ent->type == ENT_PLAYER) {
		PlayerData *p = ent->data;
		p->orbit_dir = dir;
	}
	
	// Limit orbit angle
	if(ent->orbit_angle > PI2) ent->orbit_angle -= PI2;
	else if(ent->orbit_angle < 0) ent->orbit_angle += PI2;

	ent->orbit_data.orbit_center = orb_center;
	ent->orbit_data.dir = dir;
	ent->orbit_data.height = ent->orbit_height;
	ent->orbit_data.edge = Vector2Add(orb_center, Vector2Scale(dir, orbit_body->radius));
	ent->orbit_data.curr_pos = EntCenter(ent);
}

void OrbitDataDrawDebug(OrbitData *data) {
	//DrawCircleV(data->initial_pos, 10, RAYWHITE);
	//DrawLineV(data->edge, data->orbit_center, RAYWHITE);	
	//DrawLineV(data->edge, data->initial_pos, RAYWHITE);	

	Color color = ColorAlpha(WHITE, 0.95f);

	DrawLineEx(data->curr_pos, data->edge, 4, color);

	Vector2 ground_dir = (Vector2){-data->dir.y, data->dir.x};
	Vector2 line_start = Vector2Add(data->edge, Vector2Scale(ground_dir, -100));
	Vector2 line_end   = Vector2Add(data->edge, Vector2Scale(ground_dir,  100));
	DrawLineV(line_start, line_end, color);
	DrawLineEx(line_start, line_end, 4, color);

	DrawCircleLinesV(data->orbit_center, data->body_radius + data->height, color);

	//Vector2 next_dir = (Vector2){cosf(data->angular_vel), sinf(data->angular_vel)};
	//Vector2 next_edge = Vector2Add(data->orbit_center, Vector2Scale(next_dir, data->body_radius));	

	DrawCircleV(data->edge, 5, color);
	//DrawCircleV(next_edge, 5, color);
	//DrawLineEx(data->curr_pos, next_edge, 4, color);
	//DrawLineEx(data->edge, next_edge, 4, color);

	//DrawText(TextFormat("%f", data->rad_vel), data->orbit_center.x, data->orbit_center.y, 16, color);
}

