#include "Timer.h"
#include <Windows.h>

namespace Engine
{
    Timer::Timer() : mSecondsPerCount(0.0), mDeltaTime(-1.0), mBaseTime(0),
                     mPausedTime(0), mStopTime(0), mPrevTime(0), mCurrTime(0),
                     mPaused(false)
    {
        int64 countsPerSec;
        QueryPerformanceFrequency((LARGE_INTEGER *)&countsPerSec);
        mSecondsPerCount = 1 / (float64)countsPerSec;
    }

    float32 Timer::TotalTime() const
    {
        if (mPaused)
        {
            return (float32)(((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
        }
        else
        {
            return (float32)(((mCurrTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
        }
    }

    float32 Timer::DeltaTime() const
    {
        return (float32)mDeltaTime;
    }

    void Timer::Start()
    {
        int64 startTime;
        QueryPerformanceCounter((LARGE_INTEGER *)&startTime);

        if (mPaused)
        {
            mPausedTime += (startTime - mStopTime);
            mPrevTime = startTime;
            mStopTime = 0;

            mPaused = false;
        }
    }

    void Timer::Stop()
    {
        int64 stopTime;
        QueryPerformanceCounter((LARGE_INTEGER *)&stopTime);

        if (!mPaused)
        {
            mStopTime = stopTime;
            mPaused = true;
        }
    }

    void Timer::Reset()
    {
        int64 resetTime;
        QueryPerformanceCounter((LARGE_INTEGER *)&resetTime);

        mBaseTime = resetTime;
        mPrevTime = resetTime;

        mStopTime = 0;
        mPaused = false;
    }

    void Timer::Tick()
    {
        if (mPaused)
        {
            mDeltaTime = 0.0;
            return;
        }

        int64 currentTime;
        QueryPerformanceCounter((LARGE_INTEGER *)&currentTime);

        mCurrTime = currentTime;
        mDeltaTime = (currentTime - mPrevTime) * mSecondsPerCount;

        mPrevTime = mCurrTime;

        if (mDeltaTime < 0.0)
        {
            mDeltaTime = 0.0;
        }
    }

} // namespace Engine