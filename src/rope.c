#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "rope.h"

#define GRAVITY (Vector2){0, 30}
#define dt2		(dt * dt)

void RopeInit(Rope *rope, Vector2 pos) {
	rope->length = ROPE_LENGTH;
	rope->iterations = 32;
	rope->dampening = 0.5f;
	rope->segment_dist = 3.25f;
	rope->start_id = 0;

	//rope->nodes = MemAlloc(sizeof(RopeNode) * rope->length); 
	rope->nodes = calloc(rope->length, sizeof(RopeNode));

	for(uint8_t i = 1; i < rope->length; i++) {
		RopeNode *node = &rope->nodes[i];
		node->pos_curr = (Vector2){pos.x + GetRandomValue(-200, 200), pos.y + (i*rope->segment_dist)};
		node->pos_prev = node->pos_curr;
		node->mass = 1.0f;
	}

	rope->nodes[0] = (RopeNode) {
		.flags = (NODE_PINNED),
		.pos_prev = pos,
		.pos_curr = pos,
		.mass = 1.25f
	};

	rope->nodes[rope->length-1] = (RopeNode) {
		.flags = (NODE_PINNED),
		.pos_prev = pos,
		.pos_curr = pos,
		.mass = 1.5f
	};
}
void RopeIntegrate(Rope *rope, float dt) {
	Vector2 mouse_pos = GetMousePosition();
	Vector2 mouse_delta = GetMouseDelta();

	for(uint8_t i = rope->start_id; i < rope->length; i++) {
		RopeNode *node = &rope->nodes[i];
		Vector2 new_prev = node->pos_curr;

		Vector2 vel = Vector2Subtract(node->pos_curr, node->pos_prev);	
		vel = Vector2Scale(vel, rope->dampening);
		Vector2 accel = Vector2Scale(rope->gravity, dt2);
		
		node->pos_curr = Vector2Add(Vector2Add(node->pos_curr, vel), accel);
		if(node->flags & NODE_PINNED) node->pos_curr = node->pos_prev;

		node->pos_prev = new_prev;
	}
}

void RopeSolveConstraints(Rope *rope, float dt) {
	rope->stretch = 0;
	rope->max_stretch = rope->segment_dist * (ROPE_TAIL) * 2.0f;

	for(uint8_t i = rope->start_id; i < ROPE_TAIL; i++) {
		RopeNode *node_a = &rope->nodes[i];
		RopeNode *node_b = &rope->nodes[i + 1];

		Vector2 delta = Vector2Subtract(node_a->pos_curr, node_b->pos_curr); 	
		float dist = Vector2Length(delta);
		float correction = (dist - rope->segment_dist) / dist; 

		rope->stretch += dist;

		if(dist == 0) continue;

		Vector2 prev_a = node_a->pos_curr;
		Vector2 prev_b = node_b->pos_curr;

		Vector2 vel_transfer = Vector2Scale(Vector2Subtract(node_b->pos_curr, node_a->pos_curr), 0.95f);
		
		if(node_a->flags & NODE_PINNED && node_b->flags & NODE_PINNED)
			continue;
		else if(node_a->flags & NODE_PINNED) {
			node_b->pos_curr = Vector2Add(node_b->pos_curr, Vector2Scale(delta, correction));	
			node_b->pos_prev = Vector2Add(node_b->pos_prev, Vector2Scale(vel_transfer, node_a->mass));
		} else if(node_b->flags & NODE_PINNED) {
			node_a->pos_curr = Vector2Add(node_b->pos_curr, Vector2Scale(delta, correction));	
			node_a->pos_prev = Vector2Subtract(node_a->pos_prev, Vector2Scale(vel_transfer, node_b->mass));
		}

		if(!Vector2Equals(prev_a, node_a->pos_curr) || !Vector2Equals(prev_b,  node_b->pos_curr)) continue;

		node_a->pos_curr = Vector2Subtract(node_a->pos_curr, Vector2Scale(delta, 0.5f * correction * node_a->mass));
		node_b->pos_curr = Vector2Add(node_b->pos_curr, Vector2Scale(delta, 0.5f * correction * node_b->mass));
	}
}

void RopeUpdate(Rope *rope, float dt) {
	//rope->nodes[0].pos_prev = rope->nodes[0].pos_curr;

	for(uint8_t i = 0; i < rope->iterations; i++) {
		RopeIntegrate(rope, dt);

		for(uint8_t j = 0; j < 4; j++) 
			RopeSolveConstraints(rope, dt);
	}
}

void RopeDraw(Rope *rope) {
	for(uint8_t i = rope->start_id; i < ROPE_TAIL; i++) {
		if(rope->nodes[i].flags & NODE_SKIP_DRAW) continue;

		Vector2 p0 = rope->nodes[i].pos_curr;
		Vector2 p1 = rope->nodes[i + 1].pos_curr;

		float dist = Vector2Distance(p0, p1);

		if(dist > rope->segment_dist) {
			float step = 4;
			float n = 0;

			Vector2 dir = Vector2Normalize(Vector2Subtract(p1, p0));

			while(n < dist) {
				Vector2 p = Vector2Add(p0, Vector2Scale(dir, n));
				DrawCircleV(p, 4, SKYBLUE);
				n += step;

				if(n > dist) break;
			}

		} else DrawCircleV(p0, 4, SKYBLUE);
	}
}

void RopeClose(Rope *rope) {
	free(rope->nodes);
}

void RopeNodeSetPos(RopeNode *node, Vector2 pos) {
	node->pos_prev = pos;
	node->pos_curr = pos;
}

