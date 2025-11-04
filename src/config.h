#ifndef CONFIG_H_
#define CONFIG_H_

// Configuration defaults, revert to these values if config file not found
// Default window width, height and refresh rate
#define CONFIG_DEFAULT_WW	1920
#define CONFIG_DEFAULT_WH	1080
#define CONFIG_DEFAULT_RR	  60

#define AUTO "auto"
#define streq(a, b) (strcmp((a), (b)) == 0)

typedef struct {
	int windowWidth, windowHeight;
	float refreshRate;
} Config;

void ConfigRead(Config *conf, char *path);
void ConfigParseLine(Config *conf, char *line);
void ConfigSetDefault(Config *conf);
void ConfigPrintValues(Config *conf);

#endif // !CONFIG_H_
