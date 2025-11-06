#include <stdint.h>
#include "raylib.h"

#ifndef ROPE_H_
#define ROPE_H_

#define ROPE_LENGTH 128 
#define ROPE_TAIL 	(ROPE_LENGTH - 1)

#define NODE_PINNED 	0x01
#define NODE_SKIP_DRAW	0x02

typedef struct {
	uint8_t flags;
	
	Vector2 pos_prev;
	Vector2 pos_curr;
} RopeNode;

typedef struct {
	uint8_t flags;
	uint8_t length;
	uint8_t iterations;

	float segment_dist;
	float dampening;

	Vector2 gravity;

	RopeNode *nodes;
} Rope;

void RopeInit(Rope *rope, Vector2 pos);
void RopeIntegrate(Rope *rope, float dt);
void RopeSolveConstraints(Rope *rope, float dt);
void RopeUpdate(Rope *rope, float dt);
void RopeDraw(Rope *rope);

void RopeNodeSetPos(RopeNode *node, Vector2 pos);

#endif
