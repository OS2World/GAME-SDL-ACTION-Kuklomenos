#ifndef INC_SOUND_H
#define INC_SOUND_H

#include "coords.h"
#include <vector>

struct Mix_Chunk;

class SoundEvent
{
    private:
	RelPolarCoord pos;
	Mix_Chunk* chunk;
	int volume;
	bool noSight;

	bool hadFirstUpdate;
	float startVolMult;

    public:
	int channel;
	bool finished;

	static bool isFinished(const SoundEvent& ev) { return ev.finished; }

	void update(RelPolarCoord aimPos);

	SoundEvent(RelPolarCoord pos, Mix_Chunk* chunk,
		int volume=128, int stretch=1000, bool noSight=false);
};

class SoundEvents : public std::vector<SoundEvent>
{
    private:
	bool audioInitialised;
	bool initialiseAudio();
    public:
	void update(RelPolarCoord aimPos);
	void newEvent(RelPolarCoord pos, Mix_Chunk* chunk,
		int volume=128, int stretch=1000, bool noSight=false);
	void channelDone(int channel);
	SoundEvents() : audioInitialised(false) {}
};

extern SoundEvents soundEvents;

extern Mix_Chunk* invDieChunk;
extern Mix_Chunk* invHitChunk;
extern Mix_Chunk* sparkChunk;
extern Mix_Chunk* shotChunk;
extern Mix_Chunk* shieldChunk;
extern Mix_Chunk* primedChunk;
extern Mix_Chunk* nodeHumChunk;
extern Mix_Chunk* mutChunk;

#endif /* INC_SOUND_H */
