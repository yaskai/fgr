#ifndef SPRITES_H_
#define SPRITES_H_

#include <stdint.h>
#include "raylib.h"

#define SPR_TEX_VALID	0x01
#define SPR_ALLOCATED	0x02
#define SPR_PERSIST  	0x04
#define SPR_FLIP_X	   	0x08
#define SPR_FLIP_Y	   	0x10

typedef struct {
	uint8_t flags;
	uint8_t frame_count;		// Number of frames
	uint8_t cols, rows;			// Number of columns and rows

	uint8_t frame_w, frame_h;	// Width and height of frames

	Texture2D texture;			// Source image
} Spritesheet;

Spritesheet SpritesheetCreate(char *texture_path, Vector2 frame_dimensions);
void SpritesheetClose(Spritesheet *spritesheet);

void DrawSprite(Spritesheet *spritesheet, uint8_t frame_index, Vector2 position, uint8_t flags);
void DrawSpritePro(Spritesheet *spritesheet, uint8_t frame_index, Vector2 position, float rotation, float scale, uint8_t flags);

uint8_t FrameIndex(Spritesheet *spritesheet, uint8_t c, uint8_t r);
Rectangle GetFrameRec(uint8_t idx, Spritesheet *spritesheet);

typedef struct {
	uint8_t frame_count;		 // Total number of frames 
	uint8_t start_frame;		 // Which frame is "zero" or first frame
	uint8_t cur_frame;			 // Which frame should be displayed  

	float speed;				 // Animation speed
	float timer;				 // Timer for switching frames

	Spritesheet *spritesheet; 	 // Pointer to spritesheet instance
} SpriteAnimation;

SpriteAnimation AnimCreate(Spritesheet *spritesheet, uint8_t start_frame, uint8_t frame_count, float speed);
void AnimPlay(SpriteAnimation *anim, float delta_time);
void AnimDraw(SpriteAnimation *anim, Vector2 position, uint8_t flags);
void AnimDrawPro(SpriteAnimation *anim, Vector2 position, float rotation, float scale, uint8_t flags);

#define SPR_POOL_CAPACITY	255

enum spritesheets {
	SHEET_PLAYER,
	SHEET_ASTEROIDS,
	SHEET_FISH,
	SHEET_ITEMS
};

typedef struct {
	uint8_t spr_count;
	uint8_t anim_count;

	Spritesheet spr_pool[SPR_POOL_CAPACITY];
	SpriteAnimation anims[SPR_POOL_CAPACITY];
} SpriteLoader;

void LoadSpritesheet(char *tex_path, Vector2 frame_dimensions, SpriteLoader *sl);
void AddSpriteAnim(Spritesheet *spritesheet, uint8_t start_frame, uint8_t frame_count, float speed, SpriteLoader *sl);
void SpriteLoaderClose(SpriteLoader *sl);

void LoadSpritesAll(SpriteLoader *sl);

#endif // !SPRITES_H_ 
