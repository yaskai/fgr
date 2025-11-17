#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "map.h"

void MapLoad(EntHandler *handler, char *path) {
	FILE *pf = fopen(path, "r");	

	if(!pf) {
		printf("Could not load level from path: \n%s\n", path);
		return;
	}

	int16_t curr_id = 0;
	Entity *curr_ent = NULL;

	char line[128];
	while(fgets(line, sizeof(line), pf)) {
		if(line[0] == '[') {
			char name[128];
			strncpy(name, line + 1, sizeof(line) - 2);	

			curr_ent = &handler->ents[curr_id];

			if(!strcmp(name, "player")) {
				//ReserveDataPlayer(handler, curr_ent);
			} else if(!strcmp(name, "asteroid")) {
				//ReserveDataAsteroid(handler, curr_ent);
			} else if(!strcmp(name, "fish_spawner")) {
				//ReserveDataFish(handler, curr_ent);
			} else if(!strcmp(name, "item_spawner")) {

			}

			curr_id++;
		}
	}

	fclose(pf);
}

