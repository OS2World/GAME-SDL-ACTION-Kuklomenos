/*
 * Kuklomenos
 * Copyright (C) 2008-2009 Martin Bays <mbays@sdf.lonestar.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include <cstdlib>
#include <vector>
#include <algorithm>

#include "sound.h"

Mix_Chunk* invDieChunk = NULL;
Mix_Chunk* invHitChunk = NULL;
Mix_Chunk* sparkChunk = NULL;
Mix_Chunk* shotChunk = NULL;
Mix_Chunk* shieldChunk = NULL;
Mix_Chunk* primedChunk = NULL;
Mix_Chunk* nodeHumChunk = NULL;
Mix_Chunk* mutChunk = NULL;

SoundEvents soundEvents;


#ifndef SOUND
void SoundEvents::newEvent(RelPolarCoord pos, Mix_Chunk* chunk,
	int volume, int stretch, bool noSight)
{} 
void SoundEvents::update(RelPolarCoord aimPos)
{}
#else

#include <SDL2/SDL_mixer.h>

/* Mix_Stretch is a non-standard extension in the bundled SDL_mixer.
 * Stub it out — the pitch-shifting effect is sacrificed for portability. */
#ifndef Mix_Stretch
static inline int Mix_Stretch(int channel, int stretch) { (void)channel; (void)stretch; return 0; }
#endif

#include "data.h"
#include "coords.h"
#include "geom.h"
#include "random.h"
#include "settings.h"

void channelDone(int channel)
{
    soundEvents.channelDone(channel);
}

SoundEvent::SoundEvent(RelPolarCoord pos, Mix_Chunk* chunk,
	int volume, int stretch, bool noSight) :
    pos(pos), chunk(chunk), volume(volume), noSight(noSight),
    hadFirstUpdate(false), finished(false)
{
    channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel == -1)
	finished = true;
    else
    {
	Mix_ChannelFinished(channelDone);
	Mix_Stretch(channel, stretch);
	Mix_Volume(channel, 0);
    }
}

void SoundEvent::update(RelPolarCoord aimPos)
{
    if (!hadFirstUpdate)
    {
	int evAngle;
	if (pos.dist == 0)
	    evAngle=0;
	else
	{
	    Angle da = pos.angle - aimPos.angle;
	    evAngle = 360 - int(float(da)*90);
	}
	int evDist = 255*int(pos.dist)/ARENA_RAD;
	SDL_LockAudio();
	Mix_SetPosition(channel, evAngle, evDist);
	SDL_UnlockAudio();
    }

    float sightBoundVolMult = 1;
    if (!noSight)
    {
	// we can only hear the event when it's in sight

	// sightBoundsDist: distance of event from centre of sight bounding
	// circle, where 1.0 is the radius of that circle
	const float sightBoundsDist =
	    dist((ARENA_CENTRE+pos) - (ARENA_CENTRE+aimPos)) /
	    (ARENA_RAD - aimPos.dist);

	// linear dropoff at the edges
	sightBoundVolMult =
	    sightBoundsDist > 1 ? 0.0 :
	    sightBoundsDist < 0.8 ? 1.0 :
	    1.0 - 5*(sightBoundsDist-0.8);

	if (!hadFirstUpdate)
	    startVolMult = sightBoundVolMult;

	// don't allow to vary too far from initial mult:
	sightBoundVolMult = std::min(std::max(
		    startVolMult - 0.3f,
		    sightBoundVolMult),
		startVolMult + 0.3f);
    }

    Mix_Volume(channel, int(volume*settings.volume*sightBoundVolMult));

    if (!hadFirstUpdate)
	hadFirstUpdate = true;
}

void SoundEvents::update(RelPolarCoord aimPos)
{
    for (std::vector<SoundEvent>::iterator it = begin(); it != end(); it++)
	it->update(aimPos);

    erase(remove_if(begin(),
		end(), SoundEvent::isFinished),
	    end());
}

void SoundEvents::newEvent(RelPolarCoord pos, Mix_Chunk* chunk,
	int volume, int stretch, bool noSight)
{
    if (settings.sound && ( audioInitialised || initialiseAudio() ))
	push_back(SoundEvent(pos, chunk, volume, stretch, noSight));
}

void SoundEvents::channelDone(int channel)
{
    for (std::vector<SoundEvent>::iterator it = begin(); it != end(); it++)
	if (it->channel == channel)
	    it->finished = true;
}

bool SoundEvents::initialiseAudio()
{
    const int audio_buffers = 512;
    int audio_rate, audio_channels, bits;
    Uint16 audio_format;
    if (Mix_OpenAudio(settings.soundFreq, MIX_DEFAULT_FORMAT, 2,
		audio_buffers) < 0)
	return false;
    // print out some info on the audio device and stream
    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
    bits=audio_format&0xFF;
    printf("Opened audio at %d Hz %d bit %s, %d bytes audio buffer\n", audio_rate,
	    bits, audio_channels>1?"stereo":"mono", audio_buffers );

    invDieChunk = Mix_LoadWAV(findDataPath("sounds/invdie.ogg").c_str());
    invHitChunk = Mix_LoadWAV(findDataPath("sounds/invhit.ogg").c_str());
    sparkChunk = Mix_LoadWAV(findDataPath("sounds/spark.ogg").c_str());
    primedChunk = Mix_LoadWAV(findDataPath("sounds/primed.ogg").c_str());
    shieldChunk = Mix_LoadWAV(findDataPath("sounds/shield.ogg").c_str());
    shotChunk = Mix_LoadWAV(findDataPath("sounds/shot.ogg").c_str());
    nodeHumChunk = Mix_LoadWAV(findDataPath("sounds/hum.ogg").c_str());
    mutChunk = Mix_LoadWAV(findDataPath("sounds/mutilation.ogg").c_str());

    if (!shotChunk)
	printf("Failed to open sound file 'sounds/shot.ogg'\n");

    Mix_AllocateChannels(32);

    audioInitialised = true;
    return true;
}

#endif
