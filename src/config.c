#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "config.h"

// Read configuration options from provided file
void ConfigRead(Config *conf, char *path) {
	// Open file
	FILE *pF = fopen(path, "r");

	// Early out and error log if file path invalid
	if(!pF) {
		printf("ERROR: Could not open configuration file at: %s\n", path);	
		ConfigSetDefault(conf);
		return;
	}

	puts("Reading configuration file...");
	
	// Parse config file line by line
	char line[64];
	while(fgets(line, sizeof(line), pF)) 
		ConfigParseLine(conf, line);		
	
	// Close file, print debug messages
	fclose(pF);
	ConfigPrintValues(conf);
}

// Parse an individual line from config file
void ConfigParseLine(Config *conf, char *line) {
	// Ignore commented lines
	if(line[0] == '#') return;
	
	// Split value and option key
	char *eq = strchr(line, '='); 
	if(!eq) return;	

	*eq = '\0';
	char *key = line;
	char *val = eq + 1;

	if(streq(key, "window_width")) {
		// Window width:    
		// if auto option provided, set from monitor resolution
		if(streq(val, AUTO)) 
			conf->windowWidth = GetMonitorWidth(0);
		else 
			sscanf(val, "%d", &conf->windowWidth);

	} else if(streq(key, "window_height")) {
		// Window height:    
		// if auto option provided, set from monitor resolution
		if(streq(val, AUTO)) 
			conf->windowHeight = GetMonitorHeight(0);
		else
			sscanf(val, "%d", &conf->windowHeight);
		
	} else if(streq(key, "refresh_rate")) {
		// Refresh rate:
		// if auto option provided, set from monitor refresh rate
		if(streq(val, AUTO)) 
			conf->refreshRate = GetMonitorRefreshRate(0);
		else 
			sscanf(val, "%f", &conf->refreshRate);
	}
}

// Set default config options 
void ConfigSetDefault(Config *conf) {
	*conf = (Config) {
		.windowWidth  = CONFIG_DEFAULT_WW,
		.windowHeight = CONFIG_DEFAULT_WH,
		.refreshRate  = CONFIG_DEFAULT_RR
	};

	ConfigPrintValues(conf);
}

// Print debug messages to shell
void ConfigPrintValues(Config *conf) {
	printf("resolution: %dx%d\n", conf->windowWidth, conf->windowHeight);
	printf("refresh rate: %f\n", conf->refreshRate);
}

