#ifndef INC_CLOCK_H
#define INC_CLOCK_H

struct GameClock
{
    int rate; // milliseconds of game-time per second of real-time
    bool paused;
    unsigned int ticks; // milliseconds of game-time elapsed

    GameClock(int rate=1000);

    int scale(int time);
    void update(int time);
    void updatePreScaled(int time);
};

#endif /* INC_CLOCK_H */
