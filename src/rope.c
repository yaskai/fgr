#include <stdlib.h>
#include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "rope.h"
#include "ent_handler.h"

#define GRAVITY (Vector2){0, 30}
#define dt2		(dt * dt)

EntHandler *rope_handler_ptr = NULL;
void RopeSetHandler(EntHandler *handler) { rope_handler_ptr = handler; }

void RopeInit(Rope *rope, Vector2 pos) {
	rope->length = ROPE_LENGTH;
	rope->iterations = 32;
	rope->dampening = 0.825f;
	rope->segment_dist = 3.25f;
	rope->start_id = 0;

	rope->nodes = calloc(rope->length, sizeof(RopeNode));

	for(uint8_t i = 1; i < rope->length; i++) {
		RopeNode *node = &rope->nodes[i];
		node->pos_curr = pos;
		node->pos_prev = node->pos_curr;
		node->mass = 1.0f;
	}

	rope->nodes[0] = (RopeNode) {
		.flags = (NODE_PINNED),
		.pos_prev = pos,
		.pos_curr = pos,
		.mass = 1.0f
	};

	rope->nodes[rope->length-1] = (RopeNode) {
		.flags = (NODE_PINNED),
		.pos_prev = pos,
		.pos_curr = pos,
		.mass = 1.0f
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
		
		Vector2 vel_transfer = (Vector2){0, 0};
		if(i < ROPE_TAIL) {
			vel_transfer = Vector2Scale(Vector2Subtract(rope->nodes[i+1].pos_curr, rope->nodes[i].pos_curr), 0.01f);
			rope->nodes[i+1].pos_prev = Vector2Add(rope->nodes[i+1].pos_prev, vel_transfer);
		}

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

		if(dist == 0)  	continue;

		Vector2 prev_a = node_a->pos_curr;
		Vector2 prev_b = node_b->pos_curr;

		Vector2 vel_transfer = Vector2Scale(Vector2Subtract(node_b->pos_curr, node_a->pos_curr), 0.15f);
		
		if((node_a->flags & NODE_PINNED) && (node_b->flags & NODE_PINNED))
			continue;
		else if(node_a->flags & NODE_PINNED) {
			node_b->pos_curr = Vector2Add(node_b->pos_curr, Vector2Scale(delta, correction));	
			node_b->pos_prev = Vector2Add(node_b->pos_prev, Vector2Scale(vel_transfer, node_a->mass));
		} else if(node_b->flags & NODE_PINNED) {
			node_a->pos_curr = Vector2Add(node_b->pos_curr, Vector2Scale(delta, correction));	
			node_a->pos_prev = Vector2Subtract(node_a->pos_prev, Vector2Scale(vel_transfer, node_b->mass));
		}

		//if(!Vector2Equals(prev_a, node_a->pos_curr) || !Vector2Equals(prev_b,  node_b->pos_curr)) continue;

		node_a->pos_curr = Vector2Subtract(node_a->pos_curr, Vector2Scale(delta, 0.5f * correction * node_a->mass));
		node_b->pos_curr = Vector2Add(node_b->pos_curr, Vector2Scale(delta, 0.5f * correction * node_b->mass));
	}
}

void RopeCollision(Rope *rope, float dt) {
	if(rope_handler_ptr == NULL) return;
	EntHandler *handler = rope_handler_ptr;

	rope->coll_id = 0;

	for(uint8_t i = 0; i < rope->length - 1; i++) {
		RopeNode *node = &rope->nodes[i];
		if(node->flags & NODE_PINNED) continue;

		for(uint16_t j = 0; j < handler->count; j++) {
			Entity *ent = &handler->ents[j];		
			
			if(!(ent->flags & ENT_IS_BODY)) continue;

			Vector2 center = EntCenter(ent);
			Vector2 to_node = Vector2Subtract(node->pos_curr, center);
			float dist = Vector2Length(to_node);

			if(dist <= ent->radius && dist > 0) {
				Vector2 norm = Vector2Normalize(to_node);

				float depth = ent->radius - dist;

				node->pos_curr = Vector2Add(center, Vector2Scale(norm, ent->radius * 1.01f));

				Vector2 vel = Vector2Subtract(node->pos_curr, node->pos_prev);

				float vel_to_body = Vector2DotProduct(vel, norm);
				if(vel_to_body < 0) 
					vel = Vector2Subtract(vel, Vector2Scale(norm, vel_to_body * 1.25f));

				vel = Vector2Scale(vel, 0.59f);
				node->pos_prev = Vector2Subtract(node->pos_curr, vel);

				rope->coll_id = i;

				break;
			}
		}	
	}
}

void RopeDiffuse(Rope *rope, float dt) {
}

void RopeUpdate(Rope *rope, float dt) {
	//rope->nodes[0].pos_prev = rope->nodes[0].pos_curr;

	for(uint8_t i = 0; i < rope->iterations; i++) {
		RopeIntegrate(rope, dt);
		RopeCollision(rope, dt);

		for(uint8_t j = 0; j < 5; j++) { 
			RopeSolveConstraints(rope, dt);
		}
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
	node->pos_curr = pos;
	node->pos_prev = pos;
}

