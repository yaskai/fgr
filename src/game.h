#include <stdint.h>
#include "raylib.h"
#include "config.h"
#include "sprites.h"
#include "entity.h"
#include "ent_handler.h"
#include "input.h"

#ifndef GAME_H_
#define GAME_H_

// Render target resolution
#define VIRTUAL_WIDTH	1920	
#define VIRTUAL_HEIGHT	1080 

#define SPR_POOL &game->sprite_loader.spr_pool

// Game flags
#define GAME_DEBUG_MODE		0x01
#define GAME_PAUSED			0x02
#define GAME_QUIT_REQUEST   0x04
#define INPUT_SPECIFIED	    0x08

enum GAME_STATES {
	GAME_TITLE,
	GAME_MAIN,
	GAME_END,
	GAME_OPTIONS
};

enum INPUT_METHODS {
	KEYBOARD,
	GAMEPAD
};

typedef struct {
	uint8_t flags; 
	uint8_t state;
	uint8_t input_method;

	Rectangle render_src_rec, render_dest_rec;

	Config conf;
	Camera2D cam;
	InputState input_state;
	SpriteLoader sprite_loader;
	EntHandler ent_handler;
} Game;

void GameInit(Game *game);
void GameRenderInit(Game *game);
void GameContentInit(Game *game);

void GameUpdate(Game *game);

void GameDrawToBuffer(Game *game, uint8_t flags);
void GameDrawToWindow(Game *game);

void GameClose(Game *game);

void TitleUpdate(Game *game, float delta_time);
void TitleDraw(Game *game, uint8_t flags);

void MainUpdate(Game *game, float delta_time);
void MainDraw(Game *game, uint8_t flags);

void OverScreenUpdate(Game *game, float delta_time);
void OverScreenDraw(Game *game, uint8_t flags);

void OptionsScreenUpdate(Game *game, float delta_time);
void OptionsScreenDraw(Game *game, uint8_t flags);

void MainStart(Game *game);

void GameLoadSpriteBlock(Game *game, uint8_t block_id);
void GameLoadAudioBlock(Game *game, uint8_t block_id);

#endif
