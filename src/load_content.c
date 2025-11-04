#include <stdio.h>
#include "raylib.h"
#include "sprites.h"

//typedef void(*SpriteLoadFunc)(Game *game);
//typedef void(*AudioLoadFunc)(Game *game);

void LoadSpritesAll(SpriteLoader *sl) {
	puts("loading sprites...");

	LoadSpritesheet("resources/player_sheet.png", (Vector2){64, 64}, sl);
	AddSpriteAnim(&sl->spr_pool[0], FrameIndex(&sl->spr_pool[0], 0, 1), 4, 1, sl);

	LoadSpritesheet("resources/asteroid00.png", (Vector2){128, 128}, sl);
}

