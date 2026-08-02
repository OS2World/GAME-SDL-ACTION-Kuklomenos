#ifndef INC_CONFIG_H
#define INC_CONFIG_H

#include <string>
#include <vector>
using namespace std;

#include "settings.h"
#include "keybindings.h"

const int CONFFILE_VERSION_CURRENT = 1;
const int CONFFILE_VERSION_FIRST = 1;

class Config
{
    public:
	double rating[3];
	double highestRating[3];

	UseAALevel useAA;
	bool showGrid;
	bool zoomEnabled;
	bool rotatingView;
	float turnRateFactor;
	int fps;
	bool showFPS;
	bool sound;
	float volume;
	int soundFreq;
	double aaGamma;
	int speed;

	bool shouldUpdateRating;
	char username[17];
	unsigned int uuid;

	Keybindings rebindings;

	BGType bgType;

	int width;
	int height;
	int bpp;
	bool fullscreen;

	int confFileVersion;
	
	void read();
	void write() const;
	void importSettings(const Settings& settings);
	void exportSettings(Settings& settings) const;

	Config();
};

int obfuscatedRating(double rating);

extern Config config;

#endif /* INC_CONFIG_H */
