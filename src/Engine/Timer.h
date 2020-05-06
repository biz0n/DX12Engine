#pragma once

#include <Types.h>

class Timer
{
public:
    Timer();

    float32 TotalTime() const;
    float32 DeltaTime() const;

    void Start();
    void Stop();
    void Reset();
    void Tick();

    bool IsPaused() const { return mPaused; }

private:
    float64 mSecondsPerCount;
    float64 mDeltaTime;

    int64 mBaseTime;
    int64 mPausedTime;
    int64 mStopTime;
    int64 mPrevTime;
    int64 mCurrTime;

    bool mPaused;
};