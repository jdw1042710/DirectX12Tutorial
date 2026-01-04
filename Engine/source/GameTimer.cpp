#include "GameTimer.h"
#include <Windows.h>

namespace Engine
{
	GameTimer::GameTimer()
		: mSecondsPerCount(0.0), mDeltaTime(-1.0),
		mBaseTime(0), mPausedTime(0), mStopTime(0),
		mPrevTime(0), mCurrTime(0), mStopped(false)
	{
		__int64 countsPerSec;
		QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
		mSecondsPerCount = 1.0 / static_cast<double>(countsPerSec);
	}

	float GameTimer::TotalTime() const
	{
		if(mStopped) return static_cast<float>(((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
		return static_cast<float>(((mCurrTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
	}

	float GameTimer::DeltaTime() const
	{
		return static_cast<float>(mDeltaTime);
	}

	void GameTimer::Reset()
	{

		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		mBaseTime = currTime;
		mPrevTime = currTime;
		mStopTime = 0;
		mStopped = false;
	}

	void GameTimer::Start()
	{
		if (!mStopped) return;

		__int64 startTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&startTime);
		mPausedTime += (startTime - mStopTime);

		mPrevTime = startTime;
		mStopTime = 0;
		mStopped = false;
	}

	void GameTimer::Stop()
	{
		if (mStopped) return;

		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		mStopTime = currTime;
		mStopped = true;
	}

	void GameTimer::Tick()
	{
		if (mStopped)
		{
			mDeltaTime = 0.0;
			return;
		}

		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
		mCurrTime = currTime;
		mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;
		mPrevTime = mCurrTime;
		// 음수 방지 (프로세서가 절전모드 등인 경우, 음수가 될 가능성이 있음)
		if (mDeltaTime < 0.0)
			mDeltaTime = 0.0;
	}
}
